#pragma once

#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <span>

#include "hydrolib_bus_application_commands.hpp"
#include "hydrolib_log_macro.hpp"
#include "hydrolib_return_codes.hpp"
#include "hydrolib_stream_concepts.hpp"

namespace hydrolib::bus::application {
using namespace std::literals::chrono_literals;

template <concepts::stream::ByteFullStreamConcept TxRxStream, typename Logger>
class Master {
 public:
  static constexpr auto kRequestTimeout = 1s;

  constexpr Master(TxRxStream& stream, Logger& logger);
  Master(const Master&) = delete;
  Master(Master&&) = delete;
  Master& operator=(const Master&) = delete;
  Master& operator=(Master&&) = delete;
  ~Master() = default;

  hydrolib::ReturnCode Process();
  void RequestRead(const std::span<std::byte>& data, int address);
  void RequestWrite(std::span<const std::byte> data, int address);

 private:
  TxRxStream& stream_;
  Logger& logger_;

  std::span<std::byte> requested_data_;

  ResponseMessageBuffer rx_buffer_{};
  MemoryAccessMessageBuffer tx_buffer_{};

  std::chrono::steady_clock::time_point last_request_time_;
};

template <concepts::stream::ByteFullStreamConcept TxRxStream, typename Logger>
constexpr Master<TxRxStream, Logger>::Master(TxRxStream& stream, Logger& logger)
    : stream_(stream), logger_(logger) {}

template <concepts::stream::ByteFullStreamConcept TxRxStream, typename Logger>
hydrolib::ReturnCode Master<TxRxStream, Logger>::Process() {
  if (requested_data_.empty()) {
    return hydrolib::ReturnCode::FAIL;
  }  // TODO: vscode - fix FAIL after Write
     // https://app.weeek.net/ws/701833/task/1066

  if (std::chrono::steady_clock::now() - last_request_time_ > kRequestTimeout) {
    LOG_ERROR(logger_, "Request timeout");
    write(stream_, &tx_buffer_, sizeof(MemoryAccessHeader));
    last_request_time_ = std::chrono::steady_clock::now();
    return hydrolib::ReturnCode::TIMEOUT;
  }

  int read_length = read(stream_, &rx_buffer_, kMaxMessageLength);
  if (read_length == 0) {
    return hydrolib::ReturnCode::NO_DATA;
  }

  switch (rx_buffer_.command) {
    case Command::kResponse:
      if (static_cast<int>(requested_data_.size() + sizeof(Command)) !=
          read_length) {  // TODO: vscode
                          // - https://app.weeek.net/ws/701833/task/1067
        return hydrolib::ReturnCode::ERROR;
      }
      std::ranges::copy(
          std::span<std::byte>(static_cast<std::byte*>(rx_buffer_.data),
                               requested_data_.size()),
          requested_data_.begin());
      requested_data_ = {};
      return hydrolib::ReturnCode::OK;
    case Command::kError:  // TODO: vscode - make different reaction for
                           // different errors
    case Command::kRead:
    case Command::kWrite:
    default:
      LOG_WARNING(logger_, "Wrong command");
      return hydrolib::ReturnCode::ERROR;
  }
}

template <concepts::stream::ByteFullStreamConcept TxRxStream, typename Logger>
void Master<TxRxStream, Logger>::RequestRead(const std::span<std::byte>& data,
                                             int address) {
  requested_data_ = data;

  tx_buffer_.header.command = Command::kRead;
  tx_buffer_.header.info.address = address;
  tx_buffer_.header.info.length = data.size();

  write(stream_, &tx_buffer_, sizeof(MemoryAccessHeader));
  last_request_time_ = std::chrono::steady_clock::now();
}

template <concepts::stream::ByteFullStreamConcept TxRxStream, typename Logger>
void Master<TxRxStream, Logger>::RequestWrite(std::span<const std::byte> data,
                                              int address) {
  tx_buffer_.header.command = Command::kWrite;
  tx_buffer_.header.info.address = address;
  tx_buffer_.header.info.length = data.size();

  std::ranges::copy(data.begin(), data.end(),
                    static_cast<std::byte*>(tx_buffer_.data));

  write(stream_, &tx_buffer_, sizeof(MemoryAccessHeader) + data.size());
}

}  // namespace hydrolib::bus::application
