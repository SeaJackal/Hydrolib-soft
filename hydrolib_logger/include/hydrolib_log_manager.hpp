#ifndef HYDROS_LOGGER_H_
#define HYDROS_LOGGER_H_

#include <cstdint>

#include "hydrolib_log_observer.hpp"

namespace hydrolib::Logger
{

    template <SynchronousByteStreamConcept... Streams>
    class LogManager
    {
    private:
        template <typename Observer,
                  SynchronousByteStreamConcept Stream,
                  SynchronousByteStreamConcept... Streams_>
        class LogManager_;

        template <typename Observer,
                  SynchronousByteStreamConcept Stream1,
                  SynchronousByteStreamConcept Stream2,
                  SynchronousByteStreamConcept... Streams_>
        class LogManager_<Observer, Stream1, Stream2, Streams_...>
            : public LogManager_<LogObserver<Stream1, Observer>, Stream2, Streams_...>
        {
        public:
            constexpr LogManager_(char *format_string,
                                  Observer *next_observer,
                                  Stream1 &stream1, Stream1 &stream2,
                                  Streams_ &...streams)
                : LogManager_<LogObserver<Stream1, Observer>, Stream2, Streams_...>(
                      format_string, &observer_, stream2, streams...),
                  observer_(stream1, format_string, next_observer)

            {
            }

        private:
            LogObserver<Stream1, Observer> observer_;
        };

        template <typename Observer, SynchronousByteStreamConcept Stream>
        class LogManager_<Observer, Stream>
        {
        public:
            constexpr LogManager_(char *format_string,
                                  Observer *next_observer,
                                  Stream &stream)
                : head_observer(stream, format_string, next_observer)
            {
            }

        public:
            LogObserver<Stream, Observer> head_observer;
        };

    public:
        constexpr LogManager(Streams &...streams)
            : observers_("%s", nullptr, streams...)
        {
        }

    public:
        auto &GetHeadObserver()
        {
            return observers_.head_observer;
        }

    private:
        LogManager_<NoObserver, Streams...> observers_;
    };

    // class LoggerModule : public LogDistributor
    // {
    // public:
    //     class UARTloggerStream : public LogDistributor::SubscriberQueueInterface
    //     {
    //     public:
    //         UARTloggerStream(USART_TypeDef *USARTx, osPriority_t thread_priority);

    //     public:
    //         hydrolib_ReturnCode Push(Logger::Log &log) override;
    //         void ThreadHandler();
    //         hydrolib_ReturnCode TransmitByte();

    //     private:
    //         USART_TypeDef *USARTx_;
    //         LogTranslator translator_;

    //         osMessageQueueId_t queue_;
    //         osSemaphoreId_t tx_completed_;
    //         osThreadId_t thread_handler_;
    //     };

    // public:
    //     LoggerModule();
    // };
}

#endif
