#pragma once

#include <cstdint>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "hydrolib_return_codes.hpp"

namespace hydrolib::ring_queue
{

class RingQueue
{
public:
    constexpr RingQueue(void *buffer, int byte_capacity);

public:

    void Clear();
    ReturnCode Drop(int number);

    ReturnCode PushByte(uint8_t byte);
    ReturnCode Push(const void *data, int data_length);
    ReturnCode PullByte(uint8_t *byte);
    ReturnCode Pull(void *data, int data_length);
    ReturnCode ReadByte(uint8_t *data, uint16_t shift);
    ReturnCode Read(const void *data, int data_length, uint16_t shift);

    ReturnCode Read2BytesLE(int *data, uint16_t shift);
    ReturnCode Read4BytesLE(uint32_t *data, uint16_t shift);
    ReturnCode Read(void *data, int data_length, uint16_t shift);

    uint16_t FindByte(uint8_t target_byte, uint16_t shift);
    uint16_t Find2BytesLE(uint16_t target_bytes, uint16_t shift);
    uint16_t Find4BytesLE(uint32_t target_bytes, uint16_t shift);

    int GetLength();
    int GetCapacity();
    bool IsEmpty();
    bool IsFull();

private:
    uint8_t *buffer_;

    int capacity_;
    int head_;
    int tail_;

