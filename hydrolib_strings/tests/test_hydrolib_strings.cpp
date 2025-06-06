#include "hydrolib_cstring.hpp"
#include "hydrolib_formatable_string.hpp"

#include <cstring>
#include <gtest/gtest.h>
#include <iostream>

using namespace hydrolib::strings;
using namespace std;

class BytesReceiver
{
public:
    BytesReceiver() : current_length(0) {}

public:
    hydrolib_ReturnCode Push(const void *source, std::size_t length)
    {
        memcpy(buffer + current_length, source, length);
        current_length += length;
        return HYDROLIB_RETURN_OK;
    }

public:
    uint8_t buffer[100];
    unsigned current_length;
};

TEST(TestHydrolibStrings, TestFormatableString)
{
    constexpr StaticFormatableString s("Inserting values: {}, {}, {}, {} End");
    BytesReceiver stream;
    s.ToBytes(stream, 1, 20, -33, CString<4>("haha"));
    stream.buffer[stream.current_length] = '\0';

    EXPECT_EQ(0, strcmp(reinterpret_cast<char *>(stream.buffer),
                        "Inserting values: 1, 20, -33, haha End"));
}
