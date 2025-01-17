#ifndef HYDROLIB_RING_BUFFER_H_
#define HYDROLIB_RING_BUFFER_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    HYDROLIB_RETURN_OK = 0,
    HYDROLIB_RETURN_FAIL
} hydrolib_ReturnCode;

typedef struct
{
    uint8_t *buffer;

    uint16_t capacity;
    uint16_t head;
    uint16_t tail;

    uint16_t length;
} hydrolib_RingQueue;

void hydrolib_RingQueue_Init(hydrolib_RingQueue *self, void *buffer, uint16_t byte_capacity);

hydrolib_ReturnCode hydrolib_RingQueue_PushByte(hydrolib_RingQueue *self, uint8_t byte);
hydrolib_ReturnCode hydrolib_RingQueue_Push(hydrolib_RingQueue *self, const void *data, uint16_t data_length);
hydrolib_ReturnCode hydrolib_RingQueue_PullByte(hydrolib_RingQueue *self, uint8_t *byte);
hydrolib_ReturnCode hydrolib_RingQueue_Pull(hydrolib_RingQueue *self, void *data, uint16_t data_length);
hydrolib_ReturnCode hydrolib_RingQueue_ReadByte(hydrolib_RingQueue *self, uint8_t *data, uint16_t shift);
hydrolib_ReturnCode hydrolib_RingQueue_Read(hydrolib_RingQueue *self, void *data, uint16_t data_length, uint16_t shift);

uint16_t hydrolib_RingQueue_GetLength(hydrolib_RingQueue *self);
bool hydrolib_RingQueue_IsEmpty(hydrolib_RingQueue *self);
bool hydrolib_RingQueue_IsFull(hydrolib_RingQueue *self);

#ifdef __cplusplus
}
#endif

#endif