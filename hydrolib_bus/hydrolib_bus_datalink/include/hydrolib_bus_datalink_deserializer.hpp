#pragma once

#include <cstddef>
#include <optional>
#include <span>

#include "hydrolib_bus_datalink_message.hpp"
#include "hydrolib_bus_datalink_rx_info.hpp"
#include "hydrolib_cobs.hpp"
#include "hydrolib_crc.hpp"
#include "hydrolib_dummy_allocator.hpp"
#include "hydrolib_log_macro.hpp"
#include "hydrolib_return_codes.hpp"
#include "hydrolib_stream_concepts.hpp"

namespace hydrolib::bus::datalink {

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger,
          typename Allocator = DummyAllocator<kMaxDataLength, 2>>
class Deserializer final {
 public:
  using CurrentMessageInfo = MessageInfo<Allocator>;

  constexpr Deserializer(AddressType address, RxStream& rx_stream,
                         Logger& logger);

  Deserializer(const Deserializer&) = delete;
  Deserializer(Deserializer&&) = delete;
  Deserializer& operator=(const Deserializer&) = delete;
  Deserializer& operator=(Deserializer&&) = delete;
  ~Deserializer() = default;

  Expected<CurrentMessageInfo> Process();

  [[nodiscard]] int GetLostPackages() const;

 private:
  class RxReader;
  class Synchronizer;
  class MessageReader;

  struct RxInfo {
    MessageHeader header{};
    MessageData<Allocator> data;
    std::byte crc{};
  };

  enum class State { kSynchronizing, kReadingMessage };

  static bool CheckAddress(MessageHeader header, AddressType self_address);
  static std::optional<CurrentMessageInfo> DecodeMessage(RxInfo info,
                                                         Logger& logger);

  Logger& logger_;
  AddressType self_address_;

  Synchronizer synchronizer_;
  MessageReader message_reader_;

  State current_state_ = State::kSynchronizing;

  int lost_packages_ = 0;
};

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger,
          typename Allocator>
class Deserializer<RxStream, Logger, Allocator>::RxReader final {
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

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger,
          typename Allocator>
class Deserializer<RxStream, Logger, Allocator>::Synchronizer final {
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

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger,
          typename Allocator>
class Deserializer<RxStream, Logger, Allocator>::MessageReader final {
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

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger,
          typename Allocator>
constexpr Deserializer<RxStream, Logger, Allocator>::Deserializer(
    AddressType address, RxStream& rx_stream, Logger& logger)
    : logger_(logger),
      self_address_(address),
      synchronizer_(rx_stream, logger),
      message_reader_(rx_stream, logger) {}

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger,
          typename Allocator>
Expected<typename Deserializer<RxStream, Logger, Allocator>::CurrentMessageInfo>
Deserializer<RxStream, Logger, Allocator>::Process() {
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
        auto read_result = message_reader_();
        if (static_cast<ReturnCode>(read_result) != ReturnCode::OK) {
          return static_cast<ReturnCode>(read_result);
        }
        RxInfo info = std::move(read_result);

        auto check_result = CheckAddress(info.header, self_address_);
        if (!check_result) {
          current_state_ = State::kSynchronizing;
          break;
        }

        auto decode_result = DecodeMessage(std::move(info), logger_);
        current_state_ = State::kSynchronizing;
        if (!decode_result) {
          lost_packages_++;
          break;
        }
        return std::move(*decode_result);
      }
    }
  }
}

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger,
          typename Allocator>
int Deserializer<RxStream, Logger, Allocator>::GetLostPackages() const {
  return lost_packages_;
}

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger,
          typename Allocator>
std::optional<
    typename Deserializer<RxStream, Logger, Allocator>::CurrentMessageInfo>
Deserializer<RxStream, Logger, Allocator>::DecodeMessage(RxInfo info,
                                                         Logger& logger) {
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
  return CurrentMessageInfo{info.header.src_address, std::move(info.data)};
}

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger,
          typename Allocator>
bool Deserializer<RxStream, Logger, Allocator>::CheckAddress(
    MessageHeader header, AddressType self_address) {
  return header.dest_address == self_address;
}

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger,
          typename Allocator>
Deserializer<RxStream, Logger, Allocator>::RxReader::RxReader(RxStream& stream)
    : stream_(stream) {}

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger,
          typename Allocator>
void Deserializer<RxStream, Logger, Allocator>::RxReader::Start(
    std::span<std::byte> buffer) {
  data_ = buffer;
  current_length_ = 0;
}

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger,
          typename Allocator>
hydrolib::ReturnCode
Deserializer<RxStream, Logger, Allocator>::RxReader::operator()() {
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

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger,
          typename Allocator>
Deserializer<RxStream, Logger, Allocator>::Synchronizer::Synchronizer(
    RxStream& stream, Logger& logger)
    : logger_(logger), stream_(stream) {}

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger,
          typename Allocator>
hydrolib::ReturnCode
Deserializer<RxStream, Logger, Allocator>::Synchronizer::operator()() {
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

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger,
          typename Allocator>
Deserializer<RxStream, Logger, Allocator>::MessageReader::MessageReader(
    RxStream& stream, Logger& logger)
    : logger_(logger), reader_(stream) {}

template <concepts::stream::ByteReadableStreamConcept RxStream, typename Logger,
          typename Allocator>
Expected<typename Deserializer<RxStream, Logger, Allocator>::RxInfo>
Deserializer<RxStream, Logger, Allocator>::MessageReader::operator()() {
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
        current_rx_info_.data = MessageData<Allocator>(
            current_rx_info_.header.length - sizeof(kMagicByte) -
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
        return std::move(current_rx_info_);
      }
    }
  }
}

}  // namespace hydrolib::bus::datalink
