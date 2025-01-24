#include "hydrolib_serial_protocol.h"

#define HEADER_BYTE 0xAA

void SearchForMessage(hydrolib_SerialProtocolHandler *self);
uint8_t FindHeader(hydrolib_SerialProtocolHandler *self);

void hydrolib_SerialProtocol_Init(hydrolib_SerialProtocolHandler *self, uint8_t address,
                                  hydrolib_SerialProtocol_InterfaceFunc receive_byte_func,
                                  hydrolib_SerialProtocol_InterfaceFunc transmit_byte_func,
                                  hydrolib_SerialProtocol_CRCfunc get_crc_func)
{
    self->self_address = address;
    hydrolib_RingQueue_Init(&self->rx_ring_buffer, self->rx_buffer, HYDROLIB_SP_RX_BUFFER_CAPACITY);

    self->receive_byte_func = receive_byte_func;
    self->transmit_byte_func = transmit_byte_func;

    self->get_crc_func = get_crc_func;

    self->current_rx_data_length = 0;
}

void hydrolib_SerialProtocol_DoWork(hydrolib_SerialProtocolHandler *self)
{
    uint8_t read_byte;
    bool read_status = self->receive_byte_func(&read_byte);
    if (read_status)
    {
        hydrolib_RingQueue_PushByte(&self->rx_ring_buffer, read_byte);
        if (self->current_rx_data_length != 0)
        {
            return;
        }
        SearchForMessage(self);
    }
}

bool hydrolib_SerialProtocol_ReadMessage(hydrolib_SerialProtocolHandler *self, uint8_t *dest, uint8_t *data_length)
{
    if (self->current_rx_data_length == 0)
    {
        return false;
    }
    memcpy(dest, self->current_rx_data_buffer, self->current_rx_data_length);
    *data_length = self->current_rx_data_length;
    self->current_rx_data_length = 0;
    return true;
}

void SearchForMessage(hydrolib_SerialProtocolHandler *self)
{
    while (1)
    {
        uint8_t index = FindHeader(self);
        if (index != (uint8_t)(-1))
        {
            uint8_t message_length;
            hydrolib_ReturnCode read_status =
                hydrolib_RingQueue_ReadByte(&self->rx_ring_buffer, &message_length, index + 1);
            uint8_t data_length = message_length - 3;
            if (read_status != HYDROLIB_RETURN_OK)
            {
                break;
            }
            uint8_t i = 0;
            for (; i < data_length; i++)
            {
                read_status =
                    hydrolib_RingQueue_ReadByte(&self->rx_ring_buffer, &(self->current_rx_data_buffer[i]),
                                                index + 2 + i);
                if (read_status != HYDROLIB_RETURN_OK)
                {
                    break;
                }
            }
            if (i != data_length)
            {
                break;
            }
            uint8_t current_crc;
            read_status = hydrolib_RingQueue_ReadByte(&self->rx_ring_buffer, &current_crc, index + 2 + data_length);
            if (read_status != HYDROLIB_RETURN_OK)
            {
                break;
            }
            uint8_t target_crc = self->get_crc_func(self->current_rx_data_buffer, data_length);
            if (current_crc == target_crc)
            {
                hydrolib_RingQueue_Drop(&self->rx_ring_buffer, message_length);
                self->current_rx_data_length = data_length;
                break;
            }
            else
            {
                hydrolib_RingQueue_Drop(&self->rx_ring_buffer, index + 1);
            }
        }
        else
        {
            break;
        }
    }
}

uint8_t FindHeader(hydrolib_SerialProtocolHandler *self)
{
    hydrolib_ReturnCode finding_read_status = HYDROLIB_RETURN_OK;
    uint16_t index = 0;
    while (finding_read_status == HYDROLIB_RETURN_OK)
    {
        uint8_t read_byte;
        finding_read_status = hydrolib_RingQueue_ReadByte(&self->rx_ring_buffer, &read_byte, index);
        if (read_byte == self->self_address)
        {
            break;
        }
        else
        {
            index++;
        }
    }
    if (finding_read_status != HYDROLIB_RETURN_OK)
    {
        return -1;
    }
    return index;
}
