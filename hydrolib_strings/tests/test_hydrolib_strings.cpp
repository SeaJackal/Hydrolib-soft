#include <gtest/gtest.h>

#include <string>

#include "hydrolib_cstring.hpp"
#include "hydrolib_fixed_point.hpp"
#include "hydrolib_formatable_string.hpp"
#include "mock_stream.hpp"

using namespace hydrolib::strings;
using namespace hydrolib::math;
using namespace std;

TEST(TestHydrolibStrings, TestFormatableString) {
  constexpr StaticFormatableString<int, int, int, CString<4>, FixedPointBase> s(
      "Inserting values: {}, {}, {}, {}, {} End");
  hydrolib::streams::mock::MockByteStream stream;
  s.ToBytes(stream, 1, 20, -33, CString<4>("haha"), 1.5_fp);
  string output;
  output.resize(stream.GetSize());
  for (size_t i = 0; i < output.size(); ++i) {
    output[i] = static_cast<char>(stream[i]);
  }

  EXPECT_EQ(output, "Inserting values: 1, 20, -33, haha, 1.500 End");
}
