#include "connection_pool.h"
#include <iostream>


ConnectionPool::ConnectionPool(const std::string& host, int port, const std::string& database, size_t pool_size) {
    for (size_t i = 0; i < pool_size; ++i) {
        clients_.emplace_back(std::make_unique<ClickHouseClient>(host, port, database));
    }
}

ConnectionPool::~ConnectionPool() {
    stop();
}

void ConnectionPool::start() {
    stopping_ = false;
    for (size_t i = 0; i < clients_.size(); ++i) {
        workers_.emplace_back(&ConnectionPool::workerThread, this);
    }
}

void ConnectionPool::stop() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stopping_ = true;
    }
    cond_var_.notify_all();

    for (auto& thread : workers_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    workers_.clear();
}

void ConnectionPool::enqueueBatch(const std::vector<SensorData>& batch) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        tasks_.push(batch);
    }
    cond_var_.notify_one();
}

void ConnectionPool::workerThread() {
    static thread_local size_t index = 0;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        index = workers_.size() % clients_.size();
    }
    auto& client = clients_[index];

    while (true) {
        std::vector<SensorData> batch;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cond_var_.wait(lock, [this] { return stopping_ || !tasks_.empty(); });

            if (stopping_ && tasks_.empty()) {
                break;
            }

            batch = std::move(tasks_.front());
            tasks_.pop();
        }

        bool ok = client->insertBatch(batch);
        if (!ok) {
            std::cerr << "Failed to insert batch" << std::endl;
        }
    }
}

