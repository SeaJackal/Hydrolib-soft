#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "hydrolib_bus_datalink_message.hpp"
#include "hydrolib_cobs.hpp"
#include "hydrolib_crc.hpp"
#include "hydrolib_log_macro.hpp"
#include "hydrolib_return_codes.hpp"
#include "hydrolib_stream_concepts.hpp"

namespace hydrolib::bus::datalink {
template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger>
class Deserializer final {
 public:
  constexpr Deserializer(AddressType address, RxStream& rx_stream,
                         Logger& logger);

  Deserializer(const Deserializer&) = delete;
  Deserializer(Deserializer&&) = delete;
  Deserializer& operator=(const Deserializer&) = delete;
  Deserializer& operator=(Deserializer&&) = delete;
  ~Deserializer() = default;

  ReturnCode Process();

  [[nodiscard]] AddressType GetSourceAddress() const;
  std::span<const std::byte> GetData();
  [[nodiscard]] int GetLostBytes() const;

 private:
  ReturnCode FindHeader();
  ReturnCode ParseHeader();

  bool CheckCRC();

  const AddressType self_address_;

  RxStream& rx_stream_;
  Logger& logger_;

  MessageBuffer first_rx_buffer_{};
  MessageBuffer second_rx_buffer_{};

  int current_processed_length_ = 0;
  MessageBuffer* current_rx_buffer_ = &first_rx_buffer_;
  MessageBuffer* next_rx_buffer_ = &second_rx_buffer_;
  bool message_ready_ = false;

  int lost_bytes_ = 0;
};

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger>
constexpr Deserializer<RxStream, Logger>::Deserializer(AddressType address,
                                                       RxStream& rx_stream,
                                                       Logger& logger)
    : self_address_(address), rx_stream_(rx_stream), logger_(logger) {}

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger>
ReturnCode Deserializer<RxStream, Logger>::Process() {
  while (true)  // TODO(vscode): bad practice
  {
    if (current_processed_length_ == 0) {
      ReturnCode res = FindHeader();
      if (res != ReturnCode::OK) {
        return res;
      }
    }

    if (current_processed_length_ < sizeof(MessageHeader)) {
      ReturnCode res = ParseHeader();
      if (res == ReturnCode::FAIL) {
        continue;
      }
      if (res != ReturnCode::OK) {
        return res;
      }
    }

    current_processed_length_ +=
        read(rx_stream_,
             &current_rx_buffer_->data_and_crc[(current_processed_length_ -
                                                sizeof(MessageHeader))],
             current_rx_buffer_->header.length - current_processed_length_);

    if (current_processed_length_ != current_rx_buffer_->header.length) {
      return ReturnCode::NO_DATA;
    }

    ReturnCode res = cobs::Decode<kMagicByte>(
        std::as_writable_bytes(std::span(current_rx_buffer_, 1))
            .subspan(offsetof(MessageBuffer, header.cobs_length),
                     current_rx_buffer_->header.length - sizeof(MessageHeader) +
                         sizeof(MessageHeader::cobs_length)));

    if (res == ReturnCode::OK) {
      if (CheckCRC()) {
        auto* temp = current_rx_buffer_;
        current_rx_buffer_ = next_rx_buffer_;
        next_rx_buffer_ = temp;
        message_ready_ = true;
        current_processed_length_ = 0;
        return ReturnCode::OK;
      }
      lost_bytes_ += current_processed_length_;
      current_processed_length_ = 0;

    } else {
      lost_bytes_ += current_processed_length_;
      LOG_WARNING(logger_, "COBS error, lost {} bytes",
                  current_processed_length_);
      current_processed_length_ = 0;
    }
  }
}

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger>
AddressType Deserializer<RxStream, Logger>::GetSourceAddress() const {
  if (message_ready_) {
    return next_rx_buffer_->header.src_address;
  }
  return std::byte(0);
}

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger>
std::span<const std::byte> Deserializer<RxStream, Logger>::GetData() {
  if (message_ready_) {
    message_ready_ = false;
    return {
        static_cast<std::byte*>(next_rx_buffer_->data_and_crc),
        next_rx_buffer_->header.length - sizeof(MessageHeader) - kCRCLength};
  }
  return {};
}

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger>
int Deserializer<RxStream, Logger>::GetLostBytes() const {
  return lost_bytes_;
}

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger>
ReturnCode Deserializer<RxStream, Logger>::FindHeader() {
  int res = read(rx_stream_, current_rx_buffer_, sizeof(kMagicByte));
  while (res != 0) {
    if (res < 0) {
      return ReturnCode::ERROR;
    }
    if (current_rx_buffer_->header.magic_byte == kMagicByte) {
      current_processed_length_ = sizeof(kMagicByte);
      return ReturnCode::OK;
    }
    lost_bytes_ += sizeof(kMagicByte);
    LOG_WARNING(logger_, "Rubbish byte");

    res = read(rx_stream_, current_rx_buffer_, sizeof(kMagicByte));
  }
  return ReturnCode::NO_DATA;
}

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger>
ReturnCode Deserializer<RxStream, Logger>::ParseHeader() {
  int res =
      read(rx_stream_,
           reinterpret_cast<uint8_t*>(&current_rx_buffer_->header) +  // NOLINT
               current_processed_length_,
           sizeof(MessageHeader) - current_processed_length_);
  if (res < 0) {
    return ReturnCode::ERROR;
  }
  current_processed_length_ += res;
  if (current_processed_length_ != sizeof(MessageHeader)) {
    return ReturnCode::NO_DATA;
  }
  if (current_rx_buffer_->header.dest_address != self_address_) {
    current_processed_length_ = 0;
    return ReturnCode::FAIL;
  }
  return ReturnCode::OK;
}

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger>
bool Deserializer<RxStream, Logger>::CheckCRC() {
  uint8_t target_crc =
      crc::CountCRC8(reinterpret_cast<uint8_t*>(current_rx_buffer_),
                     current_rx_buffer_->header.length - kCRCLength);

  auto current_crc = static_cast<uint8_t>(
      current_rx_buffer_->data_and_crc[current_rx_buffer_->header.length -
                                       sizeof(MessageHeader) - kCRCLength]);

  if (target_crc != current_crc) {
    LOG_WARNING(logger_, "Wrong CRC: expected {}, got {}", target_crc,
                current_crc);
    return false;
  }
  return true;
}

}  // namespace hydrolib::bus::datalink
