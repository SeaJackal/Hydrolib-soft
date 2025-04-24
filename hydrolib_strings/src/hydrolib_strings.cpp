#include "hydrolib_strings.hpp"

#include <cstring>

namespace hydrolib::strings
{
    const char *StaticFormatableString::GetString() const
    {
        return string_;
    }

    uint32_t StaticFormatableString::GetLength() const
    {
        return length_;
    }

    // template <int CAPACITY>
    // char *CString<CAPACITY>::GetString()
    // {
    //     return string_;
    // }

    // template <int CAPACITY>
    // uint32_t CString<CAPACITY>::GetLength()
    // {
    //     return length_;
    // }
}
