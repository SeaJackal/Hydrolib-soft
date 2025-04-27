#include "hydrolib_logger.hpp"
#include "hydrolib_log_observer.hpp"
#include "hydrolib_log_distributor.hpp"

#include <gtest/gtest.h>
#include <iostream>
#include <cstring>

using namespace hydrolib::Logger;
using namespace std;

class LogStream
{
public:
    LogStream(char *buffer) : buffer_(buffer),
                              length_(0)
    {
    }

    LogStream(LogStream &) = delete;

public:
    hydrolib_ReturnCode Push(const uint8_t *source, size_t length)
    {
        memcpy(buffer_ + length_, source, length);
        length_ += length;
        return HYDROLIB_RETURN_OK;
    }

    hydrolib_ReturnCode Open()
    {
        return HYDROLIB_RETURN_OK;
    }

    hydrolib_ReturnCode Close()
    {
        return HYDROLIB_RETURN_OK;
    }

    int GetLength()
    {
        return length_;
    }

    void Reset()
    {
        length_ = 0;
    }

private:
    char *buffer_;
    int length_;
};

// class LogQueue
// {
// public:
//     LogQueue() : flag(false)
//     {
//     }

// public:
//     hydrolib_ReturnCode Push(const void *log)
//     {
//         memcpy(&log_, log, sizeof(Log));
//         flag = true;
//         return HYDROLIB_RETURN_OK;
//     }

//     hydrolib_ReturnCode Pull(void *log)
//     {
//         if (!flag)
//         {
//             return HYDROLIB_RETURN_NO_DATA;
//         }
//         memcpy(log, &log_, sizeof(Log));
//         flag = false;
//         return HYDROLIB_RETURN_OK;
//     }

// private:
//     Log log_{};
//     bool flag;
// };

char buffer1[100];
char buffer2[100];
LogStream stream1(buffer1);
LogStream stream2(buffer2);

LogDistributor manager("[%s] [%l] %m\n", stream1, stream2);
Logger logger1("Logger1", 0, manager);
Logger logger2("Logger2", 1, manager);

TEST(TestHydrolibLogger, TranslatorTest)
{
    manager.SetAllFilters(0, LogLevel::DEBUG);
    manager.SetAllFilters(1, LogLevel::DEBUG);
    logger1.WriteLog(LogLevel::INFO, "First message: {}", 1);
    int length = stream1.GetLength();
    buffer1[length] = '\0';
    buffer2[length] = '\0';
    std::cout << buffer1;
    EXPECT_EQ(0, strcmp(buffer1, "[Logger1] [INFO] First message: 1\n"));
    EXPECT_EQ(0, strcmp(buffer2, "[Logger1] [INFO] First message: 1\n"));

    stream1.Reset();
    stream2.Reset();
    logger2.WriteLog(LogLevel::DEBUG, "Message two: {}", 2);
    length = stream1.GetLength();
    buffer1[length] = '\0';
    buffer2[length] = '\0';
    std::cout << buffer1;
    EXPECT_EQ(0, strcmp(buffer1, "[Logger2] [DEBUG] Message two: 2\n"));
    EXPECT_EQ(0, strcmp(buffer2, "[Logger2] [DEBUG] Message two: 2\n"));

    stream1.Reset();
    stream2.Reset();
    logger1.WriteLog(LogLevel::CRITICAL, "Third: {}", 3);
    length = stream1.GetLength();
    buffer1[length] = '\0';
    buffer2[length] = '\0';
    std::cout << buffer1;
    EXPECT_EQ(0, strcmp(buffer1, "[Logger1] [CRITICAL] Third: 3\n"));
    EXPECT_EQ(0, strcmp(buffer2, "[Logger1] [CRITICAL] Third: 3\n"));
}

TEST(TestHydrolibLogger, FilterTest)
{
    manager.SetAllFilters(0, LogLevel::DEBUG);
    manager.SetAllFilters(1, LogLevel::DEBUG);
    manager.SetFilter(0, 0, LogLevel::INFO);
    manager.SetFilter(1, 1, LogLevel::INFO);

    stream1.Reset();
    stream2.Reset();
    logger1.WriteLog(LogLevel::INFO, "First message: {}", 1);
    int length1 = stream1.GetLength();
    int length2 = stream2.GetLength();
    EXPECT_EQ(length1, sizeof("[Logger1] [INFO] First message: 1\n") - 1);
    EXPECT_EQ(length2, sizeof("[Logger1] [INFO] First message: 1\n") - 1);

    stream1.Reset();
    stream2.Reset();
    logger1.WriteLog(LogLevel::DEBUG, "Message two: {}", 2);
    length1 = stream1.GetLength();
    length2 = stream2.GetLength();
    EXPECT_EQ(length1, 0);
    EXPECT_EQ(length2, sizeof("[Logger2] [DEBUG] Message two: 2\n") - 1);

    stream1.Reset();
    stream2.Reset();
    logger2.WriteLog(LogLevel::DEBUG, "Message two: {}", 2);
    length1 = stream1.GetLength();
    length2 = stream2.GetLength();
    EXPECT_EQ(length2, 0);
    EXPECT_EQ(length1, sizeof("[Logger2] [DEBUG] Message two: 2\n") - 1);
}

// TEST(TestHydrolibLogger, DistributorTest)
// {
//     LogTranslator translator;
//     char buffer[100];
//     LogDistributor distributor;
//     LogQueue queue;
//     Logger logger("Logger", &distributor);

//     distributor.AddSubscriber(queue, LogLevel::INFO, &logger);

//     translator.SetFormatString("[%s] [%l] %m\n");

//     logger.WriteLog(LogLevel::INFO, "Message");
//     translator.StartTranslatingToBytes(queue.current_log);
//     int length = translator.DoTranslation(buffer, 100);
//     buffer[length] = '\0';
//     std::cout << buffer;
//     EXPECT_EQ(0, strcmp(buffer, "[Logger] [INFO] Message\n"));
// }
