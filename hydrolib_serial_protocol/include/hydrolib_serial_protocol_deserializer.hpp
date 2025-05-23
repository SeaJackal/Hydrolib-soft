#ifndef HYDROLIB_SERIAL_PROTOCOL_DESERIALIZER_H_
#define HYDROLIB_SERIAL_PROTOCOL_DESERIALIZER_H_

#include <cstdint>

#include "hydrolib_common.h"
#include "hydrolib_logger.hpp"
#include "hydrolib_queue_concepts.hpp"
#include "hydrolib_serial_protocol_commands.hpp"
#include "hydrolib_serial_protocol_message.hpp"
#include "hydrolib_stream_concepts.hpp"

namespace hydrolib::serial_protocol
{

template <concepts::stream::ByteReadableStreamConcept RxStream,
          logger::LogDistributorConcept Distributor>
class Deserializer
{
public:
    constexpr Deserializer(uint8_t address, RxStream &rx_stream,
                           logger::Logger<Distributor> &logger,
                           uint8_t network = 0xA0);

public:
    hydrolib_ReturnCode Process();

    Command GetCommand();
    CommandInfo GetInfo();

private:
    hydrolib_ReturnCode AimHeader_();
    hydrolib_ReturnCode ProcessCommonHeader_();
    hydrolib_ReturnCode ProcessMessage_();
    bool CheckCRC_();

private:
    logger::Logger<Distributor> &logger_;

    const uint8_t network_;
    const uint8_t self_readable_address_;

    const uint8_t self_address_;

    RxStream &rx_stream_;

    unsigned current_message_length_;
    unsigned current_processed_length_;
    uint8_t current_rx_message_[MessageHeader::MAX_MESSAGE_LENGTH];

    Command current_command_;

