/**
 @defgroup hydrolib_serial_protocol Hydrolib serial protocol
 \brief Protocol for communication throw duplex serial port

 Protocol uses algorithm for header finding to implement non-constant message length communication.
 It supports device public memory abstraction. It allows devices to interact with each other by
 writing into each other public memory.

 @{
*/

/**
 @file hydrolib_serial_protocol.h
 @author SeaJackal
 @brief Header file for hydrolib serial protocol
 @version 0.1
 @date 03-02-2025
 
 */
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

#define HYDROLIB_SP_TX_BUFFER_CAPACITY 2 * HYDROLIB_SP_MAX_MESSAGE_LENGTH
#define HYDROLIB_SP_RX_BUFFER_CAPACITY 2 * HYDROLIB_SP_MAX_MESSAGE_LENGTH + \
                                           HYDROLIB_SP_RX_BUFFER_RESERVE
                                           
typedef enum
{
    _HYDROLIB_SP_COMMAND_WRITE = 1,
    _HYDROLIB_SP_COMMAND_READ,
    _HYDROLIB_SP_COMMAND_RESPOND
} _hydrolib_SerialProtocol_Command;

#pragma pack(push, 1)

typedef struct
{
    uint8_t device_address;
    uint8_t self_address;
    uint8_t memory_address;
    uint8_t memory_access_length;
} _hydrolib_SP_MessageHeaderMemAccess;

typedef struct
{
    uint8_t device_address;
    uint8_t self_address;
} _hydrolib_SP_MessageHeaderResponce;

#pragma pack(pop)

/**
 @brief Type for byte read/write functions which library uses
 for hardware interactions
 
 */
typedef bool (*hydrolib_SerialProtocol_InterfaceFunc)(uint8_t *byte);
/**
 @brief Type for CRC8 function which library use to sugn messages
 
 */
typedef uint8_t (*hydrolib_SerialProtocol_CRCfunc)(const uint8_t *buffer, uint16_t length);

/**
 @brief Serial protocol manager handler structure which contains all information
 about session
 
 */
typedef struct
{
    uint8_t self_address;

    uint8_t rx_buffer[HYDROLIB_SP_RX_BUFFER_CAPACITY];
    uint8_t tx_buffer[HYDROLIB_SP_TX_BUFFER_CAPACITY];

    hydrolib_RingQueue rx_ring_buffer;
    hydrolib_RingQueue tx_ring_buffer;

    hydrolib_SerialProtocol_InterfaceFunc receive_byte_func;
    hydrolib_SerialProtocol_InterfaceFunc transmit_byte_func;

    hydrolib_SerialProtocol_CRCfunc get_crc_func;

    uint16_t current_rx_message_length;
    uint16_t current_rx_processed_length;
    uint8_t current_rx_message[HYDROLIB_SP_MAX_MESSAGE_LENGTH];

    uint8_t responcing_device;
    uint8_t* responce_buffer;
    uint8_t responce_data_length;

    _hydrolib_SerialProtocol_Command current_command;

    _hydrolib_SP_MessageHeaderMemAccess* header_rx_mem_access;
    _hydrolib_SP_MessageHeaderResponce* header_rx_responce;

    uint8_t *public_memory;
    uint16_t public_memory_capacity;
} hydrolib_SerialProtocolHandler;

/**
 @brief Initialization function for serial protocol handler
 
 @param[out] self Pointer to the initializing handler
 @param[in] address Device address which serial protocol will react to
 @param[in] receive_byte_func Hardware interaction function for byte receiving
 @param[in] transmit_byte_func Hardware interaction function for byte transmiting
 @param[in] get_crc_func Hardware interaction function for getting crc8
 @param[in] public_memory Pointer to the memory correspondent can interact with
 @param[in] public_memory_capacity Capacity of the public memory
 @return Returns HYDROLIB_RETURN_FAIL if some args are not valid
 and HYDROLIB_RETURN_OK else
 */
hydrolib_ReturnCode hydrolib_SerialProtocol_Init(hydrolib_SerialProtocolHandler *self, uint8_t address,
                                                    hydrolib_SerialProtocol_InterfaceFunc receive_byte_func,
                                                    hydrolib_SerialProtocol_InterfaceFunc transmit_byte_func,
                                                    hydrolib_SerialProtocol_CRCfunc get_crc_func,
                                                    uint8_t *public_memory,
                                                    uint16_t public_memory_capacity);

/**
 @brief Function for current messages processing
 
 @param[in,out] self Pointer to the serial protocol handler

 This function receives one byte if enable and transmits one byte of current message if enable each call.
 If there is valid message in receiving buffer it processes this message:
    - for write command payload will be writen into public memory
    - for read command pass
 */
void hydrolib_SerialProtocol_DoWork(hydrolib_SerialProtocolHandler *self);

/**
 @brief Writing to the corespondent's public memory
 
 @param[in,out] self Pointer to the serial protocol handler
 @param[in] device_address Corespondent's address
 @param[in] memory_address Memory address to write
 @param[in] length Length of payload
 @param[in] buffer Pointer to payload
 @return Returns HYDROLIB_RETURN_BUSY if last message is't transmit yet,
 HYDROLIB_RETURN_FAIL if there is a mistake in parametres and HYDROLIB_RETURN_OK else
 */
hydrolib_ReturnCode hydrolib_SerialProtocol_TransmitWrite(hydrolib_SerialProtocolHandler *self,
                                                          uint8_t device_address,
                                                          uint8_t memory_address, uint8_t length,
                                                          uint8_t *buffer);

/** @} */

#ifdef __cplusplus
}
#endif

#endif
