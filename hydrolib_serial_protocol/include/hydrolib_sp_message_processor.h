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
#ifndef HYDROLIB_SP_MESSAGE_PROCESSOR_H_
#define HYDROLIB_SP_MESSAGE_PROCESSOR_H_

#include <stdint.h>

#include "hydrolib_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define HYDROLIB_SP_MAX_MESSAGE_LENGTH 255

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
    typedef void (*hydrolib_SP_Interface_TransmitFunc)(uint8_t *data, uint32_t length);

    typedef hydrolib_ReturnCode (*hydrolib_SP_Interface_QueueReadFunc)(void *queue_data, void *data,
                                                                       uint32_t length, uint32_t shift);
    typedef hydrolib_ReturnCode (*hydrolib_SP_Interface_QueueDropFunc)(void *queue_data, uint32_t number);
    typedef hydrolib_ReturnCode (*hydrolib_SP_Interface_QueueClearFunc)(void *queue_data);

    /**
     @brief Serial protocol manager handler structure which contains all information
        about session

        */
    typedef struct
    {
        uint8_t self_address;

        hydrolib_SP_Interface_TransmitFunc transmit_func;

        void *rx_queue_data;
        hydrolib_SP_Interface_QueueReadFunc rx_read_func;
        hydrolib_SP_Interface_QueueDropFunc rx_drop_func;
        hydrolib_SP_Interface_QueueClearFunc rx_clear_func;

        uint16_t current_rx_message_length;
        uint16_t current_rx_processed_length;
        uint8_t current_rx_message[HYDROLIB_SP_MAX_MESSAGE_LENGTH];

        uint8_t responding_device;
        uint8_t *responce_buffer;
        uint8_t responce_data_length;

        _hydrolib_SerialProtocol_Command current_command;

        _hydrolib_SP_MessageHeaderMemAccess *header_rx_mem_access;
        _hydrolib_SP_MessageHeaderResponce *header_rx_responce;

        uint8_t *public_memory;
        uint16_t public_memory_capacity;
    } hydrolib_SP_MessageProcessor;

    hydrolib_ReturnCode hydrolib_SP_InitProcessor(hydrolib_SP_MessageProcessor *self, uint8_t address,
                                                  hydrolib_SP_Interface_TransmitFunc transmit_func,
                                                  void *rx_queue_data,
                                                  hydrolib_SP_Interface_QueueReadFunc rx_read_func,
                                                  hydrolib_SP_Interface_QueueDropFunc rx_drop_func,
                                                  hydrolib_SP_Interface_QueueClearFunc rx_clear_func,
                                                  uint8_t *public_memory,
                                                  uint16_t public_memory_capacity);

    uint32_t hydrolib_SP_ProcessRx(hydrolib_SP_MessageProcessor *self);

    hydrolib_ReturnCode hydrolib_SP_TransmitWrite(hydrolib_SP_MessageProcessor *self,
                                                  uint8_t device_address,
                                                  uint8_t memory_address, uint8_t length,
                                                  uint8_t *data);

    hydrolib_ReturnCode hydrolib_SP_TransmitRead(hydrolib_SP_MessageProcessor *self,
                                                 uint8_t device_address,
                                                 uint8_t memory_address, uint8_t length,
                                                 uint8_t *buffer);

    /** @} */

#ifdef __cplusplus
}
#endif

#endif