    MessageHeader *current_header_;
};

template <concepts::stream::ByteReadableStreamConcept RxStream,
          logger::LogDistributorConcept Distributor>
constexpr Deserializer<RxStream, Distributor>::Deserializer(
    uint8_t address, RxStream &rx_stream, logger::Logger<Distributor> &logger,
    uint8_t network)
    : logger_(logger),
      network_(network),
      self_readable_address_(address),
      self_address_(MessageHeader::GetTrueAddress(address, network)),
      rx_stream_(rx_stream),
      current_message_length_(0),
      current_processed_length_(0),
      current_command_(Command::RESPONCE),
      current_header_(reinterpret_cast<MessageHeader *>(&current_rx_message_))
{
    for (unsigned i = 0; i < MessageHeader::MAX_MESSAGE_LENGTH; i++)
    {
        current_rx_message_[i] = 0;
    }
}

template <concepts::stream::ByteReadableStreamConcept RxStream,
          logger::LogDistributorConcept Distributor>
Command Deserializer<RxStream, Distributor>::GetCommand()
{
    return static_cast<Command>(current_header_->common.command);
}

template <concepts::stream::ByteReadableStreamConcept RxStream,
          logger::LogDistributorConcept Distributor>
CommandInfo Deserializer<RxStream, Distributor>::GetInfo()
{
    CommandInfo info;
    switch (current_header_->common.command)
    {
    case Command::READ:
        info.read = {.source_address = MessageHeader::GetReadableAddress(
                         current_header_->memory_access.self_address),
                     .dest_address = self_readable_address_,
                     .memory_address =
                         current_header_->memory_access.memory_address,
                     .memory_access_length =
                         current_header_->memory_access.memory_access_length};
        break;
    case Command::WRITE:
        info.write = {
            .source_address = MessageHeader::GetReadableAddress(
                current_header_->memory_access.self_address),
            .dest_address = self_readable_address_,
            .memory_address = current_header_->memory_access.memory_address,
            .memory_access_length =
                current_header_->memory_access.memory_access_length,
            .data = current_rx_message_ + sizeof(MessageHeader::MemoryAccess)};
        break;
    case Command::RESPONCE:
        info.responce = {
            .source_address = MessageHeader::GetReadableAddress(
                current_header_->responce.self_address),
            .dest_address = self_readable_address_,
            .data_length = current_header_->responce.message_length -
                           static_cast<uint8_t>(sizeof(MessageHeader::Common)) -
                           MessageHeader::CRC_LENGTH,
            .data = current_rx_message_ + sizeof(MessageHeader::Common)};
        break;
    case Command::ERROR:
        info.error = {.source_address = MessageHeader::GetReadableAddress(
                          current_header_->error.self_address),
                      .dest_address = self_readable_address_,
                      .error_code = static_cast<ErrorCode>(
                          current_header_->error.error_code)};
        break;
    }
    return info;
}

template <concepts::stream::ByteReadableStreamConcept RxStream,
          logger::LogDistributorConcept Distributor>
hydrolib_ReturnCode Deserializer<RxStream, Distributor>::Process()
{
    bool message_found = false;
    while (!message_found)
    {
        if (current_processed_length_ < sizeof(self_address_))
        {
            hydrolib_ReturnCode header_search_res = AimHeader_();
            if (header_search_res != HYDROLIB_RETURN_OK)
            {
                return header_search_res;
            }
        }
        if (current_processed_length_ < sizeof(MessageHeader::Common))
        {
            hydrolib_ReturnCode header_process_res = ProcessCommonHeader_();
            if (header_process_res != HYDROLIB_RETURN_OK)
            {
                return header_process_res;
            }
        }
        hydrolib_ReturnCode message_process_res = ProcessMessage_();
        if (message_process_res == HYDROLIB_RETURN_FAIL)
        {
            continue;
        }
        if (message_process_res != HYDROLIB_RETURN_OK)
        {
            return message_process_res;
        }

        message_found = CheckCRC_();
    }

    rx_stream_.Drop(current_header_->common.message_length);

    return HYDROLIB_RETURN_OK;
}

template <concepts::stream::ByteReadableStreamConcept RxStream,
          logger::LogDistributorConcept Distributor>
hydrolib_ReturnCode Deserializer<RxStream, Distributor>::AimHeader_()
{
    unsigned index = 0;
    hydrolib_ReturnCode finding_read_status =
        rx_stream_.Read(&current_rx_message_[0], sizeof(self_address_), index);
    while (finding_read_status == HYDROLIB_RETURN_OK)
    {
        if (current_rx_message_[0] == self_address_)
        {
            rx_stream_.Drop(index);
            if (index)
            {
                logger_.WriteLog(logger::LogLevel::WARNING, "Rubbish bytes: {}",
                                 index);
            }
            current_processed_length_ = sizeof(self_address_);
            return HYDROLIB_RETURN_OK;
        }
        else
        {
            index++;
        }
        finding_read_status = rx_stream_.Read(&current_rx_message_[0],
                                              sizeof(self_address_), index);
    }

    if (finding_read_status != HYDROLIB_RETURN_NO_DATA)
    {
        return finding_read_status;
    }

    rx_stream_.Drop(index);
    logger_.WriteLog(logger::LogLevel::WARNING, "Rubbish bytes: {}", index);
    return HYDROLIB_RETURN_NO_DATA;
}

template <concepts::stream::ByteReadableStreamConcept RxStream,
          logger::LogDistributorConcept Distributor>
hydrolib_ReturnCode Deserializer<RxStream, Distributor>::ProcessCommonHeader_()
{
    hydrolib_ReturnCode read_res =
        rx_stream_.Read(&current_rx_message_[sizeof(self_address_)],
                        sizeof(MessageHeader::Common) - sizeof(self_address_),
                        sizeof(self_address_));

    if (read_res != HYDROLIB_RETURN_OK)
    {
        return read_res;
    }
    current_processed_length_ = sizeof(MessageHeader::Common);
    return HYDROLIB_RETURN_OK;
}

template <concepts::stream::ByteReadableStreamConcept RxStream,
          logger::LogDistributorConcept Distributor>
hydrolib_ReturnCode Deserializer<RxStream, Distributor>::ProcessMessage_()
{
    if (current_header_->common.message_length < sizeof(MessageHeader::Common))
    {
        logger_.WriteLog(logger::LogLevel::WARNING, "Wrong message length: ",
                         current_header_->common.message_length);
        rx_stream_.Drop(1);
        current_processed_length_ = 0;
        return HYDROLIB_RETURN_FAIL;
    }
    hydrolib_ReturnCode header_read_res = rx_stream_.Read(
        &current_rx_message_[sizeof(MessageHeader::Common)],
        current_header_->common.message_length - sizeof(MessageHeader::Common),
        sizeof(MessageHeader::Common));
    if (header_read_res != HYDROLIB_RETURN_OK)
    {
        return HYDROLIB_RETURN_NO_DATA;
    }
    if (current_header_->common.command == Command::WRITE)
    {
        unsigned target_length = current_header_->common.message_length -
                                 sizeof(MessageHeader::MemoryAccess) -
                                 MessageHeader::CRC_LENGTH;
        if (current_header_->memory_access.memory_access_length !=
            target_length)
        {
            logger_.WriteLog(
                logger::LogLevel::WARNING,
                "Wrong data length: expected {}, in header {}", target_length,
                current_header_->memory_access.memory_access_length);
            rx_stream_.Drop(1);
            current_processed_length_ = 0;
            return HYDROLIB_RETURN_FAIL;
        }
    }
    else if (current_header_->common.command != Command::READ &&
             current_header_->common.command != Command::RESPONCE &&
             current_header_->common.command != Command::ERROR)
    {
        logger_.WriteLog(
            logger::LogLevel::WARNING, "Wrong command: {}",
            static_cast<unsigned>(current_header_->common.command));
        rx_stream_.Drop(1);
        current_processed_length_ = 0;
        return HYDROLIB_RETURN_FAIL;
    }
    current_processed_length_ = 0;
    return HYDROLIB_RETURN_OK;
}

template <concepts::stream::ByteReadableStreamConcept RxStream,
          logger::LogDistributorConcept Distributor>
bool Deserializer<RxStream, Distributor>::CheckCRC_()
{
    uint8_t target_crc = MessageHeader::CountCRC(
        current_rx_message_,
        current_header_->common.message_length - MessageHeader::CRC_LENGTH);

    uint8_t current_crc =
        current_rx_message_[current_header_->common.message_length -
                            MessageHeader::CRC_LENGTH];

    if (target_crc != current_crc)
    {
        logger_.WriteLog(logger::LogLevel::WARNING,
                         "Wrong CRC: expected {}, got {}", target_crc,
                         current_crc);
        rx_stream_.Drop(1);
        return false;
    }
    return true;
}

} // namespace hydrolib::serial_protocol

#endif
