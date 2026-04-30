#pragma once

#include <span>

#include "hydrolib_bus_datalink_message.hpp"

namespace hydrolib::bus::datalink {

template <typename Allocator>
class MessageData final {
 public:
  MessageData() = default;
  explicit MessageData(int length);
  MessageData(const MessageData& other) = delete;
  MessageData(MessageData&& other);
  MessageData& operator=(const MessageData& other) = delete;
  MessageData& operator=(MessageData&& other);
  ~MessageData();

  operator std::span<std::byte>(); // NOLINT
  operator std::span<const std::byte>() const; // NOLINT

 private:
  std::span<std::byte> data_;
};

template <typename Allocator>
struct MessageInfo final {
  AddressType src_address = std::byte(0);
  MessageData<Allocator> data;
};

template <typename Allocator>
inline MessageData<Allocator>::MessageData(int length)
    : data_(Allocator::allocate(length), length) {}

template <typename Allocator>
inline MessageData<Allocator>::MessageData(MessageData&& other)
    : data_(other.data_) {
  other.data_ = {};
}

template <typename Allocator>
inline MessageData<Allocator>& MessageData<Allocator>::operator=(
    MessageData&& other) {
  data_ = other.data_;
  other.data_ = {};
  return *this;
}

template <typename Allocator>
inline MessageData<Allocator>::~MessageData() {
  Allocator::deallocate(data_.data(), data_.size());
}

template <typename Allocator>
inline MessageData<Allocator>::operator std::span<std::byte>() {
  return data_;
}

template <typename Allocator>
inline MessageData<Allocator>::operator std::span<const std::byte>() const {
  return data_;
}

}  // namespace hydrolib::bus::datalink
