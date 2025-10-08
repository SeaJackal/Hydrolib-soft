#include "hydrolib_fixed_point.hpp"
#include "hydrolib_iir.hpp"

#include <gtest/gtest.h>

TEST(TestFilter, IIRTest)
{
    hydrolib::filter::IIR<double, 1.0, 100.0> filter;
    for (int i = 0; i < 10000; i++)
    {
        double x_true = sin(i * 2 * 3.14 / 10000);
        double x_noized = x_true + sin(i * 2 * 3.14 / 10);
        double result = filter.Process(x_noized);
        // std::cout << x_noized << " " << result << " " << x_true << std::endl;
        EXPECT_NEAR(result, x_true, 0.1);
    }
}