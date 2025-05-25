#ifndef HYDROLIB_STREAM_CONCEPTS_H_
#define HYDROLIB_STREAM_CONCEPTS_H_

#include "hydrolib_common.h"

#include <concepts>
#include <cstdint>

namespace hydrolib::concepts::stream
{
template <typename T>
concept ByteStreamConcept =
    requires(T stream, const uint8_t *source, std::size_t length) {
        { stream.Push(source, length) } -> std::same_as<hydrolib_ReturnCode>;
    };

template <typename T>
concept ByteReadableStreamConcept =
    requires(T stream, void *dest, unsigned length) {
        { read(stream, dest, length) } -> std::same_as<int>;
    };
} // namespace hydrolib::concepts::stream

#endif
