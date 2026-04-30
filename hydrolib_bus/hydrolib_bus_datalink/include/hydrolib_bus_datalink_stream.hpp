#pragma once

#include <array>
#include <cstddef>
#include <cstring>
#include <span>

#include "hydrolib_bus_datalink_deserializer.hpp"
#include "hydrolib_bus_datalink_message.hpp"
#include "hydrolib_bus_datalink_serializer.hpp"
#include "hydrolib_ring_queue.hpp"

namespace hydrolib::bus::datalink {
template <concepts::stream::ByteFullStreamConcept RxTxStream, typename Logger,
          AddressType... kMateAddresses>
class StreamManager final {
 public:
  template <AddressType kMateAddress>
  class Stream;

  constexpr StreamManager(AddressType self_address, RxTxStream& stream,
                          Logger& logger);
  StreamManager(const StreamManager&) = delete;
  StreamManager(StreamManager&&) = delete;
  StreamManager& operator=(const StreamManager&) = delete;
  StreamManager& operator=(StreamManager&&) = delete;
  ~StreamManager() = default;

  ReturnCode Process();
  [[nodiscard]] int GetLostPackages() const;

 private:
  using SerializerType = Serializer<RxTxStream, Logger>;
  using DeserializerType = Deserializer<RxTxStream, Logger>;
  class RxManager;

  DeserializerType deserializer_;
  SerializerType serializer_;

  RxManager rx_manager_;
};

template <concepts::stream::ByteFullStreamConcept RxTxStream, typename Logger,
          AddressType... kMateAddresses>
class StreamManager<RxTxStream, Logger, kMateAddresses...>::RxManager final {
 public:
  RxManager() = default;
  RxManager(const RxManager&) = delete;
  RxManager(RxManager&&) = delete;
  RxManager& operator=(const RxManager&) = delete;
  RxManager& operator=(RxManager&&) = delete;
  ~RxManager() = default;

  void Push(DeserializerType::CurrentMessageInfo info);
  std::span<std::byte> Pull(AddressType address, int length);

 private:
  struct RxMailbox {
    AddressType address{};
    ring_queue::RingQueue<kMaxDataLength>
        queue;  // TODO(sea_jackal): make normal queue with MessageData
  };

  std::array<RxMailbox, sizeof...(kMateAddresses)> mailboxes_{
      {kMateAddresses, ring_queue::RingQueue<kMaxDataLength>()}...};
  std::array<std::byte, kMaxDataLength>
      buffer_{};  // TODO(sea_jackal): remove after queue adding
};

template <concepts::stream::ByteFullStreamConcept RxTxStream, typename Logger,
          AddressType... kMateAddresses>
template <AddressType kMateAddress>
class StreamManager<RxTxStream, Logger, kMateAddresses...>::Stream final {
 public:
  constexpr explicit Stream(
      StreamManager<RxTxStream, Logger, kMateAddresses...>& stream_manager);
  Stream(const Stream&) = default;
  Stream(Stream&&) = default;
  Stream& operator=(const Stream&) = default;
  Stream& operator=(Stream&&) = default;
  ~Stream() = default;

  static constexpr bool kHydrolibBusDatalinkStreamMarker = true;

  int Read(std::span<std::byte> buffer);
  int Write(std::span<const std::byte> data);

 private:
  static constexpr bool IsAddressValid();

  StreamManager<RxTxStream, Logger, kMateAddresses...>* manager_ = nullptr;

