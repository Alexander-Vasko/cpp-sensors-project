#pragma once
#include <boost/asio.hpp>
#include <memory>
#include "connection.h"
#include "../queue/thread_safe_queue.h"
#include "../models/sensor_data.h"

class HttpServer : public std::enable_shared_from_this<HttpServer> {
public:
    HttpServer(boost::asio::io_context& ioc, unsigned short port,
               ThreadSafeQueue<SensorData>& queue);

    void run();

private:
    void do_accept();

    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::io_context& ioc_;
    ThreadSafeQueue<SensorData>& queue_;
};
