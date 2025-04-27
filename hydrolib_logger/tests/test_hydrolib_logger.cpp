#include "hydrolib_logger.hpp"
#include "hydrolib_log_observer.hpp"

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

char buffer[100];
LogStream stream(buffer);
LogObserver observer(stream, "[%s] [%l] %m\n");

TEST(TestHydrolibLogger, TranslatorTest)
{
    Logger logger("Logger", 0, observer);
    observer.SetLogFiltration(0, LogLevel::DEBUG);

    logger.WriteLog(LogLevel::INFO, "First message: {}", 1);
    int length = stream.GetLength();
    buffer[length] = '\0';
    std::cout << buffer;
    EXPECT_EQ(0, strcmp(buffer, "[Logger] [INFO] First message: 1\n"));

    stream.Reset();
    logger.WriteLog(LogLevel::DEBUG, "Message two: {}", 2);
    length = stream.GetLength();
    buffer[length] = '\0';
    std::cout << buffer;
    EXPECT_EQ(0, strcmp(buffer, "[Logger] [DEBUG] Message two: 2\n"));

    stream.Reset();
    logger.WriteLog(LogLevel::CRITICAL, "Third: {}", 3);
    length = stream.GetLength();
    buffer[length] = '\0';
    std::cout << buffer;
    EXPECT_EQ(0, strcmp(buffer, "[Logger] [CRITICAL] Third: 3\n"));
}

TEST(TestHydrolibLogger, FilterTest)
{
    Logger logger("Logger", 0, observer);
    observer.SetLogFiltration(0, LogLevel::INFO);

    stream.Reset();
    logger.WriteLog(LogLevel::INFO, "First message");
    int length = stream.GetLength();
    buffer[length] = '\0';
    std::cout << buffer;
    EXPECT_EQ(length, sizeof("[Logger] [INFO] First message\n") - 1);

    stream.Reset();
    logger.WriteLog(LogLevel::DEBUG, "Message two");
    length = stream.GetLength();
    buffer[length] = '\0';
    std::cout << buffer;
    EXPECT_EQ(length, 0);
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
