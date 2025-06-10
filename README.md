# cpp-sencors-project

Сервер, принимающий измерения от десятка тысяч датчиков одновременно и отправляющий их в аналитическую СУБД. 

## Зависимости

Для эффективной работы с большим количеством одновременных сетевых соединений используются механизмы мультиплексированного
ввода/вывода ОС. Они позволяют обрабатывать в одном потоке события от множества сетевых соединений. [boost::asio](https://www.boost.org/doc/libs/latest/doc/html/boost_asio.html)
предоставляет удобный интерфейс для работы с этими механизмами, реализуя паттерн [proactor](https://www.boost.org/doc/libs/latest/doc/html/boost_asio/overview/core/async.html).
Операции ввода/вывода при таком подходе не блокируются, вместо этого регистрируется колбэк, который будет вызван после
окончания операции ввода/вывода.

Для работы с HTTP протоколом используется [boost::beast](https://www.boost.org/doc/libs/develop/libs/beast/doc/html/index.html).
Он, как правило, входит в состав пакета `boost`.

Парсинг и генерация JSON осуществляется с помощью библиотеки [nlohman/json](https://github.com/nlohmann/json). 

Клиент ClickHouse: [clickhouse-cpp](https://github.com/ClickHouse/clickhouse-cpp).
