#include "hydrolib_strings.hpp"

#include <cstring>

namespace hydrolib::strings
{
    char *StaticFormatableString::GetString()
    {
        return string_;
    }

    uint32_t StaticFormatableString::GetLength()
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
