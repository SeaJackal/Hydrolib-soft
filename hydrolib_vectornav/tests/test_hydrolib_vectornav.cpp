#include "hydrolib_vectronav.hpp"

#include <gtest/gtest.h>
#include <iostream>
#include <cstdint>
#include <string.h>

using namespace std;

class ByteQueue
{
public:
    ByteQueue() : head_(0),
                  tail_(0),
                  length_(0)
    {
    }

public:
    hydrolib_ReturnCode Read(void *data, unsigned length, unsigned shift) const
    {
        if (length + shift > length_)
        {
            return HYDROLIB_RETURN_FAIL;
        }
        memcpy(data, buffer_ + head_ + shift, length);
        return HYDROLIB_RETURN_OK;
    }

    void Drop(unsigned number)
    {
        head_ += number;
    }

    void Clear()
    {
        head_ = tail_;
    }

    void Push(void *data, unsigned length)
    {
        memcpy(buffer_ + tail_, data, length);
        tail_ += length;
    }

private:
    uint8_t buffer_[100];
    unsigned head_;
    unsigned tail_;
    unsigned length_;
};

TEST(TestHydrolibStrings, TestFormatableString)
{
    ByteQueue queue;
    // hydrolib::VectorNAVParser vector_nav(queue);
}
