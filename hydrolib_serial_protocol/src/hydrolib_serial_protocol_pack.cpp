#include "hydrolib_serial_protocol_pack.hpp"

namespace hydrolib::serialProtocol
{
    SerialProtocolHandler::RxQueue_::RxQueue_()
    {
        hydrolib_RingQueue_Init(&rx_queue, buffer, HYDROLIB_SP_RX_BUFFER_CAPACITY);
    }

    hydrolib_ReturnCode SerialProtocolHandler::RxQueue_::Read(void *buffer, uint32_t length, uint32_t shift) const
    {
        return hydrolib_RingQueue_Read(&rx_queue, buffer, length, shift);
    }

    void SerialProtocolHandler::RxQueue_::Drop(uint32_t number)
    {
        hydrolib_RingQueue_Drop(&rx_queue, number);
    }

    void SerialProtocolHandler::RxQueue_::Clear()
    {
        hydrolib_RingQueue_Clear(&rx_queue);
    }

    hydrolib_ReturnCode SerialProtocolHandler::RxQueue_::Push(void *data, uint32_t length)
    {
        return hydrolib_RingQueue_Push(&rx_queue, data, length);
    }

    SerialProtocolHandler::SerialProtocolHandler(uint8_t address,
                                                 hydrolib_SP_Interface_TransmitFunc transmit_func,
                                                 uint8_t *public_memory,
                                                 uint32_t public_memory_capacity) : MessageProcessor(address,
                                                                                                     transmit_func,
                                                                                                     rx_queue,
                                                                                                     public_memory,
                                                                                                     public_memory_capacity),
                                                                                    rx_queue() {};

    hydrolib_ReturnCode SerialProtocolHandler::Receive(void *buffer, uint32_t length)
    {
        return rx_queue.Push(buffer, length);
    }
}
