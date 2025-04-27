#ifndef HYDROLIB_FORMATABLE_STRTING_H_
#define HYDROLIB_FORMATABLE_STRTING_H_

#include <cstdint>
#include <cassert>
#include <concepts>

#include "hydrolib_common.h"

namespace hydrolib::strings
{
    template <typename T>
    concept ByteStreamConcept = requires(T stream, const uint8_t *source, std::size_t length) {
        { stream.Push(source, length) } -> std::same_as<hydrolib_ReturnCode>;
    };

    template <typename T>
    concept StringConsept = requires(T string) {
        { string.GetConstString() } -> std::same_as<const char *>;
        { string.GetLength() } -> std::convertible_to<std::size_t>;
    };

    class StaticFormatableString
    {
    public:
        static constexpr unsigned MAX_PARAMETRES_COUNT = 10;

    public:
        StaticFormatableString() : length_(0), param_count_(0) {};
        consteval StaticFormatableString(const char *string);

    public:
        const char *GetString() const;
        uint32_t GetLength() const;

        template <ByteStreamConcept DestType, typename... Ts>
        hydrolib_ReturnCode ToBytes(DestType &buffer, Ts... params) const;

    private:
        template <ByteStreamConcept DestType, typename... Ts>
        hydrolib_ReturnCode ToBytes_(DestType &buffer,
                                     unsigned next_param_index,
                                     unsigned translated_length,
                                     int param, Ts... others) const;

        template <ByteStreamConcept DestType, StringConsept String, typename... Ts>
        hydrolib_ReturnCode ToBytes_(DestType &buffer,
                                     unsigned next_param_index,
                                     unsigned translated_length,
                                     String param, Ts...) const;

        template <ByteStreamConcept DestType>
        hydrolib_ReturnCode ToBytes_(
            DestType &buffer,
            unsigned next_param_index,
            unsigned translated_length) const;

    private:
        static consteval unsigned CountLength_(const char *string);
        static consteval unsigned CountParametres_(const char *string);

    private:
        const char *string_;
        const unsigned length_;
        const unsigned param_count_;
        unsigned short param_pos_diffs_[MAX_PARAMETRES_COUNT];
    };

    consteval StaticFormatableString::StaticFormatableString(
        const char *string) : string_(string),
                              length_(CountLength_(string)),
                              param_count_(CountParametres_(string))
    {
        unsigned param_number = 0;
        unsigned last_param = 0;
        for (unsigned i = 0; i < length_; i++)
        {
            if (string_[i] == '{')
            {
                param_pos_diffs_[param_number] = i - last_param;
                param_number++;
            }
            else if (string_[i] == '}')
            {
                last_param = i + 1;
            }
        }
        for (unsigned i = param_number; i < MAX_PARAMETRES_COUNT; i++)
        {
            param_pos_diffs_[i] = 0;
        }
    }

    template <ByteStreamConcept DestType, typename... Ts>
    hydrolib_ReturnCode StaticFormatableString::ToBytes(DestType &buffer, Ts... params) const
    {
        // static_assert(sizeof...(params) == param_count_,
        //               "Not enough arguments for inserting to formatable string");
        if (sizeof...(params) != param_count_)
        {
            return HYDROLIB_RETURN_FAIL;
        }
        return ToBytes_(buffer, 0, 0, params...);
    }

    template <ByteStreamConcept DestType, typename... Ts>
    hydrolib_ReturnCode StaticFormatableString::ToBytes_(
        DestType &buffer,
        unsigned next_param_index,
        unsigned translated_length,
        int param, Ts... others) const
    {
        if (translated_length >= length_)
        {
            return HYDROLIB_RETURN_OK;
        }

        buffer.Push(reinterpret_cast<const uint8_t *>(string_ + translated_length),
                    param_pos_diffs_[next_param_index]);
        translated_length += param_pos_diffs_[next_param_index] + 2;
        next_param_index++;

        if (param < 0)
        {
            buffer.Push(reinterpret_cast<const uint8_t *>("-"), 1);
            param = -param;
        }

        unsigned digit = 1;
        while (digit <= static_cast<unsigned>(param))
        {
            digit *= 10;
        }
        digit /= 10;

        char symbol;

        while (digit != 0)
        {
            symbol = param / digit + '0';
            param %= digit;
            buffer.Push(reinterpret_cast<const uint8_t *>(&symbol), 1);
            digit /= 10;
        }

        return ToBytes_(buffer, next_param_index, translated_length, others...);
    }

    template <ByteStreamConcept DestType, StringConsept String, typename... Ts>
    hydrolib_ReturnCode StaticFormatableString::ToBytes_(
        DestType &buffer,
        unsigned next_param_index,
        unsigned translated_length,
        String param, Ts... others) const
    {
        if (translated_length >= length_)
        {
            return HYDROLIB_RETURN_OK;
        }
        if (param_pos_diffs_[next_param_index] == 0)
        {
            return HYDROLIB_RETURN_FAIL;
        }

        buffer.Push(reinterpret_cast<const uint8_t *>(string_ + translated_length),
                    param_pos_diffs_[next_param_index]);
        translated_length += param_pos_diffs_[next_param_index] + 2;
        next_param_index++;

        buffer.Push(reinterpret_cast<const uint8_t *>(param.GetConstString()),
                    param.GetLength());

        return ToBytes_(buffer, next_param_index, translated_length, others...);
    }

    template <ByteStreamConcept DestType>
    hydrolib_ReturnCode StaticFormatableString::ToBytes_(
        DestType &buffer,
        unsigned next_param_index,
        unsigned translated_length) const
    {
        (void)next_param_index;
        buffer.Push(reinterpret_cast<const uint8_t *>(string_ + translated_length),
                    length_ - translated_length);
        return HYDROLIB_RETURN_OK;
    }

    const char *StaticFormatableString::GetString() const
    {
        return string_;
    }

    uint32_t StaticFormatableString::GetLength() const
    {
        return length_;
    }

    consteval unsigned StaticFormatableString::CountLength_(const char *string)
    {
        unsigned length = 0;
        while (string[length] != '\0')
        {
            length++;
        }
        return length;
    }

    consteval unsigned StaticFormatableString::CountParametres_(const char *string)
    {
        unsigned param_number = 0;
        bool open_flag = false;
        unsigned length = 0;
        while (string[length] != '\0')
        {
            if (string[length] == '{')
            {
                assert(!open_flag); // TODO think about it
                open_flag = true;
                param_number++;
            }
            else if (string[length] == '}')
            {
                open_flag = false;
            }
            length++;
        }
        return param_number;
    }
}

#endif
