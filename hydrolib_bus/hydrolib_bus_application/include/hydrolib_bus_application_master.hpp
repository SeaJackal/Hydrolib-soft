#pragma once

#include <unistd.h>

#include <cstdint>
#include <cstring>

#include "hydrolib_bus_application_commands.hpp"
#include "hydrolib_log_macro.hpp"
#include "hydrolib_stream_concepts.hpp"

namespace hydrolib::bus::application {
template <concepts::stream::ByteFullStreamConcept TxRxStream, typename Logger>
class Master {
 public:
  constexpr Master(TxRxStream &stream, Logger &logger);
  Master(const Master &) = delete;
  Master(Master &&) = delete;
  Master &operator=(const Master &) = delete;
  Master &operator=(Master &&) = delete;
  ~Master() = default;

  bool Process();
  void Read(void *data, int address, int length);
  void Write(const void *data, int address, int length);

 private:
  TxRxStream &stream_;
  Logger &logger_;

  void *requested_data_ = nullptr;
  int requested_length_ = 0;

  ResponseMessageBuffer rx_buffer_{};
  MemoryAccessMessageBuffer tx_buffer_{};
};

template <concepts::stream::ByteFullStreamConcept TxRxStream, typename Logger>
constexpr Master<TxRxStream, Logger>::Master(TxRxStream &stream, Logger &logger)
    : stream_(stream), logger_(logger) {}

template <concepts::stream::ByteFullStreamConcept TxRxStream, typename Logger>
bool Master<TxRxStream, Logger>::Process() {
  if (requested_data_ == nullptr) {
    return false;
  }

  int read_length = read(stream_, &rx_buffer_, kMaxMessageLength);
  if (read_length == 0) {
    return false;
  }

  switch (rx_buffer_.command) {
    case Command::RESPONSE:
      if (requested_length_ + static_cast<int>(sizeof(Command)) !=
          read_length) {
        return false;
      }
      memcpy(requested_data_, static_cast<void *>(rx_buffer_.data),
             requested_length_);
      requested_data_ = nullptr;
      return true;
    case Command::ERROR:
    case Command::READ:
    case Command::WRITE:
    default:
      LOG_WARNING(logger_, "Wrong command");
      return false;
  }
}

template <concepts::stream::ByteFullStreamConcept TxRxStream, typename Logger>
void Master<TxRxStream, Logger>::Read(void *data, int address, int length) {
  requested_data_ = data;
  requested_length_ = length;

  tx_buffer_.header.command = Command::READ;
  tx_buffer_.header.info.address = address;
  tx_buffer_.header.info.length = length;

  write(stream_, &tx_buffer_, sizeof(MemoryAccessHeader));
}

template <concepts::stream::ByteFullStreamConcept TxRxStream, typename Logger>
void Master<TxRxStream, Logger>::Write(const void *data, int address,
                                       int length) {
  tx_buffer_.header.command = Command::WRITE;
  tx_buffer_.header.info.address = address;
  tx_buffer_.header.info.length = length;

  memcpy(static_cast<void *>(tx_buffer_.data), data, length);

  write(stream_, &tx_buffer_, sizeof(MemoryAccessHeader) + length);
}

}  // namespace hydrolib::bus::application
