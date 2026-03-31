#pragma once

#include <cstring>
#include <span>

#include "hydrolib_bus_application_commands.hpp"
#include "hydrolib_log_macro.hpp"
#include "hydrolib_return_codes.hpp"
#include "hydrolib_stream_concepts.hpp"

namespace hydrolib::bus::application {
template <typename T>
concept PublicMemoryConcept =
    requires(T mem, std::span<std::byte> read_buffer,
             std::span<const std::byte> write_buffer, unsigned address) {
      { mem.Read(read_buffer, address) } -> std::same_as<ReturnCode>;

      { mem.Write(write_buffer, address) } -> std::same_as<ReturnCode>;
    };

template <PublicMemoryConcept Memory, typename Logger,
          concepts::stream::ByteFullStreamConcept TxRxStream>
class Slave {
 public:
  constexpr Slave(TxRxStream& stream, Memory& memory, Logger& logger);
  Slave(const Slave&) = delete;
  Slave(Slave&&) = delete;
  Slave& operator=(const Slave&) = delete;
  Slave& operator=(Slave&&) = delete;
  ~Slave() = default;

  void Process();

 private:
  TxRxStream& stream_;
  Memory& memory_;

  Logger& logger_;

  MemoryAccessMessageBuffer rx_buffer_{};
  ResponseMessageBuffer tx_buffer_{};
};

template <PublicMemoryConcept Memory, typename Logger,
          concepts::stream::ByteFullStreamConcept TxRxStream>
constexpr Slave<Memory, Logger, TxRxStream>::Slave(TxRxStream& stream,
                                                   Memory& memory,
                                                   Logger& logger)
    : stream_(stream), memory_(memory), logger_(logger) {}

template <PublicMemoryConcept Memory, typename Logger,
          concepts::stream::ByteFullStreamConcept TxRxStream>
void Slave<Memory, Logger, TxRxStream>::Process() {
  int read_length = read(stream_, &rx_buffer_, kMaxMessageLength);
  if (read_length == 0) {
    return;
  }

  switch (rx_buffer_.header.command) {
    case Command::kRead: {
      ReturnCode res = memory_.Read(
          std::span<std::byte>(static_cast<std::byte*>(tx_buffer_.data),
                               rx_buffer_.header.info.length),
          rx_buffer_.header.info.address);
      if (res == ReturnCode::OK) {
        LOG_INFO(logger_, "Transmitting {} bytes from {}",
                 rx_buffer_.header.info.length, rx_buffer_.header.info.address);
        tx_buffer_.command = Command::kResponse;
        write(stream_, &tx_buffer_,
              sizeof(Command) + rx_buffer_.header.info.length);
      } else {
        LOG_WARNING(logger_, "Can't read {} bytes from {}",
                    rx_buffer_.header.info.length,
                    rx_buffer_.header.info.address);
        tx_buffer_.command = Command::kError;
        write(stream_, &tx_buffer_, sizeof(Command));
      }
      break;
    }
    case Command::kWrite: {
      ReturnCode res =
          memory_.Write(std::span<const std::byte>(
                            static_cast<const std::byte*>(rx_buffer_.data),
                            rx_buffer_.header.info.length),
                        rx_buffer_.header.info.address);
      if (res != ReturnCode::OK) {
        LOG_WARNING(logger_, "Can't write {} bytes to {}",
                    rx_buffer_.header.info.length,
                    rx_buffer_.header.info.address);
        tx_buffer_.command = Command::kError;
        write(stream_, &tx_buffer_, sizeof(Command));
      } else {
        LOG_INFO(logger_, "Wrote {} bytes to {}", rx_buffer_.header.info.length,
                 rx_buffer_.header.info.address);
      }
    } break;
    case Command::kError:
    case Command::kResponse:
    default:
      LOG_WARNING(logger_, "Wrong command");
      tx_buffer_.command = Command::kError;
      write(stream_, &tx_buffer_, sizeof(Command));
      break;
  }
}
}  // namespace hydrolib::bus::application
