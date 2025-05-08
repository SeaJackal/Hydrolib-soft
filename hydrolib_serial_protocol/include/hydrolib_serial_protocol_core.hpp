/**
 @defgroup hydrolib_serial_protocol Hydrolib serial protocol
@brief Protocol for communication through duplex serial port

Protocol uses algorithm for header finding to implement non-constant message
length communication. It supports device public memory abstraction. It allows
devices to interact with each other by writing into each other public memory.

@{
*/

/**
 @file hydrolib_serial_protocol.h
@author SeaJackal
@brief Header file for hydrolib serial protocol
@version 0.1
@date 03-02-2025

*/
#ifndef HYDROLIB_SP_MESSAGE_PROCESSOR_H_
#define HYDROLIB_SP_MESSAGE_PROCESSOR_H_

#include <string.h>

#include <concepts>
#include <cstdint>

#include "hydrolib_common.h"
#include "hydrolib_queue_concepts.hpp"
#include "hydrolib_stream_concepts.hpp"

namespace hydrolib::serial_protocol
{

/**
 @brief Serial protocol manager handler structure which contains all information
    about session

    */

template <typename T>
concept PublicMemoryConcept =
    requires(T mem, void *read_buffer, const void *write_buffer,
             unsigned address, unsigned length) {
        {
            mem.Read(read_buffer, address, length)
        } -> std::same_as<hydrolib_ReturnCode>;

        {
            mem.Write(write_buffer, address, length)
        } -> std::same_as<hydrolib_ReturnCode>;

        { mem.Size() } -> std::convertible_to<unsigned>;
    };

template <concepts::stream::ByteStreamConcept TxStream,
          concepts::queue::ReadableByteQueue RxQueue,
          PublicMemoryConcept Memory>
class MessageProcessor
{
private:
    enum Command_
    {
        HYDROLIB_SP_COMMAND_WRITE = 1,
        HYDROLIB_SP_COMMAND_READ,
        HYDROLIB_SP_COMMAND_RESPOND
    };

    union MessageHeader_
    {
    public:
#pragma pack(push, 1)

        struct MemoryAccess
        {
            uint8_t device_address;
            uint8_t self_address;
            uint8_t memory_address;
            uint8_t memory_access_length;
        };

        struct Responce
        {
            uint8_t device_address;
            uint8_t self_address;
        };

#pragma pack(pop)

    public:
        MemoryAccess memory_access_header;
        Responce responce_header;
    };

public:
    constexpr static unsigned MAX_MESSAGE_LENGTH = 255;

private:
    constexpr static unsigned ADDRESS_BITS_NUMBER = 5;
    constexpr static unsigned COMMAND_BITS_NUMBER = (8 - ADDRESS_BITS_NUMBER);

    constexpr static uint8_t COMMAND_MASK = 0xFF >> (8 - COMMAND_BITS_NUMBER);
    constexpr static uint8_t ADDRESS_MASK = 0xFF & (~COMMAND_MASK);
    constexpr static unsigned CRC_LENGTH = 1;

public:
    MessageProcessor(uint8_t address, TxStream &tx_queue, RxQueue &rx_queue,
                     Memory &public_memory);

public:
    bool ProcessRx();
    hydrolib_ReturnCode TransmitWrite(uint8_t device_address,
                                      uint32_t memory_address, uint32_t length,
                                      uint8_t *data);
    hydrolib_ReturnCode TransmitRead(uint8_t device_address,
                                     uint32_t memory_address, uint32_t length,
                                     uint8_t *buffer);

private:
    bool MoveToHeader_();
    hydrolib_ReturnCode ParseHeader_();
    void ProcessCommand_();

    hydrolib_ReturnCode ParseMemoryAccessHeader_();
    hydrolib_ReturnCode ParseResponceHeader_();

    static uint8_t CRCfunc_(const uint8_t *buffer, uint16_t length);

private:
    constexpr static uint8_t CountAddress_(uint8_t user_friendly_address);

private:
    uint8_t self_address_;

    TxStream &tx_queue_;
    RxQueue &rx_queue_;

    unsigned current_rx_message_length_;
    unsigned current_rx_processed_length_;
    uint8_t current_rx_message_[MAX_MESSAGE_LENGTH];

    uint8_t responding_device_;
    uint8_t *responce_buffer_;
    unsigned responce_data_length_;

