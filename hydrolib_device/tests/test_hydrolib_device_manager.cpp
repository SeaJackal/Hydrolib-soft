#include <gtest/gtest.h>

#include <array>

#include "hydrolib_device_manager.hpp"

namespace {

using hydrolib::device::Device;
using hydrolib::device::DeviceManager;
using hydrolib::device::DeviceType;

TEST(DeviceManager, ReturnsDeviceByName) {
  Device alpha("alpha", DeviceType::STREAM);
  Device beta("beta", DeviceType::STREAM);

  DeviceManager manager{&alpha, &beta};

  EXPECT_EQ(&alpha, manager["alpha"]);
  EXPECT_EQ(&beta, manager["beta"]);
  EXPECT_EQ(nullptr, manager["gamma"]);
}

TEST(DeviceManager, IteratesOverDevicesInOrder) {
  Device alpha("alpha", DeviceType::STREAM);
  Device beta("beta", DeviceType::STREAM);
  Device gamma("gamma", DeviceType::STREAM);

  DeviceManager manager{&alpha, &beta, &gamma};

  std::array<Device *, 3> expected{&alpha, &beta, &gamma};
  int index = 0;
  for (auto it = manager.begin(); it != manager.end(); ++it) {
    ASSERT_LT(index, static_cast<int>(expected.size()));
    EXPECT_EQ(expected[index], *it);
    EXPECT_EQ(expected[index], it.operator->());
    ++index;
  }
  EXPECT_EQ(static_cast<int>(expected.size()), index);
}

TEST(DeviceManager, AllowsBackwardIteration) {
  Device alpha("alpha", DeviceType::STREAM);
  Device beta("beta", DeviceType::STREAM);

  DeviceManager manager({&alpha, &beta});

  auto it = manager.end();
  --it;
  EXPECT_EQ(&beta, *it);
  --it;
  EXPECT_EQ(&alpha, *it);
}

TEST(DeviceManager, SetsGlobalInstancePointer) {
  Device alpha("alpha", DeviceType::STREAM);

  hydrolib::device::g_device_manager = nullptr;
  DeviceManager manager({&alpha});
  EXPECT_EQ(&manager, hydrolib::device::g_device_manager);
}

}  // namespace
