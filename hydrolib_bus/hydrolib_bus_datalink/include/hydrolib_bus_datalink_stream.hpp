#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>

#include "hydrolib_bus_datalink_deserializer.hpp"
#include "hydrolib_bus_datalink_message.hpp"
#include "hydrolib_bus_datalink_serializer.hpp"
#include "hydrolib_log_macro.hpp"
#include "hydrolib_ring_queue.hpp"

namespace hydrolib::bus::datalink {
template <concepts::stream::ByteFullStreamConcept RxTxStream, typename Logger,
          int kMatesCount = 3>
class Stream;

template <concepts::stream::ByteFullStreamConcept RxTxStream, typename Logger,
          int kMatesCount = 3>
class StreamManager {
  friend class Stream<RxTxStream, Logger, kMatesCount>;

 public:
  constexpr StreamManager(AddressType self_address, RxTxStream& stream,
                          Logger& logger);
  StreamManager(const StreamManager&) = delete;
  StreamManager(StreamManager&&) = delete;
  StreamManager& operator=(const StreamManager&) = delete;
  StreamManager& operator=(StreamManager&&) = delete;
  ~StreamManager() = default;

  void Process();
  [[nodiscard]] int GetLostBytes() const;

 private:
  using SerializerType = Serializer<RxTxStream, Logger>;
  using DeserializerType = Deserializer<RxTxStream, Logger>;

  RxTxStream& stream_;
  Logger& logger_;

  const AddressType self_address_;

  DeserializerType deserializer_;
  SerializerType serializer_;

  std::array<Stream<RxTxStream, Logger, kMatesCount>*, kMatesCount> streams_{};
  int streams_count_ = 0;
};

template <concepts::stream::ByteFullStreamConcept RxTxStream, typename Logger,
          int kMatesCount>
class Stream {
  friend class StreamManager<RxTxStream, Logger, kMatesCount>;

 public:
  constexpr Stream(
      StreamManager<RxTxStream, Logger, kMatesCount>& stream_manager,
      AddressType mate_address);
  Stream(const Stream&) = delete;
  Stream(Stream&&) = delete;
  Stream& operator=(const Stream&) = delete;
  Stream& operator=(Stream&&) = delete;
  ~Stream() = default;

  int Read(std::span<std::byte> buffer);
  int Write(std::span<const std::byte> data);

 private:
  StreamManager<RxTxStream, Logger, kMatesCount>& stream_manager_;
  const AddressType mate_address_;

  ring_queue::RingQueue<kMaxMessageLength> buffer_;
};

template <concepts::stream::ByteFullStreamConcept RxTxStream, typename Logger,
          int kMatesCount>
constexpr StreamManager<RxTxStream, Logger, kMatesCount>::StreamManager(
    AddressType self_address, RxTxStream& stream, Logger& logger)
    : stream_(stream),
      logger_(logger),
      self_address_(self_address),
      deserializer_(self_address, stream, logger),
      serializer_(self_address, stream, logger) {}

template <concepts::stream::ByteFullStreamConcept RxTxStream, typename Logger,
          int kMatesCount>
void StreamManager<RxTxStream, Logger, kMatesCount>::Process() {
  ReturnCode result = deserializer_.Process();
  if (result == ReturnCode::OK) {
    AddressType message_source_address = deserializer_.GetSourceAddress();
    auto message_data = deserializer_.GetData();

    for (int i = 0; i < streams_count_; i++) {
      if (streams_[i]->mate_address_ == message_source_address) {
        auto push_result =
            streams_[i]->buffer_.Push(message_data.data(), message_data.size());
        if (push_result != ReturnCode::OK) {
          LOG_ERROR(logger_, "Stream overflow");
        }
        break;
      }
    }
  }
}

template <concepts::stream::ByteFullStreamConcept RxTxStream, typename Logger,
          int kMatesCount>
int StreamManager<RxTxStream, Logger, kMatesCount>::GetLostBytes() const {
  return deserializer_.GetLostBytes();
}

template <concepts::stream::ByteFullStreamConcept RxTxStream, typename Logger,
          int kMatesCount>
constexpr Stream<RxTxStream, Logger, kMatesCount>::Stream(
    StreamManager<RxTxStream, Logger, kMatesCount>& stream_manager,
    AddressType mate_address)
    : stream_manager_(stream_manager), mate_address_(mate_address) {
  stream_manager.streams_[stream_manager.streams_count_] = this;
  stream_manager.streams_count_++;
}

template <concepts::stream::ByteFullStreamConcept RxTxStream, typename Logger,
          int kMatesCount>
int Stream<RxTxStream, Logger, kMatesCount>::Read(std::span<std::byte> buffer) {
  int current_length = buffer_.GetLength();
  int length = std::min(static_cast<int>(buffer.size()), current_length);
  buffer_.Pull(buffer.data(), length);
  return length;
}

template <concepts::stream::ByteFullStreamConcept RxTxStream, typename Logger,
          int kMatesCount>
int Stream<RxTxStream, Logger, kMatesCount>::Write(
    std::span<const std::byte> data) {
  ReturnCode result = stream_manager_.serializer_.Process(mate_address_, data);

  if (result == ReturnCode::OK) {
    return static_cast<int>(data.size());
  }

  return 0;
}

template <hydrolib::concepts::stream::ByteFullStreamConcept RxTxStream,
          typename Logger, int kMatesCount = 3>
int read(typename hydrolib::bus::datalink::Stream<RxTxStream, Logger,
                                                  kMatesCount>& stream,
         void* dest, unsigned length);

template <hydrolib::concepts::stream::ByteFullStreamConcept RxTxStream,
          typename Logger, int kMatesCount = 3>
int write(typename hydrolib::bus::datalink::Stream<RxTxStream, Logger,
                                                   kMatesCount>& stream,
          const void* src, unsigned length);

template <hydrolib::concepts::stream::ByteFullStreamConcept RxTxStream,
          typename Logger, int kMatesCount>
int read(typename hydrolib::bus::datalink::Stream<RxTxStream, Logger,
                                                  kMatesCount>& stream,
         void* dest, unsigned length) {
  return stream.Read(std::span(static_cast<std::byte*>(dest), length));
}

template <hydrolib::concepts::stream::ByteFullStreamConcept RxTxStream,
          typename Logger, int kMatesCount>
int write(typename hydrolib::bus::datalink::Stream<RxTxStream, Logger,
                                                   kMatesCount>& stream,
          const void* src, unsigned length) {
  return stream.Write(std::span(static_cast<const std::byte*>(src), length));
}

}  // namespace hydrolib::bus::datalink
