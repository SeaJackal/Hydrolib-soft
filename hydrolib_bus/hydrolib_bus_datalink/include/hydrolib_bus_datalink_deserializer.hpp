#pragma once

#include <cstddef>
#include <span>

#include "hydrolib_bus_datalink_message.hpp"
#include "hydrolib_bus_datalink_rx_info.hpp"
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

  Expected<RxInfo> Process();

  [[nodiscard]] int GetLostBytes() const;

 private:
  class RxReader;
  class Synchronizer;
  class MessageReader;
  enum class State { kSynchronizing, kReadingMessage };

  Synchronizer synchronizer_;
  MessageReader message_reader_;

  State current_state_ = State::kSynchronizing;

  int lost_bytes_ = 0;
};

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger>
class Deserializer<RxStream, Logger>::RxReader {
 public:
  explicit RxReader(RxStream& stream);
  RxReader(const RxReader&) = delete;
  RxReader(RxReader&&) = delete;
  RxReader& operator=(const RxReader&) = delete;
  RxReader& operator=(RxReader&&) = delete;
  ~RxReader() = default;

  void Start(std::span<std::byte> buffer);
  hydrolib::ReturnCode operator()();

 private:
  RxStream& stream_;
  std::span<std::byte> data_;
  int current_length_ = 0;
};

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger>
class Deserializer<RxStream, Logger>::Synchronizer {
 public:
  explicit Synchronizer(RxStream& stream, Logger& logger);
  Synchronizer(const Synchronizer&) = delete;
  Synchronizer(Synchronizer&&) = delete;
  Synchronizer& operator=(const Synchronizer&) = delete;
  Synchronizer& operator=(Synchronizer&&) = delete;
  ~Synchronizer() = default;

  hydrolib::ReturnCode operator()();

 private:
  Logger& logger_;
  RxStream& stream_;
};

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger>
class Deserializer<RxStream, Logger>::MessageReader {
 public:
  explicit MessageReader(RxStream& stream, AddressType address, Logger& logger);
  MessageReader(const MessageReader&) = delete;
  MessageReader(MessageReader&&) = delete;
  MessageReader& operator=(const MessageReader&) = delete;
  MessageReader& operator=(MessageReader&&) = delete;
  ~MessageReader() = default;

  Expected<RxInfo> operator()();

 private:
  enum class State { kReadingHeader, kReadingMessage, kReadingCheckSum };

  Logger& logger_;
  RxReader reader_;
  AddressType address_;

