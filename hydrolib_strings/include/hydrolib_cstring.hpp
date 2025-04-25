#ifndef HYDROLIB_CSTRING_H_
#define HYDROLIB_CSTRING_H_

#include <cstdint>

#include "hydrolib_common.h"

namespace hydrolib::strings
{
    template <int CAPACITY>
    class CString
    {
    public:
        constexpr CString(const char *str) : length_(0) // TODO Add copy constructor
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

        const char *GetConstString() const
        {
            return string_;
        }

        uint32_t GetLength() const
        {
            return length_;
        }

    private:
        char string_[CAPACITY];
        uint32_t length_;
    };
}

#endif
