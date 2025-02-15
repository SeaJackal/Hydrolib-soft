#include "hydrolib_serial_protocol_core.hpp"

#include <cstring>

using namespace hydrolib::serialProtocol;

#define MIN_MESSAGE_LENGTH 4

#define MARKER_LENGTH 1

#define ADDRESS_BITS_NUMBER 5
#define COMMAND_BITS_NUMBER (8 - ADDRESS_BITS_NUMBER)

#define ADDRESS_MASK 0xFF << (8 - ADDRESS_BITS_NUMBER)
#define COMMAND_MASK 0xFF >> (8 - COMMAND_BITS_NUMBER)

#define CRC_LENGTH 1

MessageProcessor::MessageProcessor(uint8_t address,
                                   hydrolib_SP_Interface_TransmitFunc transmit_func,
                                   RxQueue &rx_queue,
                                   uint8_t *public_memory,
                                   uint32_t public_memory_capacity)
    : transmit_func_(transmit_func),
      rx_queue_(rx_queue),
      current_rx_message_length_(0),
      current_rx_processed_length_(0),
      responce_buffer_(nullptr),
      current_header_(reinterpret_cast<MessageHeader_ *>(&current_rx_message_)),
      public_memory_(public_memory),
      public_memory_capacity_(public_memory_capacity)
{
    if (address >= 1 << (ADDRESS_BITS_NUMBER + 1))
    {
        return;
    }
    self_address_ = address << (8 - ADDRESS_BITS_NUMBER);
}

void MessageProcessor::ProcessRx()
{
    while (1)
    {
        if (current_rx_message_length_ == 0)
        {
            bool header_searching_status = MoveToHeader_();
            if (!header_searching_status)
            {
                return;
            }

            hydrolib_ReturnCode message_correct_check = ParseHeader_();
            switch (message_correct_check)
            {
            case HYDROLIB_RETURN_NO_DATA:
                return;

            case HYDROLIB_RETURN_FAIL:
                rx_queue_.Drop(1);
                continue;

            default:
                break;
            }
        }

        hydrolib_ReturnCode read_status =
            rx_queue_.Read(current_rx_message_ + current_rx_processed_length_,
                           current_rx_message_length_ - current_rx_processed_length_,
                           current_rx_processed_length_);
        if (read_status != HYDROLIB_RETURN_OK)
        {
            return;
        }
        uint8_t target_crc = CRCfunc_(current_rx_message_,
                                      current_rx_message_length_ - CRC_LENGTH);
        if (current_rx_message_[current_rx_message_length_ - CRC_LENGTH] == target_crc)
        {
            rx_queue_.Drop(current_rx_message_length_);
        }
        else
        {
            rx_queue_.Drop(1);
            current_rx_message_length_ = 0;
            current_rx_processed_length_ = 0;
            return;
        }

        ProcessCommand_();

        current_rx_message_length_ = 0;
        current_rx_processed_length_ = 0;
        return;
    }
}

hydrolib_ReturnCode MessageProcessor::TransmitWrite(uint8_t device_address,
                                                    uint32_t memory_address, uint32_t length,
                                                    uint8_t *data)
{
    if (length == 0)
    {
        return HYDROLIB_RETURN_FAIL;
    }

    uint8_t current_tx_message_length = sizeof(MessageHeader_::MemoryAccess) + length + CRC_LENGTH;

    uint8_t current_tx_message[HYDROLIB_SP_MAX_MESSAGE_LENGTH];
    MessageHeader_::MemoryAccess *tx_header =
        reinterpret_cast<MessageHeader_::MemoryAccess *>(&current_tx_message);

    tx_header->device_address =
        (device_address << (8 - ADDRESS_BITS_NUMBER)) | Command_::HYDROLIB_SP_COMMAND_WRITE;
    tx_header->self_address = self_address_;
    tx_header->memory_address = memory_address;
    tx_header->memory_access_length = length;

    std::memcpy(current_tx_message + sizeof(MessageHeader_::MemoryAccess),
                data, length);

    current_tx_message[current_tx_message_length - CRC_LENGTH] =
        CRCfunc_(current_tx_message, current_tx_message_length - CRC_LENGTH);

    transmit_func_(current_tx_message, current_tx_message_length);

    return HYDROLIB_RETURN_OK;
}

