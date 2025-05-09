/**
 @defgroup hydrolib_serial_protocol Hydrolib serial protocol
@brief Protocol for communication through duplex serial port

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

#include <cstdint>

#include "hydrolib_common.h"

namespace hydrolib
{
    namespace serialProtocol
    {

#define HYDROLIB_SP_MAX_MESSAGE_LENGTH 255

        /**
         @brief Serial protocol manager handler structure which contains all information
            about session

            */
        class MessageProcessor
        {
        public:
            class RxQueueInterface
            {
            public:
                virtual hydrolib_ReturnCode Read(void *buffer, uint32_t length, uint32_t shift) const = 0;
                virtual void Drop(uint32_t number) = 0;
                virtual void Clear() = 0;
            };

            class TxQueueInterface
            {
            public:
                virtual hydrolib_ReturnCode Push(void *buffer, uint32_t length) = 0;
            };

            class PublicMemoryInterface
            {
            public:
                virtual hydrolib_ReturnCode Read(void *buffer, uint32_t address,
                                                 uint32_t length) = 0;
                virtual hydrolib_ReturnCode Write(const void *buffer, uint32_t address,
                                                  uint32_t length) = 0;
                virtual uint32_t Size() = 0;
            };

        private:
            enum Command_
            {
                HYDROLIB_SP_COMMAND_WRITE = 1,
                HYDROLIB_SP_COMMAND_READ,
                HYDROLIB_SP_COMMAND_RESPOND
            };

            union MessageHeader_
            {
            public:
#pragma pack(push, 1)

                struct MemoryAccess
                {
                    uint8_t device_address;
                    uint8_t self_address;
                    uint8_t memory_address;
                    uint8_t memory_access_length;
                };

                struct Responce
                {
                    uint8_t device_address;
                    uint8_t self_address;
                };

#pragma pack(pop)

            public:
                MemoryAccess memory_access_header;
                Responce responce_header;
            };

        public:
            MessageProcessor(uint8_t address,
                             TxQueueInterface &tx_queue,
                             RxQueueInterface &rx_queue,
                             PublicMemoryInterface &public_memory);

        private:
            uint8_t self_address_;

            TxQueueInterface &tx_queue_;
            RxQueueInterface &rx_queue_;

            uint16_t current_rx_message_length_;
            uint16_t current_rx_processed_length_;
            uint8_t current_rx_message_[HYDROLIB_SP_MAX_MESSAGE_LENGTH];

            uint8_t responding_device_;
            uint8_t *responce_buffer_;
            uint8_t responce_data_length_;

            Command_ current_command_;

            MessageHeader_ *current_header_;

            PublicMemoryInterface &public_memory_;

        public:
            bool ProcessRx();
            hydrolib_ReturnCode TransmitWrite(uint8_t device_address,
                                              uint32_t memory_address, uint32_t length,
                                              uint8_t *data);
            hydrolib_ReturnCode TransmitRead(uint8_t device_address,
                                             uint32_t memory_address, uint32_t length,
                                             uint8_t *buffer);

        private:
            bool MoveToHeader_();
            hydrolib_ReturnCode ParseHeader_();
            void ProcessCommand_();

            hydrolib_ReturnCode ParseMemoryAccessHeader_();
            hydrolib_ReturnCode ParseResponceHeader_();

            static uint8_t CRCfunc_(const uint8_t *buffer, uint16_t length);
        };

        /** @} */
    }
}

#endif