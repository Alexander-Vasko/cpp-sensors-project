# cpp-sencors-project

Сервер, принимающий измерения от десятка тысяч датчиков одновременно и отправляющий их в аналитическую СУБД. 

## Зависимости

Для эффективной работы с большим количеством одновременных сетевых соединений используются механизмы мультиплексированного
ввода/вывода ОС. Они позволяют обрабатывать в одном потоке события от множества сетевых соединений. [boost::asio](https://www.boost.org/doc/libs/latest/doc/html/boost_asio.html)
предоставляет удобный интерфейс для работы с этими механизмами, реализуя паттерн [proactor](https://www.boost.org/doc/libs/latest/doc/html/boost_asio/overview/core/async.html).
Операции ввода/вывода при таком подходе не блокируются, вместо этого регистрируется колбэк, который вызывается после
окончания операции ввода/вывода.

Для работы с HTTP протоколом используется [boost::beast](https://www.boost.org/doc/libs/develop/libs/beast/doc/html/index.html).
Он, как правило, входит в состав пакета `boost`.

Парсинг и генерация JSON осуществляется с помощью библиотеки [nlohman/json](https://github.com/nlohmann/json). 

Клиент ClickHouse: [clickhouse-cpp](https://github.com/ClickHouse/clickhouse-cpp).

Для коммуникации между потоками используются потокобезопасные очереди из [boost::thread](https://www.boost.org/doc/libs/1_83_0/doc/html/thread.html).

# Ubuntu

```bash
apt update && apt install libboost-dev libboost-thread-dev nlohmann-json3-dev
```

## Сборка

```bash
git clone --recurse-submodules https://github.com/Alexander-Vasko/cpp-sencors-project.git
cd cpp-sencors-project
cmake -S . -B build
cmake --build build -j $(nproc)
```

## Архитектура

Rest сервер использует выделенный пул потоков для обработки соединений. В примере описана наиболее похожая на требуемую
реализация http сервера: [advanced_server.cpp](https://www.boost.org/doc/libs/develop/libs/beast/example/advanced/server/advanced_server.cpp).

Для подключения к ClickHouse используется пул соединений, каждое соединение обрабатывается в отдельном потоке.

Поток Rest сервера кладет в потокобезопасную очередь запрос для дальнейшей обработки. При успешном добавлении запроса в очередь
ответ клиенту отправляется в одном из двух случаев (см. `beast::tcp_stream`):

1. Данные успешно записаны в СУБД, и информация об этом успела дойти до потока Rest сервера
2. Произошел таймаут обработки запроса, клиенту отправляется соответствующая ошибка, сетевое соединение закрывается

Каждый поток соединения с СУБД в цикле читает потокобезопасную очередь и синхронно делает вставку в СУБД.

Результат вставки (OK/ERROR) помещается в очередь экзекьютора сетевого соединения (см. `beast::tcp_stream::get_executor()`,
`boost::asio::dispatch(executor, do_write_response)`).
