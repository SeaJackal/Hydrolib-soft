#include <gtest/gtest.h>

#include "hydrolib_device.hpp"
#include "hydrolib_stream_device.hpp"

class TestStream {};

int read([[maybe_unused]] TestStream &stream, [[maybe_unused]] void *dest,
         [[maybe_unused]] unsigned length) {
  return 0;
}

int write([[maybe_unused]] TestStream &stream,
          [[maybe_unused]] const void *source,
          [[maybe_unused]] unsigned length) {
  return 0;
}

TEST(Device, StreamDevice) {
  TestStream stream = {};
  hydrolib::device::StreamDevice<TestStream> device("test", stream);
  hydrolib::device::Device *device_ptr = &device;
  EXPECT_EQ(
      device_ptr->Upcast<hydrolib::device::IStreamDevice>()->Read(nullptr, 0),
      0);
  EXPECT_EQ(
      device_ptr->Upcast<hydrolib::device::IStreamDevice>()->Read(nullptr, 0),
      0);
}
