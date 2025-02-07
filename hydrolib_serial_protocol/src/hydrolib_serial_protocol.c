#include "hydrolib_serial_protocol.h"

#define MIN_MESSAGE_LENGTH 4

#define HEADER_BYTE 0xAA

#define ADDRESS_BITS_NUMBER 5
#define COMMAND_BITS_NUMBER (8 - ADDRESS_BITS_NUMBER)

#define ADDRESS_MASK 0xFF << (8 - ADDRESS_BITS_NUMBER)
#define COMMAND_MASK 0xFF >> (8 - COMMAND_BITS_NUMBER)

#define PUBLIC_MEMORY_MAX_INDEX 0xFF

#define CRC_LENGTH 1

static void SearchAndParseMessage_(hydrolib_SerialProtocolHandler *self);

static bool MoveToHeader_(hydrolib_SerialProtocolHandler *self);
static hydrolib_ReturnCode ParseHeader_(hydrolib_SerialProtocolHandler *self);
static void ProcessCommand_(hydrolib_SerialProtocolHandler *self);

static hydrolib_ReturnCode ParseMemoryAccess_(hydrolib_SerialProtocolHandler *self);

hydrolib_ReturnCode hydrolib_SerialProtocol_Init(hydrolib_SerialProtocolHandler *self, uint8_t address,
                                                 hydrolib_SerialProtocol_InterfaceFunc receive_byte_func,
                                                 hydrolib_SerialProtocol_InterfaceFunc transmit_byte_func,
                                                 hydrolib_SerialProtocol_CRCfunc get_crc_func,
                                                 uint8_t *public_memory,
                                                 uint16_t public_memory_capacity)
{
    if (self->self_address >= 1 << (ADDRESS_BITS_NUMBER + 1))
    {
        self = NULL;
        return HYDROLIB_RETURN_FAIL;
    }
    self->self_address = address << (8 - ADDRESS_BITS_NUMBER);
    hydrolib_RingQueue_Init(&self->rx_ring_buffer, self->rx_buffer, HYDROLIB_SP_RX_BUFFER_CAPACITY);
    hydrolib_RingQueue_Init(&self->tx_ring_buffer, self->tx_buffer, HYDROLIB_SP_TX_BUFFER_CAPACITY);

    self->receive_byte_func = receive_byte_func;
    self->transmit_byte_func = transmit_byte_func;

    self->get_crc_func = get_crc_func;

    self->current_rx_message_length = 0;

    self->current_rx_processed_length = 0;

    self->public_memory = public_memory;
    self->public_memory_capacity = public_memory_capacity;

    self->header_rx_mem_access =
        (_hydrolib_SP_MessageHeaderMemAccess *)self->current_rx_message;

    return HYDROLIB_RETURN_OK;
}

void hydrolib_SerialProtocol_DoWork(hydrolib_SerialProtocolHandler *self)
{
    uint8_t transmit_byte;
    hydrolib_ReturnCode tx_read_status =
        hydrolib_RingQueue_ReadByte(&self->tx_ring_buffer, &transmit_byte, 0);
    if (tx_read_status == HYDROLIB_RETURN_OK)
    {
        if (self->transmit_byte_func(&transmit_byte))
        {
            hydrolib_RingQueue_Drop(&self->tx_ring_buffer, 1);
        }
    }

    uint8_t read_byte;
    bool read_status = self->receive_byte_func(&read_byte);

    if (read_status)
    {
        hydrolib_RingQueue_PushByte(&self->rx_ring_buffer, read_byte);
        SearchAndParseMessage_(self);
    }
}

hydrolib_ReturnCode hydrolib_SerialProtocol_TransmitWrite(hydrolib_SerialProtocolHandler *self,
                                                          uint8_t device_address,
                                                          uint8_t memory_address, uint8_t length,
                                                          uint8_t *buffer)
{
    if (length == 0)
    {
        return HYDROLIB_RETURN_FAIL;
    }

    uint16_t access_border = memory_address + length;
    if (access_border > self->public_memory_capacity)
    {
        return HYDROLIB_RETURN_FAIL;
    }

    uint8_t current_tx_message_length = sizeof(_hydrolib_SP_MessageHeaderMemAccess) + length + CRC_LENGTH;
    uint8_t available_length = hydrolib_RingQueue_GetCapacity(&self->tx_ring_buffer) -
                               hydrolib_RingQueue_GetLength(&self->tx_ring_buffer);

    if (available_length < current_tx_message_length)
    {
        return HYDROLIB_RETURN_BUSY;
    }

    uint8_t current_tx_message[HYDROLIB_SP_MAX_MESSAGE_LENGTH];
    _hydrolib_SP_MessageHeaderMemAccess *tx_header =
        (_hydrolib_SP_MessageHeaderMemAccess *)&current_tx_message;

    tx_header->device_address =
        (device_address << (8 - ADDRESS_BITS_NUMBER)) | _HYDROLIB_SP_COMMAND_WRITE;
    tx_header->memory_address = memory_address;
    tx_header->memory_access_length = length;

    memcpy(current_tx_message + sizeof(_hydrolib_SP_MessageHeaderMemAccess),
           buffer, length);

    current_tx_message[current_tx_message_length - CRC_LENGTH] =
        self->get_crc_func(current_tx_message, current_tx_message_length - CRC_LENGTH);

    hydrolib_RingQueue_Push(&self->tx_ring_buffer, current_tx_message, current_tx_message_length);

    return HYDROLIB_RETURN_OK;
}

