#ifndef HYDROLIB_VECTORNAV_H_
#define HYDROLIB_VECTORNAV_H_

#include <concepts>
#include <cstdint>

#include "hydrolib_common.h"
#include "hydrolib_logger.hpp"
#include "hydrolib_queue_concepts.hpp"

namespace hydrolib
{
    template <concepts::queue::ReadableByteQueue InputQueue, typename Logger>
    class VectorNAVParser
    {
    private:
#pragma pack(push, 1)
        struct Message_
        {
            float yaw_;
            float pitch_;
            float roll_;

            float x_rate_;
            float y_rate_;
            float z_rate_;

            uint16_t crc;
        };
#pragma pack(pop)

    private:
        constexpr static uint32_t HEADER = (0xFA << 0) +
                                           (0x01 << 8) +
                                           (0x28 << (8 * 2));
        constexpr static unsigned CRC_LENGTH = 2;

    public:
        constexpr VectorNAVParser(InputQueue &queue, Logger &logger);

    public:
        hydrolib_ReturnCode Process();

        float GetYaw() const;
        float GetPitch() const;
        float GetRoll() const;

        float GetXRate() const;
        float GetYRate() const;
        float GetZRate() const;

    private:
        InputQueue &queue_;

        Message_ current_data_;

        Logger &logger_;
    };

    template <concepts::queue::ReadableByteQueue InputQueue, typename Logger>
    constexpr VectorNAVParser<InputQueue, Logger>::VectorNAVParser(InputQueue &queue, Logger &logger)
        : queue_(queue),
          logger_(logger)
    {
    }

    template <concepts::queue::ReadableByteQueue InputQueue, typename Logger>
    hydrolib_ReturnCode VectorNAVParser<InputQueue, Logger>::Process()
    {
        uint32_t header_buffer;
        unsigned shift = 0;
        hydrolib_ReturnCode read_res = queue_.Read(&header_buffer, sizeof(uint32_t), shift);
        while (read_res == HYDROLIB_RETURN_OK)
        {
            if (header_buffer == HEADER)
            {
                hydrolib_ReturnCode data_read_res =
                    queue_.Read(&current_data_, sizeof(Message_), sizeof(uint32_t) + shift);
                if (data_read_res != HYDROLIB_RETURN_OK)
                {
                    return HYDROLIB_RETURN_NO_DATA;
                }
                logger_.WriteLog(hydrolib::logger::LogLevel::DEBUG, "Received message");
                logger_.WriteLog(hydrolib::logger::LogLevel::DEBUG, "yaw: {}", static_cast<int>(current_data_.yaw_ * 100));
                logger_.WriteLog(hydrolib::logger::LogLevel::DEBUG, "pitch: {}", static_cast<int>(current_data_.pitch_ * 100));
                logger_.WriteLog(hydrolib::logger::LogLevel::DEBUG, "roll: {}", static_cast<int>(current_data_.roll_ * 100));
                logger_.WriteLog(hydrolib::logger::LogLevel::DEBUG, "x rate: {}", static_cast<int>(current_data_.x_rate_ * 100));
                logger_.WriteLog(hydrolib::logger::LogLevel::DEBUG, "y rate: {}", static_cast<int>(current_data_.y_rate_ * 100));
                logger_.WriteLog(hydrolib::logger::LogLevel::DEBUG, "z rate: {}", static_cast<int>(current_data_.z_rate_ * 100));
                if (shift != 0)
                {
                    logger_.WriteLog(hydrolib::logger::LogLevel::WARNING, "Rubbish bytes: {}", shift);
                }
                queue_.Drop(sizeof(Message_) + sizeof(uint32_t) + shift);
                return HYDROLIB_RETURN_OK;
            }
            shift++;
            read_res = queue_.Read(&header_buffer, sizeof(uint32_t), shift);
        }
        if (shift != 0)
        {
            logger_.WriteLog(hydrolib::logger::LogLevel::WARNING, "Rubbish bytes: {}", shift);
            queue_.Drop(shift);
        }
        return HYDROLIB_RETURN_NO_DATA;
    }

    template <concepts::queue::ReadableByteQueue InputQueue, typename Logger>
    float VectorNAVParser<InputQueue, Logger>::GetYaw() const
    {
        return current_data_.yaw_;
    }

    template <concepts::queue::ReadableByteQueue InputQueue, typename Logger>
    float VectorNAVParser<InputQueue, Logger>::GetPitch() const
    {
        return current_data_.pitch_;
    }

    template <concepts::queue::ReadableByteQueue InputQueue, typename Logger>
    float VectorNAVParser<InputQueue, Logger>::GetRoll() const
    {
        return current_data_.roll_;
    }
}

#endif
