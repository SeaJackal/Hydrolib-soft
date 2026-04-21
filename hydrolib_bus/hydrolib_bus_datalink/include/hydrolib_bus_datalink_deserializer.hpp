#pragma once

#include <cstddef>
#include <optional>
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

  Expected<MessageInfo> Process();

  [[nodiscard]] int GetLostPackages() const;

 private:
  class RxReader;
  class Synchronizer;
  class MessageReader;

  struct RxInfo {
    MessageHeader header{};
    MessageData data;
    std::byte crc{};
  };

  enum class State {
    kSynchronizing,
    kReadingMessage,
    kCheckingAddress,
    kDecodingMessage
  };

  static bool CheckAddress(MessageHeader header, AddressType self_address);
  static std::optional<MessageInfo> DecodeMessage(RxInfo info, Logger& logger);

  Logger& logger_;
  AddressType self_address_;

  Synchronizer synchronizer_;
  MessageReader message_reader_;

  State current_state_ = State::kSynchronizing;

  int lost_packages_ = 0;
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
  explicit MessageReader(RxStream& stream, Logger& logger);
  MessageReader(const MessageReader&) = delete;
  MessageReader(MessageReader&&) = delete;
  MessageReader& operator=(const MessageReader&) = delete;
  MessageReader& operator=(MessageReader&&) = delete;
  ~MessageReader() = default;

  Expected<RxInfo> operator()();

 private:
  enum class State {
    kStartReadingHeader,
    kReadingHeader,
    kStartReadingMessage,
    kReadingMessage,
    kStartReadingCheckSum,
    kReadingCheckSum
  };

  Logger& logger_;
  RxReader reader_;

  State current_state_ = State::kStartReadingHeader;
  RxInfo current_rx_info_;
};

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger>
constexpr Deserializer<RxStream, Logger>::Deserializer(AddressType address,
                                                       RxStream& rx_stream,
                                                       Logger& logger)
    : logger_(logger),
      self_address_(address),
      synchronizer_(rx_stream, logger),
      message_reader_(rx_stream, logger) {}

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger>
Expected<MessageInfo> Deserializer<RxStream, Logger>::Process() {
  RxInfo info;

  while (true) {
    switch (current_state_) {
      case State::kSynchronizing: {
        auto result = synchronizer_();
        if (result != ReturnCode::OK) {
          return result;
        }
        current_state_ = State::kReadingMessage;
        break;
      }
      case State::kReadingMessage: {
        auto result = message_reader_();
        if (static_cast<ReturnCode>(result) != ReturnCode::OK) {
          return static_cast<ReturnCode>(result);
        }
        current_state_ = State::kCheckingAddress;
        info = result;
        break;
      }
      case State::kCheckingAddress: {
        auto result = CheckAddress(info.header, self_address_);
        if (!result) {
          current_state_ = State::kSynchronizing;
        } else {
          current_state_ = State::kDecodingMessage;
        }
        break;
      }
      case State::kDecodingMessage: {
        auto result = DecodeMessage(info, logger_);
        current_state_ = State::kSynchronizing;
        if (!result) {
          lost_packages_++;
          break;
        }
        return *result;
      }
    }
  }
}

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger>
int Deserializer<RxStream, Logger>::GetLostPackages() const {
  return lost_packages_;
}

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger>
std::optional<MessageInfo> Deserializer<RxStream, Logger>::DecodeMessage(
    RxInfo info, Logger& logger) {
  ReturnCode res = cobs::Decode<kMagicByte>(info.header.cobs_length, info.data);
  info.header.cobs_length = 0;
  if (res != ReturnCode::OK) {
    LOG_WARNING(logger, "COBS error");
    return {};
  }

  crc::CRC8 crc_counter;
  crc_counter.Next(kMagicByte);
  crc_counter.Next(std::as_writable_bytes(std::span(&info.header, 1)));
  crc_counter.Next(info.data);
  auto target_crc = crc_counter.Get();

  if (target_crc != info.crc) {
    LOG_WARNING(logger, "Wrong CRC: expected {}, got {}",
                static_cast<int>(target_crc), static_cast<int>(info.crc));
    return {};
  }
  return MessageInfo{info.header.src_address, info.data};
}

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger>
bool Deserializer<RxStream, Logger>::CheckAddress(MessageHeader header,
                                                  AddressType self_address) {
  return header.dest_address == self_address;
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
  current_length_ += read_length;
  if (read_length == remaining_length) {
    return ReturnCode::OK;
  }
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
Deserializer<RxStream, Logger>::MessageReader::MessageReader(RxStream& stream,
                                                             Logger& logger)
    : logger_(logger), reader_(stream) {}

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger>
Expected<typename Deserializer<RxStream, Logger>::RxInfo>
Deserializer<RxStream, Logger>::MessageReader::operator()() {
  while (true) {
    auto result = reader_();
    if (result != ReturnCode::OK) {
      return result;
    }
    switch (current_state_) {
      case State::kStartReadingHeader: {
        reader_.Start(
            std::as_writable_bytes(std::span(&current_rx_info_.header, 1)));
        current_state_ = State::kReadingHeader;
        break;
      }
      case State::kReadingHeader: {
        if (current_rx_info_.header.length < kMinMessageLength ||
            current_rx_info_.header.length > kMaxMessageLength) {
          LOG_WARNING(logger_, "Wrong length: {}",
                      current_rx_info_.header.length);
          current_state_ = State::kStartReadingHeader;
          return ReturnCode::FAIL;
        }
        current_rx_info_.data =
            MessageData(current_rx_info_.header.length - sizeof(kMagicByte) -
                        sizeof(MessageHeader) - kCRCLength);
        current_state_ = State::kStartReadingMessage;
        break;
      }
      case State::kStartReadingMessage: {
        reader_.Start(current_rx_info_.data);
        current_state_ = State::kReadingMessage;
        break;
      }
      case State::kReadingMessage: {
        current_state_ = State::kStartReadingCheckSum;
        break;
      }
      case State::kStartReadingCheckSum: {
        reader_.Start(
            std::as_writable_bytes(std::span(&current_rx_info_.crc, 1)));
        current_state_ = State::kReadingCheckSum;
        break;
      }
      case State::kReadingCheckSum: {
        current_state_ = State::kStartReadingHeader;
        return current_rx_info_;
      }
    }
  }
}

}  // namespace hydrolib::bus::datalink