hydrolib_ReturnCode MessageProcessor::TransmitRead(uint8_t device_address,
                                                   uint32_t memory_address, uint32_t length,
                                                   uint8_t *buffer)
{
    if (!responce_buffer_)
    {
        return HYDROLIB_RETURN_BUSY;
    }

    if (length == 0)
    {
        return HYDROLIB_RETURN_FAIL;
    }

    uint8_t current_tx_message_length = sizeof(MessageHeader_::MemoryAccess) + length + CRC_LENGTH;

    uint8_t current_tx_message[HYDROLIB_SP_MAX_MESSAGE_LENGTH];
    MessageHeader_::MemoryAccess *tx_header =
        reinterpret_cast<MessageHeader_::MemoryAccess *>(&current_tx_message);

    tx_header->device_address =
        (device_address << (8 - ADDRESS_BITS_NUMBER)) | Command_::HYDROLIB_SP_COMMAND_READ;
    tx_header->self_address = self_address_;
    tx_header->memory_address = memory_address;
    tx_header->memory_access_length = length;

    current_tx_message[current_tx_message_length - CRC_LENGTH] =
        CRCfunc_(current_tx_message, current_tx_message_length - CRC_LENGTH);

    transmit_func_(current_tx_message, current_tx_message_length);

    responce_buffer_ = buffer;

    return HYDROLIB_RETURN_OK;
}

bool MessageProcessor::MoveToHeader_()
{
    uint16_t index = 0;
    hydrolib_ReturnCode finding_read_status = rx_queue_.Read(&current_rx_message_[0], MARKER_LENGTH, index);
    while (finding_read_status == HYDROLIB_RETURN_OK)
    {
        if (current_rx_message_[0] == (self_address_ | Command_::HYDROLIB_SP_COMMAND_WRITE) ||
            current_rx_message_[0] == (self_address_ | Command_::HYDROLIB_SP_COMMAND_READ) ||
            current_rx_message_[0] == (self_address_ | Command_::HYDROLIB_SP_COMMAND_RESPOND))
        {
            rx_queue_.Drop(index);
            return true;
        }
        else
        {
            index++;
        }
        finding_read_status = rx_queue_.Read(&current_rx_message_[0], MARKER_LENGTH, index);
    }

    rx_queue_.Clear();
    return false;
}

hydrolib_ReturnCode MessageProcessor::ParseHeader_()
{
    current_command_ = static_cast<Command_>(current_rx_message_[0] & COMMAND_MASK);

    hydrolib_ReturnCode read_status;
    hydrolib_ReturnCode parse_header_status;
    switch (current_command_)
    {
    case Command_::HYDROLIB_SP_COMMAND_READ:
        parse_header_status = ParseMemoryAccessHeader_();
        if (parse_header_status != HYDROLIB_RETURN_OK)
        {
            return parse_header_status;
        }

        current_rx_message_length_ = sizeof(MessageHeader_::MemoryAccess) + CRC_LENGTH;
        current_rx_processed_length_ = sizeof(MessageHeader_::MemoryAccess);
        break;

    case Command_::HYDROLIB_SP_COMMAND_WRITE:
        parse_header_status = ParseMemoryAccessHeader_();
        if (parse_header_status != HYDROLIB_RETURN_OK)
        {
            return parse_header_status;
        }

        current_rx_message_length_ =
            sizeof(MessageHeader_::MemoryAccess) +
            current_header_->memory_access_header.memory_access_length + CRC_LENGTH;
        current_rx_processed_length_ = sizeof(MessageHeader_::MemoryAccess);
        break;

    case Command_::HYDROLIB_SP_COMMAND_RESPOND:
        parse_header_status = ParseResponceHeader_();
        if (parse_header_status != HYDROLIB_RETURN_OK)
        {
            return parse_header_status;
        }

        current_rx_message_length_ =
            sizeof(MessageHeader_::Responce) +
            responce_data_length_ + CRC_LENGTH;
        current_rx_processed_length_ = sizeof(MessageHeader_::Responce);
        break;

    default:
        return HYDROLIB_RETURN_FAIL;
    }

    return HYDROLIB_RETURN_OK;
}

