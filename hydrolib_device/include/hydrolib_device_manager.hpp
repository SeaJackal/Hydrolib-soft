#pragma once

#include <array>

#include "hydrolib_device.hpp"

namespace hydrolib::device {
class DeviceManager {
 public:
  class DeviceIterator {
    friend class DeviceManager;

   private:
    DeviceIterator(DeviceManager &device_manager, int index);

   public:
    Device *operator*();
    Device *operator->();
    DeviceIterator &operator++();
    DeviceIterator &operator--();
    bool operator==(const DeviceIterator &other);
    bool operator!=(const DeviceIterator &other);

   private:
    DeviceManager &device_manager_;
    int index_;
  };

 public:
  static constexpr int kMaxDevices = 20;

 public:
  DeviceManager(std::initializer_list<Device *> devices);

 public:
  Device *operator[](std::string_view name);
  DeviceIterator begin();
  DeviceIterator end();

 private:
  std::array<Device *, kMaxDevices> devices_ = {nullptr};
  int devices_count_ = 0;
};

inline DeviceManager *g_device_manager = nullptr;

inline DeviceManager::DeviceManager(std::initializer_list<Device *> devices)
    : devices_count_(devices.size()) {
  if (devices_count_ > kMaxDevices) {
    return;
  }
  std::copy(devices.begin(), devices.end(), devices_.begin());
  g_device_manager = this;
}

inline Device *DeviceManager::operator[](std::string_view name) {
  for (int i = 0; i < devices_count_; i++) {
    if (devices_[i]->GetName() == name) {
      return devices_[i];
    }
  }
  return nullptr;
}

inline DeviceManager::DeviceIterator DeviceManager::begin() {
  return DeviceIterator(*this, 0);
}

inline DeviceManager::DeviceIterator DeviceManager::end() {
  return DeviceIterator(*this, devices_count_);
}

inline DeviceManager::DeviceIterator::DeviceIterator(
    DeviceManager &device_manager, int index)
    : device_manager_(device_manager), index_(index) {}

inline Device *DeviceManager::DeviceIterator::operator*() {
  return device_manager_.devices_[index_];
}

inline Device *DeviceManager::DeviceIterator::operator->() {
  return device_manager_.devices_[index_];
}

inline DeviceManager::DeviceIterator &
DeviceManager::DeviceIterator::operator++() {
  index_++;
  return *this;
}

inline DeviceManager::DeviceIterator &
DeviceManager::DeviceIterator::operator--() {
  index_--;
  return *this;
}

inline bool DeviceManager::DeviceIterator::operator==(
    const DeviceIterator &other) {
  return index_ == other.index_;
}

inline bool DeviceManager::DeviceIterator::operator!=(
    const DeviceIterator &other) {
  return index_ != other.index_;
}
}  // namespace hydrolib::device