#include <stdint.h>

#include <hydrolib_ring_queue.h>

#define MAX_MESSAGE_LENGTH 10
#define RX_BUFFER_RESERVE 10

#define TX_BUFFER_CAPACITY MAX_MESSAGE_LENGTH
#define RX_BUFFER_CAPACITY 2 * MAX_MESSAGE_LENGTH + RX_BUFFER_RESERVE

typedef struct
{
    uint8_t self_address;

    uint8_t rx_buffer[RX_BUFFER_CAPACITY];
    uint8_t tx_buffer[TX_BUFFER_CAPACITY];

    hydrolib_RingQueue rx_ring_buffer;
} hydrolib_SerialProtocolHandler;

void hydrolib_SerialProtocol_Init(hydrolib_SerialProtocolHandler* self, uint8_t address)
{
    self->self_address = address;
    hydrolib_RingBuffer_Init(&self->rx_ring_buffer, self->rx_buffer, RX_BUFFER_CAPACITY);
}

void hydrolib_SerialProtocol_DoWork(hydrolib_SerialProtocolHandler* self)
{
    
}