static void SearchAndParseMessage_(hydrolib_SerialProtocolHandler *self)
{
    while (1)
    {
        if (self->current_rx_message_length == 0)
        {
            bool header_searching_status = MoveToHeader_(self);
            if (!header_searching_status)
            {
                return;
            }

            hydrolib_ReturnCode message_correct_check = ParseHeader_(self);
            switch (message_correct_check)
            {
            case HYDROLIB_RETURN_NO_DATA:
                return;

            case HYDROLIB_RETURN_FAIL:
                hydrolib_RingQueue_Drop(&self->rx_ring_buffer, 1);
                continue;

            default:
                break;
            }
        }

        hydrolib_ReturnCode read_status =
            hydrolib_RingQueue_Read(&self->rx_ring_buffer,
                                    self->current_rx_message + self->current_rx_processed_length,
                                    self->current_rx_message_length - self->current_rx_processed_length,
                                    self->current_rx_processed_length);
        if (read_status != HYDROLIB_RETURN_OK)
        {
            return;
        }
        uint8_t target_crc = self->get_crc_func(self->current_rx_message,
                                                self->current_rx_message_length - CRC_LENGTH);
        if (self->current_rx_message[self->current_rx_message_length - CRC_LENGTH] == target_crc)
        {
            hydrolib_RingQueue_Drop(&self->rx_ring_buffer, self->current_rx_message_length);
        }
        else
        {
            hydrolib_RingQueue_Drop(&self->rx_ring_buffer, 1);
            self->current_rx_message_length = 0;
            self->current_rx_processed_length = 0;
            return;
        }

        ProcessCommand_(self);

        self->current_rx_message_length = 0;
        self->current_rx_processed_length = 0;
        return;
    }
}

static bool MoveToHeader_(hydrolib_SerialProtocolHandler *self)
{
    uint16_t index = 0;
    hydrolib_ReturnCode finding_read_status =
        hydrolib_RingQueue_ReadByte(&self->rx_ring_buffer,
                                    &self->current_rx_message[0], index);
    while (finding_read_status == HYDROLIB_RETURN_OK)
    {
        if (self->current_rx_message[0] == (self->self_address | _HYDROLIB_SP_COMMAND_WRITE) ||
            self->current_rx_message[0] == (self->self_address | _HYDROLIB_SP_COMMAND_READ))
        {
            hydrolib_RingQueue_Drop(&self->rx_ring_buffer, index);
            return true;
        }
        else
        {
            index++;
        }
        finding_read_status = hydrolib_RingQueue_ReadByte(&self->rx_ring_buffer,
                                                          &self->current_rx_message[0], index);
    }

    hydrolib_RingQueue_Clear(&self->rx_ring_buffer);
    return false;
}

static hydrolib_ReturnCode ParseHeader_(hydrolib_SerialProtocolHandler *self)
{
    self->current_command = self->current_rx_message[0] & COMMAND_MASK;

    hydrolib_ReturnCode read_status;
    hydrolib_ReturnCode parse_access_status;
    switch (self->current_command)
    {
    case _HYDROLIB_SP_COMMAND_READ:
        parse_access_status = ParseMemoryAccess_(self);
        if (parse_access_status != HYDROLIB_RETURN_OK)
        {
            return parse_access_status;
        }

        self->current_rx_message_length = sizeof(_hydrolib_SP_MessageHeaderMemAccess) + CRC_LENGTH;
        self->current_rx_processed_length = sizeof(_hydrolib_SP_MessageHeaderMemAccess);
        break;

    case _HYDROLIB_SP_COMMAND_WRITE:
        parse_access_status = ParseMemoryAccess_(self);
        if (parse_access_status != HYDROLIB_RETURN_OK)
        {
            return parse_access_status;
        }

        self->current_rx_message_length =
            sizeof(_hydrolib_SP_MessageHeaderMemAccess) +
            self->header_rx_mem_access->memory_access_length + CRC_LENGTH;
        self->current_rx_processed_length = sizeof(_hydrolib_SP_MessageHeaderMemAccess);
        break;

    default:
        return HYDROLIB_RETURN_FAIL;
    }

    return HYDROLIB_RETURN_OK;
}

static void ProcessCommand_(hydrolib_SerialProtocolHandler *self)
{
    switch (self->current_command)
    {
    case _HYDROLIB_SP_COMMAND_WRITE:
        memcpy(self->public_memory + self->header_rx_mem_access->memory_address,
               self->current_rx_message + sizeof(_hydrolib_SP_MessageHeaderMemAccess),
               self->header_rx_mem_access->memory_access_length);
        break;
    default:
        break;
    }
}

static hydrolib_ReturnCode ParseMemoryAccess_(hydrolib_SerialProtocolHandler *self)
{
    hydrolib_ReturnCode read_status =
        hydrolib_RingQueue_Read(&self->rx_ring_buffer,
                                self->current_rx_message + 1,
                                sizeof(_hydrolib_SP_MessageHeaderMemAccess), 1);
    if (read_status != HYDROLIB_RETURN_OK)
    {
        return HYDROLIB_RETURN_NO_DATA;
    }

    if (self->header_rx_mem_access->memory_access_length == 0)
    {
        return HYDROLIB_RETURN_FAIL;
    }

    uint16_t current_access_border =
        self->header_rx_mem_access->memory_address + self->header_rx_mem_access->memory_access_length;
    if (current_access_border > self->public_memory_capacity)
    {
        return HYDROLIB_RETURN_FAIL;
    }

    return HYDROLIB_RETURN_OK;
}
