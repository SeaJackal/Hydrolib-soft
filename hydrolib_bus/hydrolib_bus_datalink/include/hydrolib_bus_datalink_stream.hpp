#pragma once

#include "hydrolib_bus_datalink_deserializer.hpp"
#include "hydrolib_bus_datalink_message.hpp"
#include "hydrolib_bus_datalink_serializer.hpp"
#include <cstring>

namespace hydrolib::bus::datalink
{
template <concepts::stream::ByteFullStreamConcept RxTxStream,
          logger::LogDistributorConcept Distributor>
class Stream
{
private:
    using SerializerType = Serializer<RxTxStream, Distributor>;
    using DeserializerType = Deserializer<RxTxStream, Distributor>;

public:
    constexpr Stream(SerializerType &serializer, DeserializerType &deserializer,
                     AddressType mate_address);

public:
    int Read(void *dest, unsigned length);
    int Write(const void *dest, unsigned length);

private:
    SerializerType &serializer_;
    DeserializerType &deserializer_;
    const AddressType mate_address_;
};

template <concepts::stream::ByteFullStreamConcept RxTxStream,
          logger::LogDistributorConcept Distributor>
constexpr Stream<RxTxStream, Distributor>::Stream(
    SerializerType &serializer, DeserializerType &deserializer,
    AddressType mate_address)
    : serializer_(serializer),
      deserializer_(deserializer),
      mate_address_(mate_address)
{
}

template <concepts::stream::ByteFullStreamConcept RxTxStream,
          logger::LogDistributorConcept Distributor>
int Stream<RxTxStream, Distributor>::Read(void *dest, unsigned length)
{
    deserializer_.Process();
    hydrolib::bus::datalink::AddressType src_address =
        deserializer_.GetSourceAddress();
    if (src_address == mate_address_)
    {
        unsigned received_length = deserializer_.GetDataLength();
        if (received_length < length)
        {
            length = received_length;
        }
        if (length)
        {
            memcpy(dest, deserializer_.GetData(), length);
        }
        return length;
    }
    return 0;
}

template <concepts::stream::ByteFullStreamConcept RxTxStream,
          logger::LogDistributorConcept Distributor>
int Stream<RxTxStream, Distributor>::Write(const void *dest, unsigned length)
{
    serializer_.Process(mate_address_, dest, length);
    return length;
}

} // namespace hydrolib::bus::datalink

template <hydrolib::concepts::stream::ByteFullStreamConcept RxTxStream,
          hydrolib::logger::LogDistributorConcept Distributor>
int read(hydrolib::bus::datalink::Stream<RxTxStream, Distributor> &stream,
         void *dest, unsigned length);

template <hydrolib::concepts::stream::ByteFullStreamConcept RxTxStream,
          hydrolib::logger::LogDistributorConcept Distributor>
int write(hydrolib::bus::datalink::Stream<RxTxStream, Distributor> &stream,
          const void *dest, unsigned length);

template <hydrolib::concepts::stream::ByteFullStreamConcept RxTxStream,
          hydrolib::logger::LogDistributorConcept Distributor>
int read(hydrolib::bus::datalink::Stream<RxTxStream, Distributor> &stream,
         void *dest, unsigned length)
{
    return stream.Read(dest, length);
}

template <hydrolib::concepts::stream::ByteFullStreamConcept RxTxStream,
          hydrolib::logger::LogDistributorConcept Distributor>
int write(hydrolib::bus::datalink::Stream<RxTxStream, Distributor> &stream,
          const void *dest, unsigned length)
{
    return stream.Write(dest, length);
}
