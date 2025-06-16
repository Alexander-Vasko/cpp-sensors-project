#include "server/http_server.h"
#include "database/connection_pool.h"
#include "queue/thread_safe_queue.h"
#include "models/sensor_data.h"

#include <utility>
#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include <csignal>
#include <atomic>
#include <chrono>

using boost::asio::ip::tcp;

std::atomic<bool> running = true;

void signal_handler(int signal) {
    std::cout << "Received signal: " << signal << ", stopping..." << std::endl;
    running = false;
}

int main() {
    try {
        // Параметры подключения к ClickHouse
        const std::string host = "127.0.0.1";
        const int port = 9000;
        const std::string database = "default";
        const size_t pool_size = 4;

        ThreadSafeQueue<SensorData> data_queue;

        // Инициализация пула соединений
        ConnectionPool db_pool(host, port, database, pool_size);
        db_pool.start();  // запускаем рабочие потоки пула

        // Рабочий поток, который собирает данные из очереди в батчи и отправляет в ClickHouse
        std::thread worker([&]() {
            const size_t batch_size = 50;
            std::vector<SensorData> batch;
            batch.reserve(batch_size);

            while (running) {
                SensorData data;
                if (data_queue.try_pop(data)) {
                    batch.push_back(std::move(data));
                    if (batch.size() >= batch_size) {
                        db_pool.enqueueBatch(batch);
                        batch.clear();
                    }
                } else {
                    // Если данных нет, но в батче что-то есть — отправим
                    if (!batch.empty()) {
                        db_pool.enqueueBatch(batch);
                        batch.clear();
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            }

            // При завершении отправляем остаток данных
            if (!batch.empty()) {
                db_pool.enqueueBatch(batch);
            }
        });

        // Запуск HTTP сервера
        boost::asio::io_context ioc(1);
        HttpServer server(ioc, 8080, data_queue);
        std::thread server_thread([&ioc]() { ioc.run(); });

        // Обработка сигналов SIGINT и SIGTERM для корректного завершения
        std::signal(SIGINT, signal_handler);
        std::signal(SIGTERM, signal_handler);

        std::cout << "Server running on port 8080..." << std::endl;

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        ioc.stop();
        server_thread.join();

        worker.join();

        db_pool.stop();  // корректно остановить пул

        std::cout << "Shutdown complete." << std::endl;

    } catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
