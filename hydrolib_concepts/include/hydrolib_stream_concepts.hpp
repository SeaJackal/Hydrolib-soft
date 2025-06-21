#ifndef HYDROLIB_STREAM_CONCEPTS_H_
#define HYDROLIB_STREAM_CONCEPTS_H_

#include "hydrolib_common.h"

#include <concepts>
#include <cstdint>

namespace hydrolib::concepts::stream
{

template <typename T>
concept ByteWritableStreamConceptPOSIX =
    requires(T stream, const void *source, unsigned length) {
        { write(stream, source, length) } -> std::convertible_to<int>;
    };

template <typename T>
concept ByteWritableStreamConceptMethod =
    requires(T stream, const void *source, unsigned length) {
        { stream.Push(source, length) } -> std::same_as<hydrolib_ReturnCode>;
    };

template <typename T>
concept ByteWritableStreamConcept =
    ByteWritableStreamConceptPOSIX<T> || ByteWritableStreamConceptMethod<T>;

template <typename T>
concept ByteReadableStreamConcept =
    requires(T stream, void *dest, unsigned length) {
        { read(stream, dest, length) } -> std::convertible_to<int>;
    };

template <typename T>
concept ByteFullStreamConcept =
    ByteWritableStreamConcept<T> && ByteReadableStreamConcept<T>;
} // namespace hydrolib::concepts::stream

#endif