hydrolib_ReturnCode MessageProcessor::ParseMemoryAccessHeader_()
{
    hydrolib_ReturnCode read_status = rx_queue_.Read(
        current_rx_message_ + MARKER_LENGTH,
        sizeof(MessageHeader_::MemoryAccess), MARKER_LENGTH);
    if (read_status != HYDROLIB_RETURN_OK)
    {
        return HYDROLIB_RETURN_NO_DATA;
    }

    if (current_header_->memory_access_header.memory_access_length == 0)
    {
        return HYDROLIB_RETURN_FAIL;
    }

    uint16_t current_access_border =
        current_header_->memory_access_header.memory_address +
        current_header_->memory_access_header.memory_access_length;
    if (current_access_border > public_memory_capacity_)
    {
        return HYDROLIB_RETURN_FAIL;
    }

    return HYDROLIB_RETURN_OK;
}

hydrolib_ReturnCode MessageProcessor::ParseResponceHeader_()
{
    if (!responce_buffer_)
    {
        return HYDROLIB_RETURN_FAIL;
    }

    hydrolib_ReturnCode read_status = rx_queue_.Read(
        current_rx_message_ + MARKER_LENGTH,
        sizeof(MessageHeader_::Responce), MARKER_LENGTH);

    if (read_status != HYDROLIB_RETURN_OK)
    {
        return HYDROLIB_RETURN_NO_DATA;
    }

    if (current_header_->responce_header.self_address != responding_device_)
    {
        return HYDROLIB_RETURN_FAIL;
    }

    return HYDROLIB_RETURN_OK;
}

uint8_t MessageProcessor::CRCfunc_(const uint8_t *buffer, uint16_t length)
{
    uint16_t pol = 0x0700;
    uint16_t crc = buffer[0] << 8;
    for (uint8_t i = 1; i < length; i++)
    {
        crc |= buffer[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x8000)
            {
                crc = (crc << 1 ^ pol);
            }
            else
            {
                crc = crc << 1;
            }
        }
    }
    return crc >> 8;
}

void MessageProcessor::ProcessCommand_()
{
    switch (current_command_)
    {
    case Command_::HYDROLIB_SP_COMMAND_WRITE:
        memcpy(public_memory_ + current_header_->memory_access_header.memory_address,
               current_rx_message_ + sizeof(MessageHeader_::MemoryAccess),
               current_header_->memory_access_header.memory_access_length);
        break;

    case Command_::HYDROLIB_SP_COMMAND_READ:
    {
        uint8_t current_tx_message_length =
            sizeof(MessageHeader_::Responce) +
            current_header_->memory_access_header.memory_access_length + CRC_LENGTH;

        uint8_t current_tx_message[HYDROLIB_SP_MAX_MESSAGE_LENGTH];
        MessageHeader_::Responce *tx_header =
            reinterpret_cast<MessageHeader_::Responce *>(&current_tx_message);

        tx_header->device_address =
            (current_header_->memory_access_header.self_address << (8 - ADDRESS_BITS_NUMBER)) | Command_::HYDROLIB_SP_COMMAND_RESPOND;
        tx_header->self_address = self_address_;

        memcpy(current_tx_message + sizeof(MessageHeader_::Responce),
               public_memory_ + current_header_->memory_access_header.memory_address,
               current_header_->memory_access_header.memory_access_length);

        current_tx_message[current_tx_message_length - CRC_LENGTH] =
            CRCfunc_(current_tx_message, current_tx_message_length - CRC_LENGTH);

        transmit_func_(current_tx_message, current_tx_message_length);
    }
    break;

    case Command_::HYDROLIB_SP_COMMAND_RESPOND:
    {
        uint8_t current_tx_message[HYDROLIB_SP_MAX_MESSAGE_LENGTH];
        memcpy(current_tx_message + sizeof(MessageHeader_::Responce),
               responce_buffer_, responce_data_length_);
        responce_buffer_ = nullptr;
    }
    break;

    default:
        break;
    }
}
