#pragma once

#include <algorithm>
#include <cstring>
#include <span>

#include "hydrolib_bus_datalink_message.hpp"
#include "hydrolib_cobs.hpp"
#include "hydrolib_crc.hpp"
#include "hydrolib_return_codes.hpp"
#include "hydrolib_stream_concepts.hpp"

namespace hydrolib::bus::datalink {
template <concepts::stream::ByteWritableStreamConcept TxStream, typename Logger>
class Serializer final {
 public:
  constexpr Serializer(AddressType self_address, TxStream& tx_stream,
                       Logger& logger);
  Serializer(const Serializer&) = delete;
  Serializer(Serializer&&) = delete;
  Serializer& operator=(const Serializer&) = delete;
  Serializer& operator=(Serializer&&) = delete;
  ~Serializer() = default;

  ReturnCode Process(AddressType dest_address, std::span<const std::byte> data);

 private:
  const AddressType address_;
  TxStream& tx_stream_;
  Logger& logger_;

  MessageBuffer current_message_{};
};

template <concepts::stream::ByteWritableStreamConcept TxStream, typename Logger>
constexpr Serializer<TxStream, Logger>::Serializer(AddressType address,
                                                   TxStream& tx_stream,
                                                   Logger& logger)
    : address_(address), tx_stream_(tx_stream), logger_(logger) {
  current_message_.magic_byte = kMagicByte;
  current_message_.header.cobs_length = 0;
}

template <concepts::stream::ByteWritableStreamConcept TxStream, typename Logger>
ReturnCode Serializer<TxStream, Logger>::Process(
    AddressType dest_address, std::span<const std::byte> data) {
  current_message_.header.dest_address = dest_address;
  current_message_.header.src_address = address_;
  current_message_.header.cobs_length = 0;
  current_message_.header.length = static_cast<uint8_t>(
      sizeof(kMagicByte) + sizeof(MessageHeader) + data.size() + kCRCLength);
  std::ranges::copy(data,
                    static_cast<std::byte*>(current_message_.data_and_crc));
  crc::CRC8 crc8;
  crc8.Next(std::as_bytes(std::span(&current_message_, 1))
                .subspan(0, current_message_.header.length - kCRCLength));
  current_message_.data_and_crc[data.size()] = crc8.Get();

  current_message_.header.cobs_length = cobs::Encode<kMagicByte>(
      std::as_writable_bytes(std::span(&current_message_, 1))
          .subspan(
              offsetof(MessageBuffer, data_and_crc),
              current_message_.header.length -
                  offsetof(MessageBuffer, data_and_crc) -
                  kCRCLength));  // TODO(sea_jackal): need tests for specific
                                 // crc, crc = kMagicByte for example

  int res =
      write(tx_stream_, &current_message_, current_message_.header.length);
  if (res < 0) {
    return ReturnCode::ERROR;
  }
  if (res != static_cast<int>(current_message_.header.length)) {
    return ReturnCode::OVERFLOW;
  }
  return ReturnCode::OK;
}

}  // namespace hydrolib::bus::datalink