  State current_state_ = State::kReadingHeader;
  RxInfo current_rx_info_;
  MessageHeader current_header_{};
  std::byte current_crc_{};
};

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger>
constexpr Deserializer<RxStream, Logger>::Deserializer(AddressType address,
                                                       RxStream& rx_stream,
                                                       Logger& logger)
    : synchronizer_(rx_stream, logger),
      message_reader_(rx_stream, address, logger) {}

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger>
Expected<RxInfo> Deserializer<RxStream, Logger>::Process() {
  while (true) {
    switch (current_state_) {
      case State::kSynchronizing: {
        auto result = synchronizer_();
        if (result != ReturnCode::OK) {
          return result;
        }
        current_state_ = State::kReadingMessage;
      }
      case State::kReadingMessage: {
        auto result = message_reader_();
        if (static_cast<ReturnCode>(result) == ReturnCode::OK) {
          current_state_ = State::kSynchronizing;
          return result;
        }
        if (static_cast<ReturnCode>(result) != ReturnCode::FAIL) {
          return result;
        }
        current_state_ = State::kSynchronizing;
      }
    }
  }
}

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger>
int Deserializer<RxStream, Logger>::GetLostBytes() const {
  return lost_bytes_;
}

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger>
Deserializer<RxStream, Logger>::RxReader::RxReader(RxStream& stream)
    : stream_(stream) {}

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger>
void Deserializer<RxStream, Logger>::RxReader::Start(
    std::span<std::byte> buffer) {
  data_ = buffer;
  current_length_ = 0;
}

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger>
hydrolib::ReturnCode Deserializer<RxStream, Logger>::RxReader::operator()() {
  auto remaining_length =
      static_cast<int>(data_.size_bytes()) - current_length_;
  if (remaining_length <= 0) {
    return ReturnCode::OK;
  }
  auto read_length =
      read(stream_, data_.subspan(current_length_).data(), remaining_length);
  if (read_length < 0) {
    return ReturnCode::ERROR;
  }
  if (read_length == remaining_length) {
    return ReturnCode::OK;
  }
  current_length_ += read_length;
  return ReturnCode::NO_DATA;
}

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger>
Deserializer<RxStream, Logger>::Synchronizer::Synchronizer(RxStream& stream,
                                                           Logger& logger)
    : logger_(logger), stream_(stream) {}

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger>
hydrolib::ReturnCode
Deserializer<RxStream, Logger>::Synchronizer::operator()() {
  while (true) {
    std::byte byte_buffer{};
    auto read_length = read(stream_, &byte_buffer, 1);
    if (read_length < 0) {
      return ReturnCode::ERROR;
    }
    if (read_length == 0) {
      return ReturnCode::NO_DATA;
    }
    if (byte_buffer == kMagicByte) {
      return ReturnCode::OK;
    }
    LOG_WARNING(logger_, "Rubbish byte");
  }
}

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger>
Deserializer<RxStream, Logger>::MessageReader::MessageReader(
    RxStream& stream, AddressType address, Logger& logger)
    : logger_(logger), reader_(stream), address_(address) {
  reader_.Start(std::as_writable_bytes(std::span(&current_header_, 1)));
}

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger>
Expected<RxInfo> Deserializer<RxStream, Logger>::MessageReader::operator()() {
  while (true) {
    auto result = reader_();
    if (result != ReturnCode::OK) {
      return result;
    }
    switch (current_state_) {
      case State::kReadingHeader: {
        if (current_header_.dest_address != address_) {
          reader_.Start(std::as_writable_bytes(std::span(&current_header_, 1)));
          return ReturnCode::FAIL;
        }
        current_rx_info_ = RxInfo(current_header_.src_address,
                                  current_header_.length - sizeof(kMagicByte) -
                                      sizeof(MessageHeader) - kCRCLength);
        reader_.Start(current_rx_info_.GetData());
        current_state_ = State::kReadingMessage;
        break;
      }
      case State::kReadingMessage: {
        ReturnCode res = cobs::Decode<kMagicByte>(current_header_.cobs_length,
                                                  current_rx_info_.GetData());
        current_header_.cobs_length = 0;
        if (res != ReturnCode::OK) {
          LOG_WARNING(logger_, "COBS error");
          current_state_ = State::kReadingHeader;
          return ReturnCode::FAIL;
        }
        reader_.Start(std::span(&current_crc_, 1));
        current_state_ = State::kReadingCheckSum;
        break;
      }
      case State::kReadingCheckSum: {
        crc::CRC8 crc_counter;
        crc_counter.Next(kMagicByte);
        crc_counter.Next(
            std::as_writable_bytes(std::span(&current_header_, 1)));
        crc_counter.Next(current_rx_info_.GetData());
        auto target_crc = crc_counter.Get();

        current_state_ = State::kReadingHeader;
        reader_.Start(std::as_writable_bytes(std::span(&current_header_, 1)));
        if (target_crc != current_crc_) {
          LOG_WARNING(logger_, "Wrong CRC: expected {}, got {}",
                      static_cast<int>(target_crc),
                      static_cast<int>(current_crc_));
          return ReturnCode::FAIL;
        }
        return current_rx_info_;
      }
    }
  }

  return RxInfo(current_header_.src_address, current_header_.length);
}

}  // namespace hydrolib::bus::datalink
