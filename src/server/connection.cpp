#include "connection.h"
#include <utility>
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

Connection::Connection(tcp::socket socket, ThreadSafeQueue<SensorData>& queue)
    : socket_(std::move(socket)), queue_(queue) {}

void Connection::start() {
    std::cout << "Connection started" << std::endl;
    do_read();
}

void Connection::do_read() {
    std::cout << "Starting async_read..." << std::endl;
    auto self = shared_from_this();
    http::async_read(socket_, buffer_, req_,
        [self](beast::error_code ec, std::size_t bytes_transferred) {
            std::cout << "async_read callback, ec=" << ec.message() << ", bytes=" << bytes_transferred << std::endl;
            self->on_read(ec, bytes_transferred);
        });
}


void Connection::on_read(beast::error_code ec, std::size_t bytes_transferred) {
    std::cout << "[on_read] ec=" << ec.message() << ", bytes=" << bytes_transferred << std::endl;

    if (ec == http::error::end_of_stream) {
        socket_.shutdown(tcp::socket::shutdown_send, ec);
        return;
    }
    if (ec) {
        std::cout << "[on_read] error: " << ec.message() << std::endl;
        return;
    }
    handle_request(std::move(req_));
}

void Connection::handle_request(http::request<http::string_body>&& req) {
    std::cout << "[handle_request] got request body: " << req.body() << std::endl;

    http::response<http::string_body> res{http::status::ok, req.version()};
    res.set(http::field::server, "cpp-sensors-project");
    res.set(http::field::content_type, "application/json");
    res.keep_alive(req.keep_alive());

    try {
        auto j = json::parse(req.body());

        SensorData data;
        data.sensor_id = j.at("sensor_id").get<std::string>();
        data.value = j.at("value").get<double>();
        auto ts = j.at("timestamp").get<int64_t>();
        data.timestamp = std::chrono::system_clock::time_point{std::chrono::seconds{ts}};

        queue_.push(std::move(data));

        res.body() = R"({"status":"ok"})";
        res.prepare_payload();
    }
    catch (const std::exception& e) {
        std::cout << "[handle_request] json parse error: " << e.what() << std::endl;
        res.result(http::status::bad_request);
        res.body() = R"({"status":"error","message":")" + std::string(e.what()) + "\"}";
        res.prepare_payload();
    }

    send_response(std::move(res));
}

void Connection::send_response(http::response<http::string_body>&& res) {
    std::cout << "[send_response] sending response" << std::endl;
    auto self = shared_from_this();
    http::async_write(socket_, res,
        [self](beast::error_code ec, std::size_t) {
            if (ec) {
                std::cerr << "Write error: " << ec.message() << std::endl;
            } else {
                std::cout << "[send_response] write completed" << std::endl;
            }
        });
}
