#ifndef HYDROLIB_STRINGS_H_
#define HYDROLIB_STRINGS_H_

#include <cstdint>

namespace hydrolib::strings
{
    class StaticFormatableString
    {
    public:
        StaticFormatableString() {}
        consteval StaticFormatableString(char *format_string)
        {
            string_ = format_string;
            length_ = 0;
            while (string_[length_] != '\0')
            {
                length_++;
            }
        }

    public:
        char *GetString();
        uint32_t GetLength();

    private:
        char *string_;
        uint32_t length_;
    };

    template <int CAPACITY>
    class CString
    {
    public:
        constexpr CString(char *str) : length_(0)
        {
            while (str[length_])
            {
                string_[length_] = str[length_];
                length_++;
            }
        }

    public:
        char *GetString()
        {
            return string_;
        }

        uint32_t GetLength()
        {
            return length_;
        }

    private:
        char string_[CAPACITY];
        uint32_t length_;
    };
}

#endif
