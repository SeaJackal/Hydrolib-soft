#ifndef HYDROLIB_SERIAL_PROTOCOL_PACK_H_
#define HYDROLIB_SERIAL_PROTOCOL_PACK_H_

#include "hydrolib_serial_protocol_core.hpp"
#include "hydrolib_ring_queue.h"
#include "hydrolib_common.h"

#define HYDROLIB_SP_RX_BUFFER_RESERVE 10

#define HYDROLIB_SP_TX_BUFFER_CAPACITY 2 * HYDROLIB_SP_MAX_MESSAGE_LENGTH
#define HYDROLIB_SP_RX_BUFFER_CAPACITY HYDROLIB_SP_MAX_MESSAGE_LENGTH + \
                                           HYDROLIB_SP_RX_BUFFER_RESERVE

namespace hydrolib::serialProtocol
{
    class SerialProtocolHandler : public MessageProcessor
    {
    private:
        class RxQueue_ : public RxQueueInterface
        {
        public:
            RxQueue_();

        private:
            hydrolib_RingQueue rx_queue;
            uint8_t buffer[HYDROLIB_SP_RX_BUFFER_CAPACITY];

        public:
            hydrolib_ReturnCode Read(void *buffer, uint32_t length, uint32_t shift) const override;
            void Drop(uint32_t number) override;
            void Clear() override;

            hydrolib_ReturnCode Push(void *data, uint32_t length);
        };

        class TxQueue_ : public TxQueueInterface
        {
        public:
            TxQueue_();

        private:
            hydrolib_RingQueue rx_queue;
            uint8_t buffer[HYDROLIB_SP_RX_BUFFER_CAPACITY];

        public:
            hydrolib_ReturnCode Push(void *data, uint32_t length) override;
        };

    public:
        SerialProtocolHandler(uint8_t address,
                              TxQueueInterface &tx_queue,
                              PublicMemoryInterface &public_memory);

    private:
        RxQueue_ rx_queue;

    public:
        hydrolib_ReturnCode Receive(void *buffer, uint32_t length);
    };

}

#endif
