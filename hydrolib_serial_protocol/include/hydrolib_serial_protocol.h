#ifndef HYDROLIB_SERIAL_PROTOCOL_H_
#define HYDROLIB_SERIAL_PROTOCOL_H_

#include <stdint.h>
#include <string.h>

#include <hydrolib_ring_queue.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define HYDROLIB_SP_MAX_MESSAGE_LENGTH 255
#define HYDROLIB_SP_RX_BUFFER_RESERVE 10

#define HYDROLIB_SP_TX_BUFFER_CAPACITY HYDROLIB_SP_MAX_MESSAGE_LENGTH
#define HYDROLIB_SP_RX_BUFFER_CAPACITY 2 * HYDROLIB_SP_MAX_MESSAGE_LENGTH + \
                                           HYDROLIB_SP_RX_BUFFER_RESERVE

typedef enum
{
    _HYDROLIB_SP_COMMAND_WRITE = 0,
    _HYDROLIB_SP_COMMAND_READ
} _hydrolib_SerialProtocol_Command;

#pragma pack(push, 1)

typedef struct
{
    uint8_t device_address;
    uint8_t memory_address;
    uint8_t memory_access_length;
} _hydrolib_SP_MessageHeaderMemAccess;

#pragma pack(pop)

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

    uint16_t current_rx_message_length;
    uint8_t current_rx_message[HYDROLIB_SP_MAX_MESSAGE_LENGTH];

    _hydrolib_SerialProtocol_Command current_command;

    _hydrolib_SP_MessageHeaderMemAccess* header_rx_mem_access;
    _hydrolib_SP_MessageHeaderMemAccess* header_tx_mem_access;

    uint16_t current_tx_message_length;
    uint8_t current_tx_message[HYDROLIB_SP_MAX_MESSAGE_LENGTH];
    uint16_t tx_pos;

    uint8_t *public_memory;
    uint16_t public_memory_capacity;
} hydrolib_SerialProtocolHandler;

hydrolib_ReturnCode hydrolib_SerialProtocol_Init(hydrolib_SerialProtocolHandler *self, uint8_t address,
                                                    hydrolib_SerialProtocol_InterfaceFunc receive_byte_func,
                                                    hydrolib_SerialProtocol_InterfaceFunc transmit_byte_func,
                                                    hydrolib_SerialProtocol_CRCfunc get_crc_func,
                                                    uint8_t *public_memory,
                                                    uint16_t public_memory_capacity);

void hydrolib_SerialProtocol_DoWork(hydrolib_SerialProtocolHandler *self);

hydrolib_ReturnCode hydrolib_SerialProtocol_TransmitWrite(hydrolib_SerialProtocolHandler *self,
                                                          uint8_t device_address,
                                                          uint8_t memory_address, uint8_t length,
                                                          uint8_t *buffer);

#ifdef __cplusplus
}
#endif

#endif
