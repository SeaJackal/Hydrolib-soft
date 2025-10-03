#include "hydrolib_fixed_point.hpp"
#include "hydrolib_imu_processor.hpp"

#include "mock/raw_imu_mock.hpp"

#include <gtest/gtest.h>

using namespace hydrolib::sensors;
using namespace hydrolib::math;

TEST(TestIMUProcessor, Process)
{
    RawIMUMock<FixedPointBase> imu_mock;
    Vector3D<FixedPointBase> axis{1, 1, 1};
    axis.Normalize();
    FixedPointBase angle_rad = FixedPointBase(40, 180) * pi;
    Quaternion<FixedPointBase> target{axis * sin(angle_rad / 2),
                                      cos(angle_rad / 2)};

    imu_mock.SetTarget(axis, angle_rad, 10);

    IMUProcessor<FixedPointBase, 1.0> imu_processor;

    Quaternion<FixedPointBase> result{0, 0, 0, 1};

    auto q = imu_mock.GetOrientation();

    while (imu_mock.Step())
    {
        q = imu_mock.GetOrientation();
        std::cout << static_cast<double>(q.x) << " " << static_cast<double>(q.y)
                  << " " << static_cast<double>(q.z) << " "
                  << static_cast<double>(q.w) << std::endl;
        result = imu_processor.Process(imu_mock.GetAcceleration(),
                                       imu_mock.GetGyro());
        std::cout << static_cast<double>(result.x) << " "
                  << static_cast<double>(result.y) << " "
                  << static_cast<double>(result.z) << " "
                  << static_cast<double>(result.w) << std::endl;
        std::cout << "===========" << std::endl;
    }

    q = imu_mock.GetOrientation();
    std::cout << static_cast<double>(target.x) << " "
              << static_cast<double>(target.y) << " "
              << static_cast<double>(target.z) << " "
              << static_cast<double>(target.w) << std::endl;

    EXPECT_NEAR(static_cast<double>(result.w), static_cast<double>(target.w),
                0.05);
    EXPECT_NEAR(static_cast<double>(result.x), static_cast<double>(target.x),
                0.05);
    EXPECT_NEAR(static_cast<double>(result.y), static_cast<double>(target.y),
                0.05);
    EXPECT_NEAR(static_cast<double>(result.z), static_cast<double>(target.z),
                0.05);
}