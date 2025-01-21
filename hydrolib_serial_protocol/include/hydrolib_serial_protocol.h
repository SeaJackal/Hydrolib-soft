#ifndef HYDROLIB_SERIAL_PROTOCOL_H_
#define HYDROLIB_SERIAL_PROTOCOL_H_

#include <stdint.h>
#include <string.h>

#include <hydrolib_ring_queue.h>

#ifdef __cplusplus
extern "C" {
#endif


#define HYDROLIB_SP_MAX_MESSAGE_LENGTH 10
#define HYDROLIB_SP_RX_BUFFER_RESERVE 10

#define HYDROLIB_SP_TX_BUFFER_CAPACITY HYDROLIB_SP_MAX_MESSAGE_LENGTH
#define HYDROLIB_SP_RX_BUFFER_CAPACITY 2 * HYDROLIB_SP_MAX_MESSAGE_LENGTH + \
                                           HYDROLIB_SP_RX_BUFFER_RESERVE

typedef bool (*hydrolib_SerialProtocol_InterfaceFunc)(uint8_t *byte);
typedef uint8_t (*hydrolib_SerialProtocol_CRCfunc)(const uint8_t *buffer, uint16_t length);

typedef struct
{
    uint8_t self_address;

    uint8_t rx_buffer[HYDROLIB_SP_RX_BUFFER_CAPACITY];
    uint8_t tx_buffer[HYDROLIB_SP_TX_BUFFER_CAPACITY];

    hydrolib_RingQueue rx_ring_buffer;

    hydrolib_SerialProtocol_InterfaceFunc receive_byte_func;
    hydrolib_SerialProtocol_InterfaceFunc transmit_byte_func;

    hydrolib_SerialProtocol_CRCfunc get_crc_func;

    uint16_t current_rx_data_length;
    uint8_t current_rx_data_buffer[HYDROLIB_SP_MAX_MESSAGE_LENGTH];
} hydrolib_SerialProtocolHandler;

void hydrolib_SerialProtocol_Init(hydrolib_SerialProtocolHandler *self, uint8_t address,
                                  hydrolib_SerialProtocol_InterfaceFunc receive_byte_func,
                                  hydrolib_SerialProtocol_InterfaceFunc transmit_byte_func,
                                  hydrolib_SerialProtocol_CRCfunc get_crc_func);

void hydrolib_SerialProtocol_DoWork(hydrolib_SerialProtocolHandler *self);

bool hydrolib_SerialProtocol_ReadMessage(hydrolib_SerialProtocolHandler *self, uint8_t *dest, uint8_t *data_length);

#ifdef __cplusplus
}
#endif

#endif
