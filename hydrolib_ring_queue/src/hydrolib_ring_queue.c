#include <hydrolib_ring_queue.h>

#include <string.h>

void hydrolib_RingQueue_Init(hydrolib_RingQueue *self, void *buffer, uint16_t byte_capacity)
{
    self->buffer = buffer;
    self->capacity = byte_capacity;
    self->head = 0;
    self->tail = 0;
    self->length = 0;
}

hydrolib_ReturnCode hydrolib_RingQueue_PushByte(hydrolib_RingQueue *self, uint8_t byte)
{
    if (self->length + 1 > self->capacity)
    {
        return HYDROLIB_RETURN_FAIL;
    }

    self->buffer[self->tail] = byte;
    self->tail = (self->tail + 1) % self->capacity;
    self->length++;

    return HYDROLIB_RETURN_OK;
}

hydrolib_ReturnCode hydrolib_RingQueue_Push(hydrolib_RingQueue *self, const void *data, uint16_t data_length)
{
    if (self->length + data_length > self->capacity)
    {
        return HYDROLIB_RETURN_FAIL;
    }

    uint16_t forward_length = self->capacity - self->tail;
    if (forward_length >= data_length)
    {
        memcpy(self->buffer + self->tail, data, data_length);
    }
    else
    {
        memcpy(self->buffer + self->tail, data, forward_length);
        memcpy(self->buffer, data + forward_length, data_length - forward_length);
    }
    self->tail = (self->tail + data_length) % self->capacity;
    self->length += data_length;

    return HYDROLIB_RETURN_OK;
}

hydrolib_ReturnCode hydrolib_RingQueue_PullByte(hydrolib_RingQueue *self, uint8_t *byte)
{
    if (self->length == 0)
    {
        return HYDROLIB_RETURN_FAIL;
    }

    *byte = self->buffer[self->head];
    self->head = (self->head + 1) % self->capacity;
    self->length--;

    return HYDROLIB_RETURN_OK;
}

hydrolib_ReturnCode hydrolib_RingQueue_Pull(hydrolib_RingQueue *self, void *data, uint16_t data_length)
{
    if (self->length < data_length)
    {
        return HYDROLIB_RETURN_FAIL;
    }

    uint16_t forward_length = self->capacity - self->head;
    if (forward_length >= data_length)
    {
        memcpy(data, self->buffer + self->head, data_length);
    }
    else
    {
        memcpy(data, self->buffer + self->head, forward_length);
        memcpy(data + forward_length, self->buffer, data_length - forward_length);
    }
    self->head = (self->head + data_length) % self->capacity;
    self->length -= data_length;

    return HYDROLIB_RETURN_OK;
}

hydrolib_ReturnCode hydrolib_RingQueue_ReadByte(hydrolib_RingQueue *self, uint8_t *data, uint16_t shift)
{
    if (shift >= self->length)
    {
        return HYDROLIB_RETURN_FAIL;
    }
    uint16_t forward_length = self->capacity - self->head;
    if (forward_length > shift)
    {
        *data = self->buffer[self->head + shift];
    }
    else
    {
        *data = self->buffer[shift - forward_length];
    }
    return HYDROLIB_RETURN_OK;
}

hydrolib_ReturnCode hydrolib_RingQueue_Read(hydrolib_RingQueue *self,
                                            void *data, uint16_t data_length, uint16_t shift)
{
    if (shift + data_length > self->length)
    {
        return HYDROLIB_RETURN_FAIL;
    }
    uint16_t forward_length = self->capacity - self->head;
    if (forward_length > shift)
    {
        if (shift + data_length > forward_length)
        {
            memcpy(data, self->buffer + self->head + shift, forward_length - shift);
            memcpy(data, self->buffer, data_length - (forward_length - shift));
        }
        else
        {
            memcpy(data, self->buffer + self->head + shift, data_length);
        }
    }
    else
    {
        memcpy(data, self->buffer + shift - forward_length, data_length);
    }
    return HYDROLIB_RETURN_OK;
}

uint16_t hydrolib_RingQueue_GetLength(hydrolib_RingQueue *self)
{
    return self->length;
}

bool hydrolib_RingQueue_IsEmpty(hydrolib_RingQueue *self)
{
    return self->length == 0;
}

bool hydrolib_RingQueue_IsFull(hydrolib_RingQueue *self)
{
    return self->length == self->capacity;
}
