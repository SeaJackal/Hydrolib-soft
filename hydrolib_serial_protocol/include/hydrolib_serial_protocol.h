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

#include "hydrolib_common.h"
#include "hydrolib_sp_message_processor.h"
#include "hydrolib_ring_queue.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define HYDROLIB_SP_RX_BUFFER_RESERVE 10

#define HYDROLIB_SP_TX_BUFFER_CAPACITY 2 * HYDROLIB_SP_MAX_MESSAGE_LENGTH
#define HYDROLIB_SP_RX_BUFFER_CAPACITY HYDROLIB_SP_MAX_MESSAGE_LENGTH + \
                                           HYDROLIB_SP_RX_BUFFER_RESERVE

    /**
     @brief Type for byte read/write functions which library uses
        for hardware interactions

        */
    typedef void (*hydrolib_SP_Interface_TransmitFunc)(uint8_t *data, uint32_t length);

    /**
     @brief Serial protocol manager handler structure which contains all information
        about session

        */
    typedef struct
    {
        uint8_t rx_buffer[HYDROLIB_SP_RX_BUFFER_CAPACITY];
        uint8_t tx_buffer[HYDROLIB_SP_TX_BUFFER_CAPACITY];

        hydrolib_RingQueue rx_ring_buffer;
        hydrolib_RingQueue tx_ring_buffer;

        bool got_rx;

        hydrolib_SP_MessageProcessor processor;
    } hydrolib_SerialProtocolHandler;

    /**
     @brief Initialization function for serial protocol handler

        @param[out] self Pointer to the initializing handler
        @param[in] address Device address which serial protocol will react to
        @param[in] transmit_func Hardware interaction function for byte transmiting
        @param[in] public_memory Pointer to the memory correspondent can interact with
        @param[in] public_memory_capacity Capacity of the public memory
        @return Returns HYDROLIB_RETURN_FAIL if some args are not valid
        and HYDROLIB_RETURN_OK else
        */
    hydrolib_ReturnCode hydrolib_SerialProtocol_Init(hydrolib_SerialProtocolHandler *self, uint8_t address,
                                                     hydrolib_SP_Interface_TransmitFunc transmit_func,
                                                     void *rx_queue_data,
                                                     hydrolib_SP_Interface_QueueReadFunc rx_read_func,
                                                     hydrolib_SP_Interface_QueueDropFunc rx_drop_func,
                                                     hydrolib_SP_Interface_QueueClearFunc rx_clear_func,
                                                     uint8_t *public_memory,
                                                     uint16_t public_memory_capacity);
    /**
     @brief Function for current messages processing

        @param[in,out] self Pointer to the serial protocol handler

        This function transmits one byte of current message if enable each call.
        If there is valid message in receiving buffer it processes this message:
        - for write command payload will be writen into public memory
        - for read command pass
        */
    void hydrolib_SerialProtocol_DoWork(hydrolib_SerialProtocolHandler *self);

    /**
     @brief Writing to the corespondent's public memory

        @param[in,out] self Pointer to the serial protocol handler
        @param[in] data Received data for the processing
        @param[in] length Length of received data
        @return Returns HYDROLIB_RETURN_BUSY if last message is't transmit yet,
        HYDROLIB_RETURN_FAIL if there is a mistake in parametres and HYDROLIB_RETURN_OK else
        */
    hydrolib_ReturnCode hydrolib_SerialProtocol_Receive(hydrolib_SerialProtocolHandler *self,
                                                        const void *data, uint8_t length);

    /**
     @brief Writing to the corespondent's public memory

        @param[in,out] self Pointer to the serial protocol handler
        @param[in] device_address Corespondent's address
        @param[in] memory_address Memory address to write
        @param[in] length Length of payload
        @param[in] data Pointer to payload
        @return Returns HYDROLIB_RETURN_BUSY if last message is't transmit yet,
        HYDROLIB_RETURN_FAIL if there is a mistake in parametres and HYDROLIB_RETURN_OK else
        */
    hydrolib_ReturnCode hydrolib_SerialProtocol_TransmitWrite(hydrolib_SerialProtocolHandler *self,
                                                              uint8_t device_address,
                                                              uint8_t memory_address, uint8_t length,
                                                              uint8_t *data);

    hydrolib_ReturnCode hydrolib_SerialProtocol_TransmitRead(hydrolib_SerialProtocolHandler *self,
                                                             uint8_t device_address,
                                                             uint8_t memory_address, uint8_t length,
                                                             uint8_t *buffer);

/**
 @brief Function for current messages processing
 
 @param[in,out] self Pointer to the serial protocol handler

 This function transmits one byte of current message if enable each call.
 If there is valid message in receiving buffer it processes this message:
    - for write command payload will be writen into public memory
    - for read command pass
 */
void hydrolib_SerialProtocol_DoWork(hydrolib_SerialProtocolHandler *self);

/**
 @brief Putting data to receivers buffer
 
 @param[in,out] self Pointer to the serial protocol handler
 @param[in] data Received data for the processing
 @param[in] length Length of received data
 @return Returns HYDROLIB_RETURN_BUSY if last message is't processed yet and HYDROLIB_RETURN_OK else
 */
hydrolib_ReturnCode hydrolib_SerialProtocol_Receive(hydrolib_SerialProtocolHandler *self,
                                                    const void* data, uint8_t length);

/**
 @brief Writing to the corespondent's public memory
 
 @param[in,out] self Pointer to the serial protocol handler
 @param[in] device_address Corespondent's address
 @param[in] memory_address Memory address to write
 @param[in] length Length of payload
 @param[in] data Pointer to payload
 @return Returns HYDROLIB_RETURN_BUSY if last message is't transmit yet,
 HYDROLIB_RETURN_FAIL if there is a mistake in parametres and HYDROLIB_RETURN_OK else
 */
hydrolib_ReturnCode hydrolib_SerialProtocol_TransmitWrite(hydrolib_SerialProtocolHandler *self,
                                                          uint8_t device_address,
                                                          uint8_t memory_address, uint8_t length,
                                                          uint8_t *data);

hydrolib_ReturnCode hydrolib_SerialProtocol_TransmitRead(hydrolib_SerialProtocolHandler *self,
                                                         uint8_t device_address,
                                                         uint8_t memory_address, uint8_t length,
                                                         uint8_t *buffer);

/** @} */

#ifdef __cplusplus
}
#endif

#endif
