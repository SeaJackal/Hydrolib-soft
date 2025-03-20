#include "hydrolib_logger.hpp"

#include <gtest/gtest.h>
#include <iostream>
#include <cstring>

using namespace hydrolib::Logger;
using namespace std;

class LogQueue : public LogDistributor::SubscriberQueueInterface
{
public:
    Logger::Log current_log;

public:
    hydrolib_ReturnCode Push(Logger::Log &log) override
    {
        current_log = log;
        return HYDROLIB_RETURN_OK;
    }
};

TEST(TestHydrolibLogger, TranslatorTest)
{
    Logger logger("Logger");
    LogTranslator translator;
    char buffer[100];

    translator.SetFormatString("[%s] [%l] %m\n");

    logger.WriteLog(LogLevel::INFO, "Message");
    Logger::Log log = logger.GetLastLog();
    translator.StartTranslatingToBytes(log);
    int length = translator.DoTranslation(buffer, 100);
    buffer[length] = '\0';
    std::cout << buffer;
    EXPECT_EQ(0, strcmp(buffer, "[Logger] [INFO] Message\n"));
}

TEST(TestHydrolibLogger, DistributorTest)
{
    LogTranslator translator;
    char buffer[100];
    LogDistributor distributor;
    LogQueue queue;
    Logger logger("Logger", &distributor);

    distributor.AddSubscriber(queue, LogLevel::INFO, &logger);

    translator.SetFormatString("[%s] [%l] %m\n");

    logger.WriteLog(LogLevel::INFO, "Message");
    translator.StartTranslatingToBytes(queue.current_log);
    int length = translator.DoTranslation(buffer, 100);
    buffer[length] = '\0';
    std::cout << buffer;
    EXPECT_EQ(0, strcmp(buffer, "[Logger] [INFO] Message\n"));
}
