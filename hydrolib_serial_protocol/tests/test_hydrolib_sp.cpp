#include "hydrolib_serial_protocol.h"

#include <string.h>

#include <gtest/gtest.h>

#define TEST_BUFFER_CAPACITY 20

uint8_t test_buffer[TEST_BUFFER_CAPACITY];
const char test_data[] = "Hello";
uint16_t buffer_pos = 0;

hydrolib_SerialProtocolHandler handler;

bool ReceiveFunc(uint8_t *byte);
bool TransmitFunc(uint8_t *byte);
uint8_t CRCfunc(const uint8_t *buffer, uint16_t length);

TEST(TestHydrolibSerialProtocol, ReceivingTest)
{
    hydrolib_SerialProtocol_Init(&handler, 1, ReceiveFunc, TransmitFunc, CRCfunc);
    uint8_t message_length = strlen(test_data) + 4;
    test_buffer[0] = 1;
    test_buffer[1] = message_length;
    strcpy((char *)(test_buffer + 2), test_data);
    test_buffer[message_length - 1] = CRCfunc((const uint8_t *)test_data, strlen(test_data));
    for (uint8_t i = 0; i < TEST_BUFFER_CAPACITY; i++)
    {
        hydrolib_SerialProtocol_DoWork(&handler);
    }
    uint8_t read_buffer[TEST_BUFFER_CAPACITY];
    uint8_t data_length;
    bool result = hydrolib_SerialProtocol_ReadMessage(&handler, read_buffer, &data_length);
    int cmp_result = strcmp(test_data, (char *)read_buffer);
    EXPECT_TRUE(result);
    EXPECT_EQ(strlen(test_data) + 1, data_length);
    EXPECT_EQ(0, cmp_result);
}

bool ReceiveFunc(uint8_t *byte)
{
    if (buffer_pos >= TEST_BUFFER_CAPACITY)
    {
        return false;
    }
    *byte = test_buffer[buffer_pos];
    buffer_pos++;
    return true;
}

bool TransmitFunc(uint8_t *byte)
{
    (void)byte;
    return true;
}

uint8_t CRCfunc(const uint8_t *buffer, uint16_t length)
{
    (void)buffer;
    (void)length;
    return 0;
}