  static_assert(IsAddressValid(), "Invalid mate address");
};

template <concepts::stream::ByteFullStreamConcept RxTxStream, typename Logger,
          AddressType... kMateAddresses>
constexpr StreamManager<RxTxStream, Logger, kMateAddresses...>::StreamManager(
    AddressType self_address, RxTxStream& stream, Logger& logger)
    : deserializer_(self_address, stream, logger),
      serializer_(self_address, stream, logger) {}

template <concepts::stream::ByteFullStreamConcept RxTxStream, typename Logger,
          AddressType... kMateAddresses>
ReturnCode StreamManager<RxTxStream, Logger, kMateAddresses...>::Process() {
  auto result = deserializer_.Process();
  if (result == ReturnCode::OK) {
    auto message = static_cast<DeserializerType::CurrentMessageInfo>(
        std::move(result));
    rx_manager_.Push(std::move(message));
    return ReturnCode::OK;
  }
  return result;
}

template <concepts::stream::ByteFullStreamConcept RxTxStream, typename Logger,
          AddressType... kMateAddresses>
int StreamManager<RxTxStream, Logger, kMateAddresses...>::GetLostPackages()
    const {
  return deserializer_.GetLostPackages();
}

template <concepts::stream::ByteFullStreamConcept RxTxStream, typename Logger,
          AddressType... kMateAddresses>
void StreamManager<RxTxStream, Logger, kMateAddresses...>::RxManager::Push(
    DeserializerType::CurrentMessageInfo info) {
  for (int i = 0; i < static_cast<int>(sizeof...(kMateAddresses)); i++) {
    if (mailboxes_[i].address == info.src_address) {
      auto data = static_cast<std::span<const std::byte>>(info.data);
      mailboxes_[i].queue.Push(data.data(), data.size());
      return;
    }
  }
}

template <concepts::stream::ByteFullStreamConcept RxTxStream, typename Logger,
          AddressType... kMateAddresses>
std::span<std::byte>
StreamManager<RxTxStream, Logger, kMateAddresses...>::RxManager::Pull(
    AddressType address, int length) {
  for (int i = 0; i < static_cast<int>(sizeof...(kMateAddresses)); i++) {
    if (mailboxes_[i].address == address) {
      length = std::min(length, mailboxes_[i].queue.GetLength());
      mailboxes_[i].queue.Pull(buffer_.data(), length);
      return std::span(buffer_).subspan(0, length);
    }
  }
  __builtin_unreachable();
}

template <concepts::stream::ByteFullStreamConcept RxTxStream, typename Logger,
          AddressType... kMateAddresses>
template <AddressType kMateAddress>
constexpr StreamManager<RxTxStream, Logger, kMateAddresses...>::Stream<
    kMateAddress>::Stream(StreamManager<RxTxStream, Logger, kMateAddresses...>&
                              stream_manager)
    : manager_(&stream_manager) {}

template <concepts::stream::ByteFullStreamConcept RxTxStream, typename Logger,
          AddressType... kMateAddresses>
template <AddressType kMateAddress>
int StreamManager<RxTxStream, Logger, kMateAddresses...>::Stream<
    kMateAddress>::Read(std::span<std::byte> buffer) {
  auto data = manager_->rx_manager_.Pull(kMateAddress, buffer.size());
  std::ranges::copy(data, buffer.begin());
  return data.size();
}

template <concepts::stream::ByteFullStreamConcept RxTxStream, typename Logger,
          AddressType... kMateAddresses>
template <AddressType kMateAddress>
int StreamManager<RxTxStream, Logger, kMateAddresses...>::Stream<
    kMateAddress>::Write(std::span<const std::byte> data) {
  auto result = manager_->serializer_.Process(kMateAddress, data);
  if (result == ReturnCode::OK) {
    return static_cast<int>(data.size());
  }
  return -1;
}

template <concepts::stream::ByteFullStreamConcept RxTxStream, typename Logger,
          AddressType... kMateAddresses>
template <AddressType kMateAddress>
constexpr bool StreamManager<RxTxStream, Logger, kMateAddresses...>::Stream<
    kMateAddress>::IsAddressValid() {
  std::array addresses = {kMateAddresses...};
  return std::ranges::find(addresses, kMateAddress) != addresses.end();
}

template <typename Stream>
  requires Stream::kHydrolibBusDatalinkStreamMarker
int read(Stream& stream, void* dest, unsigned length);
template <typename Stream>
  requires Stream::kHydrolibBusDatalinkStreamMarker
int write(Stream& stream, const void* src, unsigned length);

template <typename Stream>
  requires Stream::kHydrolibBusDatalinkStreamMarker
int read(Stream& stream, void* dest, unsigned length) {
  return stream.Read(std::span(static_cast<std::byte*>(dest), length));
}

template <typename Stream>
  requires Stream::kHydrolibBusDatalinkStreamMarker
int write(Stream& stream, const void* src, unsigned length) {
  return stream.Write(std::span(static_cast<const std::byte*>(src), length));
}

}  // namespace hydrolib::bus::datalink
