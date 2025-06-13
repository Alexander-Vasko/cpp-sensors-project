#pragma once
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include "../models/sensor_data.h"
#include "clickhouse_client.h"

class ConnectionPool {
public:
    ConnectionPool(const std::string& host, int port, const std::string& database, size_t pool_size = 4);
    ~ConnectionPool();

    void start();
    void stop();

    // Добавить данные для записи
    void enqueueBatch(const std::vector<SensorData>& batch);

private:
    void workerThread();

    std::vector<std::unique_ptr<ClickHouseClient>> clients_;
    std::vector<std::thread> workers_;

    std::queue<std::vector<SensorData>> tasks_;
    std::mutex mutex_;
    std::condition_variable cond_var_;
    bool stopping_ = false;
};