    int length_;
};

constexpr RingQueue::RingQueue(void *buffer, int byte_capacity = 0)
    : buffer_(static_cast<uint8_t *>(buffer)),
      capacity_(byte_capacity),
      head_(0),
      tail_(0),
      length_(0)
{
}

inline void RingQueue::Clear()
{
    head_ = 0;
    tail_ = 0;
    length_ = 0;
}

inline ReturnCode RingQueue::Drop(int number)
{
    if (number > length_)
    {
        return ReturnCode::FAIL;
    }
    head_ = (head_ + number) % capacity_;
    length_ -= number;
    return ReturnCode::OK;
}

inline ReturnCode RingQueue::PushByte(uint8_t byte)
{
    if (length_ + 1 > capacity_)
    {
        return ReturnCode::FAIL;
    }

    buffer_[tail_] = byte;
    tail_ = (tail_ + 1) % capacity_;
    length_++;

    return ReturnCode::OK;
}

inline ReturnCode RingQueue::Push(const void *data, int data_length)
{
    if (length_ + data_length > capacity_)
    {
        return ReturnCode::FAIL;
    }

    int forward_length = capacity_ - tail_;
    if (forward_length >= data_length)
    {
        memcpy(buffer_ + tail_, data, data_length);
    }
    else
    {
        memcpy(buffer_ + tail_, data, forward_length);
        memcpy(buffer_, (uint8_t *)data + forward_length,
               data_length - forward_length);
    }
    tail_ = (tail_ + data_length) % capacity_;
    length_ += data_length;

    return ReturnCode::OK;
}

inline ReturnCode RingQueue::PullByte(uint8_t *byte)
{
    if (length_ == 0)
    {
        return ReturnCode::FAIL;
    }

    *byte = buffer_[head_];
    head_ = (head_ + 1) % capacity_;
    length_--;

    return ReturnCode::OK;
}

inline ReturnCode RingQueue::Pull(void *data, int data_length)
{
    if (length_ < data_length)
    {
        return ReturnCode::FAIL;
    }

    int forward_length = capacity_ - head_;
    if (forward_length >= data_length)
    {
        memcpy(data, buffer_ + head_, data_length);
    }
    else
    {
        memcpy(data, buffer_ + head_, forward_length);
        memcpy((uint8_t *)data + forward_length, buffer_,
               data_length - forward_length);
    }
    head_ = (head_ + data_length) % capacity_;
    length_ -= data_length;

    return ReturnCode::OK;
}

inline ReturnCode RingQueue::ReadByte(uint8_t *data, uint16_t shift)
{
    if (shift >= length_)
    {
        return ReturnCode::FAIL;
    }

    int buffer_index = (head_ + shift) % capacity_;
    *data = buffer_[buffer_index];

    return ReturnCode::OK;
}

inline ReturnCode RingQueue::Read2BytesLE(int *data, uint16_t shift)
{
    if (shift >= length_ - 1)
    {
        return ReturnCode::FAIL;
    }
    int buffer_index = (head_ + shift) % capacity_;

    if (buffer_index == capacity_ - 1)
    {
        *data = buffer_[buffer_index] | buffer_[0] << 8;
    }
    else
    {
        *data = *(int *)(buffer_ + buffer_index);
    }

    return ReturnCode::OK;
}

inline ReturnCode RingQueue::Read4BytesLE(uint32_t *data, uint16_t shift)
{
    if (shift >= length_ - 3)
    {
        return ReturnCode::FAIL;
    }
    int buffer_index = (head_ + shift) % capacity_;

    if (buffer_index == capacity_ - 1)
    {
        *data = buffer_[buffer_index] | *((uint32_t *)buffer_) << 8;
    }
    else if (buffer_index == capacity_ - 2)
    {
        *data = *(int *)(buffer_ + buffer_index) | *((uint32_t *)buffer_) << 16;
    }
    else if (buffer_index == capacity_ - 3)
    {
        *data = *(uint32_t *)(buffer_ + buffer_index - 1) >> 8 | buffer_[0]
                                                                     << 24;
    }
    else
    {
        *data = *(uint32_t *)(buffer_ + buffer_index);
    }

    return ReturnCode::OK;
}

inline ReturnCode RingQueue::Read(void *data, int data_length, uint16_t shift)
{
    if (shift + data_length > length_)
    {
        return ReturnCode::FAIL;
    }
    int forward_length = capacity_ - head_;
    if (forward_length > shift)
    {
        if (shift + data_length > forward_length)
        {
            memcpy(data, buffer_ + head_ + shift, forward_length - shift);
            memcpy((uint8_t *)data + forward_length - shift, buffer_,
                   data_length - (forward_length - shift));
        }
        else
        {
            memcpy(data, buffer_ + head_ + shift, data_length);
        }
    }
    else
    {
        memcpy(data, buffer_ + shift - forward_length, data_length);
    }
    return ReturnCode::OK;
}

inline uint16_t RingQueue::FindByte(uint8_t target_byte, uint16_t shift)
{
    for (uint16_t i = shift; i < length_; i++)
    {
        uint16_t current_index = (head_ + i) % capacity_;
        if (buffer_[current_index] == target_byte)
        {
            return i;
        }
    }
    return -1;
}

inline uint16_t RingQueue::Find2BytesLE(uint16_t target_bytes, uint16_t shift)
{
    for (uint16_t i = shift; i < length_; i++)
    {
        uint16_t current_index = (head_ + i) % capacity_;
        uint16_t current_data;
        if (current_index == capacity_ - 1)
        {
            current_data = buffer_[current_index] | buffer_[0] << 8;
        }
        else
        {
            current_data = *(int *)(buffer_ + current_index);
        }
        if (current_data == target_bytes)
        {
            return i;
        }
    }
    return -1;
}

inline uint16_t RingQueue::Find4BytesLE(uint32_t target_bytes, uint16_t shift)
{
    for (uint16_t i = shift; i < length_; i++)
    {
        uint16_t current_index = (head_ + i) % capacity_;
        uint32_t current_data;
        if (current_index == capacity_ - 1)
        {
            current_data = buffer_[current_index] | *((uint32_t *)buffer_) << 8;
        }
        else if (current_index == capacity_ - 2)
        {
            current_data = *(uint16_t *)(buffer_ + current_index) |
                           *((uint32_t *)buffer_) << 16;
        }
        else if (current_index == capacity_ - 3)
        {
            current_data = *(uint32_t *)(buffer_ + current_index - 1) >> 8 |
                           buffer_[0] << 24;
        }
        else
        {
            current_data = *(uint32_t *)(buffer_ + current_index);
        }
        if (current_data == target_bytes)
        {
            return i;
        }
    }
    return -1;
}

inline int RingQueue::GetLength() { return length_; }

inline int RingQueue::GetCapacity() { return capacity_; }

inline bool RingQueue::IsEmpty() { return length_ == 0; }

inline bool RingQueue::IsFull() { return length_ == capacity_; }

} // namespace hydrolib::ring_queue
