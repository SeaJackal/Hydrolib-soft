# Модуль кольцевой очереди

Модуль на **языке C**, реализующий кольцевой буфер с порядком доступа *LIFO* (очередь).

## Подключение

Для подключения модуля к своему CMake проекту подключите директорию модуля как *subdirectory*.
Интерфейсный заголовочный файл *hydrolib_ring_queue.h* находится в папке include.

## Сборка

В директории находится файл CMakeLists.txt для сборки модуля
и прилагающихся unit тестов.

Внимательно следите, что подключая модуль вы подключаете все его зависимости.
**Сборка отдельного модуля не подразумевает сборки модулей, от которых он зависит!**

## Зависимости

Модуль не зависит от других модулей библиотеки.