    Command_ current_command_;

    MessageHeader_ *current_header_;

    Memory &public_memory_;
};

template <concepts::stream::ByteStreamConcept TxStream,
          concepts::queue::ReadableByteQueue RxQueue,
          PublicMemoryConcept Memory>
MessageProcessor<TxStream, RxQueue, Memory>::MessageProcessor(
    uint8_t address, TxStream &tx_queue, RxQueue &rx_queue,
    Memory &public_memory)
    : tx_queue_(tx_queue),
      rx_queue_(rx_queue),
      current_rx_message_length_(0),
      current_rx_processed_length_(0),
      responce_buffer_(nullptr),
      current_header_(reinterpret_cast<MessageHeader_ *>(&current_rx_message_)),
      public_memory_(public_memory)
{
    if (address >= 1 << (ADDRESS_BITS_NUMBER + 1))
    {
        return;
    }
    self_address_ = CountAddress_(address);
}

template <concepts::stream::ByteStreamConcept TxStream,
          concepts::queue::ReadableByteQueue RxQueue,
          PublicMemoryConcept Memory>
bool MessageProcessor<TxStream, RxQueue, Memory>::ProcessRx()
{
    while (1)
    {
        if (current_rx_processed_length_ == 0)
        {
            bool header_searching_status = MoveToHeader_();
            if (!header_searching_status)
            {
                return false;
            }

            hydrolib_ReturnCode message_correct_check = ParseHeader_();
            switch (message_correct_check)
            {
            case HYDROLIB_RETURN_NO_DATA:
                return false;

            case HYDROLIB_RETURN_FAIL:
                rx_queue_.Drop(1);
                continue;

            default:
                break;
            }
        }

        hydrolib_ReturnCode read_status = rx_queue_.Read(
            current_rx_message_ + current_rx_processed_length_,
            current_rx_message_length_ - current_rx_processed_length_,
            current_rx_processed_length_);
        if (read_status != HYDROLIB_RETURN_OK)
        {
            return false;
        }
        uint8_t target_crc = CRCfunc_(current_rx_message_,
                                      current_rx_message_length_ - CRC_LENGTH);
        if (current_rx_message_[current_rx_message_length_ - CRC_LENGTH] ==
            target_crc)
        {
            rx_queue_.Drop(current_rx_message_length_);
        }
        else
        {
            rx_queue_.Drop(1);
            current_rx_message_length_ = 0;
            current_rx_processed_length_ = 0;
            continue;
        }

        ProcessCommand_();

        current_rx_message_length_ = 0;
        current_rx_processed_length_ = 0;
        return true;
    }
}

template <concepts::stream::ByteStreamConcept TxStream,
          concepts::queue::ReadableByteQueue RxQueue,
          PublicMemoryConcept Memory>
hydrolib_ReturnCode MessageProcessor<TxStream, RxQueue, Memory>::TransmitWrite(
    uint8_t device_address, uint32_t memory_address, uint32_t length,
    uint8_t *data)
{
    if (length == 0)
    {
        return HYDROLIB_RETURN_FAIL;
    }

    uint8_t current_tx_message_length =
        sizeof(typename MessageHeader_::MemoryAccess) + length + CRC_LENGTH;

    uint8_t current_tx_message[MAX_MESSAGE_LENGTH];
    typename MessageHeader_::MemoryAccess *tx_header =
        reinterpret_cast<typename MessageHeader_::MemoryAccess *>(
            &current_tx_message);

    tx_header->device_address =
        CountAddress_(device_address) | Command_::HYDROLIB_SP_COMMAND_WRITE;
    tx_header->self_address = self_address_;
    tx_header->memory_address = memory_address;
    tx_header->memory_access_length = length;

    memcpy(current_tx_message + sizeof(typename MessageHeader_::MemoryAccess),
           data, length);

    current_tx_message[current_tx_message_length - CRC_LENGTH] =
        CRCfunc_(current_tx_message, current_tx_message_length - CRC_LENGTH);

    tx_queue_.Push(current_tx_message, current_tx_message_length);

    return HYDROLIB_RETURN_OK;
}

template <concepts::stream::ByteStreamConcept TxStream,
          concepts::queue::ReadableByteQueue RxQueue,
          PublicMemoryConcept Memory>
hydrolib_ReturnCode MessageProcessor<TxStream, RxQueue, Memory>::TransmitRead(
    uint8_t device_address, uint32_t memory_address, uint32_t length,
    uint8_t *buffer)
{
    if (responce_buffer_)
    {
        return HYDROLIB_RETURN_BUSY;
    }

    if (length == 0)
    {
        return HYDROLIB_RETURN_FAIL;
    }

    uint8_t current_tx_message_length =
        sizeof(typename MessageHeader_::MemoryAccess) + CRC_LENGTH;

    uint8_t current_tx_message[MAX_MESSAGE_LENGTH];
    typename MessageHeader_::MemoryAccess *tx_header =
        reinterpret_cast<typename MessageHeader_::MemoryAccess *>(
            &current_tx_message);

    tx_header->device_address =
        CountAddress_(device_address) | Command_::HYDROLIB_SP_COMMAND_READ;
    tx_header->self_address = self_address_;
    tx_header->memory_address = memory_address;
    tx_header->memory_access_length = length;

    current_tx_message[current_tx_message_length - CRC_LENGTH] =
        CRCfunc_(current_tx_message, current_tx_message_length - CRC_LENGTH);

    tx_queue_.Push(current_tx_message, current_tx_message_length);

    responce_buffer_ = buffer;
    responce_data_length_ = length;
    responding_device_ = CountAddress_(device_address);

    return HYDROLIB_RETURN_OK;
}

template <concepts::stream::ByteStreamConcept TxStream,
          concepts::queue::ReadableByteQueue RxQueue,
          PublicMemoryConcept Memory>
bool MessageProcessor<TxStream, RxQueue, Memory>::MoveToHeader_()
{
    uint16_t index = 0;
    hydrolib_ReturnCode finding_read_status =
        rx_queue_.Read(&current_rx_message_[0], sizeof(self_address_), index);
    while (finding_read_status == HYDROLIB_RETURN_OK)
    {
        if (current_rx_message_[0] ==
                (self_address_ | Command_::HYDROLIB_SP_COMMAND_WRITE) ||
            current_rx_message_[0] ==
                (self_address_ | Command_::HYDROLIB_SP_COMMAND_READ) ||
            current_rx_message_[0] ==
                (self_address_ | Command_::HYDROLIB_SP_COMMAND_RESPOND))
        {
            rx_queue_.Drop(index);
            return true;
        }
        else
        {
            index++;
        }
        finding_read_status = rx_queue_.Read(&current_rx_message_[0],
                                             sizeof(self_address_), index);
    }

    rx_queue_.Drop(index);
    return false;
}

template <concepts::stream::ByteStreamConcept TxStream,
          concepts::queue::ReadableByteQueue RxQueue,
          PublicMemoryConcept Memory>
hydrolib_ReturnCode MessageProcessor<TxStream, RxQueue, Memory>::ParseHeader_()
{
    current_command_ =
        static_cast<Command_>(current_rx_message_[0] & COMMAND_MASK);

    hydrolib_ReturnCode parse_header_status;
    switch (current_command_)
    {
    case Command_::HYDROLIB_SP_COMMAND_READ:
        parse_header_status = ParseMemoryAccessHeader_();
        if (parse_header_status != HYDROLIB_RETURN_OK)
        {
            return parse_header_status;
        }

        current_rx_message_length_ =
            sizeof(typename MessageHeader_::MemoryAccess) + CRC_LENGTH;
        current_rx_processed_length_ =
            sizeof(typename MessageHeader_::MemoryAccess);
        break;

    case Command_::HYDROLIB_SP_COMMAND_WRITE:
        parse_header_status = ParseMemoryAccessHeader_();
        if (parse_header_status != HYDROLIB_RETURN_OK)
        {
            return parse_header_status;
        }

        current_rx_message_length_ =
            sizeof(typename MessageHeader_::MemoryAccess) +
            current_header_->memory_access_header.memory_access_length +
            CRC_LENGTH;
        current_rx_processed_length_ =
            sizeof(typename MessageHeader_::MemoryAccess);
        break;

    case Command_::HYDROLIB_SP_COMMAND_RESPOND:
        parse_header_status = ParseResponceHeader_();
        if (parse_header_status != HYDROLIB_RETURN_OK)
        {
            return parse_header_status;
        }

        current_rx_message_length_ = sizeof(typename MessageHeader_::Responce) +
                                     responce_data_length_ + CRC_LENGTH;
        current_rx_processed_length_ =
            sizeof(typename MessageHeader_::Responce);
        break;

    default:
        return HYDROLIB_RETURN_FAIL;
    }

    return HYDROLIB_RETURN_OK;
}

template <concepts::stream::ByteStreamConcept TxStream,
          concepts::queue::ReadableByteQueue RxQueue,
          PublicMemoryConcept Memory>
hydrolib_ReturnCode
MessageProcessor<TxStream, RxQueue, Memory>::ParseMemoryAccessHeader_()
{
    hydrolib_ReturnCode read_status = rx_queue_.Read(
        current_rx_message_ + sizeof(self_address_),
        sizeof(typename MessageHeader_::MemoryAccess), sizeof(self_address_));
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
    if (current_access_border > public_memory_.Size())
    {
        return HYDROLIB_RETURN_FAIL;
    }

    return HYDROLIB_RETURN_OK;
}

template <concepts::stream::ByteStreamConcept TxStream,
          concepts::queue::ReadableByteQueue RxQueue,
          PublicMemoryConcept Memory>
hydrolib_ReturnCode
MessageProcessor<TxStream, RxQueue, Memory>::ParseResponceHeader_()
{
    if (!responce_buffer_)
    {
        return HYDROLIB_RETURN_FAIL;
    }

    hydrolib_ReturnCode read_status = rx_queue_.Read(
        current_rx_message_ + sizeof(self_address_),
        sizeof(typename MessageHeader_::Responce), sizeof(self_address_));

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

template <concepts::stream::ByteStreamConcept TxStream,
          concepts::queue::ReadableByteQueue RxQueue,
          PublicMemoryConcept Memory>
uint8_t
MessageProcessor<TxStream, RxQueue, Memory>::CRCfunc_(const uint8_t *buffer,
                                                      uint16_t length)
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

template <concepts::stream::ByteStreamConcept TxStream,
          concepts::queue::ReadableByteQueue RxQueue,
          PublicMemoryConcept Memory>
void MessageProcessor<TxStream, RxQueue, Memory>::ProcessCommand_()
{
    switch (current_command_)
    {
    case Command_::HYDROLIB_SP_COMMAND_WRITE:
        public_memory_.Write(
            current_rx_message_ + sizeof(typename MessageHeader_::MemoryAccess),
            current_header_->memory_access_header.memory_address,
            current_header_->memory_access_header.memory_access_length);
        break;

    case Command_::HYDROLIB_SP_COMMAND_READ:
    {
        uint8_t current_tx_message_length =
            sizeof(typename MessageHeader_::Responce) +
            current_header_->memory_access_header.memory_access_length +
            CRC_LENGTH;

        uint8_t current_tx_message[MAX_MESSAGE_LENGTH];
        typename MessageHeader_::Responce *tx_header =
            reinterpret_cast<typename MessageHeader_::Responce *>(
                &current_tx_message);

        tx_header->device_address =
            (current_header_->memory_access_header.self_address) |
            Command_::HYDROLIB_SP_COMMAND_RESPOND;
        tx_header->self_address = self_address_;

        public_memory_.Read(
            current_tx_message + sizeof(typename MessageHeader_::Responce),
            current_header_->memory_access_header.memory_address,
            current_header_->memory_access_header.memory_access_length);

        current_tx_message[current_tx_message_length - CRC_LENGTH] = CRCfunc_(
            current_tx_message, current_tx_message_length - CRC_LENGTH);

        tx_queue_.Push(current_tx_message, current_tx_message_length);
    }
    break;

    case Command_::HYDROLIB_SP_COMMAND_RESPOND:
    {
        memcpy(responce_buffer_,
               current_rx_message_ + sizeof(typename MessageHeader_::Responce),
               responce_data_length_);
        responce_buffer_ = nullptr;
    }
    break;

    default:
        break;
    }
}

template <concepts::stream::ByteStreamConcept TxStream,
          concepts::queue::ReadableByteQueue RxQueue,
          PublicMemoryConcept Memory>
constexpr uint8_t MessageProcessor<TxStream, RxQueue, Memory>::CountAddress_(
    uint8_t user_friendly_address)
{
    return user_friendly_address << (8 - ADDRESS_BITS_NUMBER);
}

/** @} */
} // namespace hydrolib::serial_protocol

#endif