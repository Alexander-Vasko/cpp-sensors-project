#include "connection.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

Connection::Connection(tcp::socket socket, ThreadSafeQueue<SensorData>& queue)
    : socket_(std::move(socket)), queue_(queue) {}

void Connection::start() {
    do_read();
}

void Connection::do_read() {
    auto self = shared_from_this();
    http::async_read(socket_, buffer_, req_,
        [self](beast::error_code ec, std::size_t bytes_transferred) {
            self->on_read(ec, bytes_transferred);
        });
}

void Connection::on_read(beast::error_code ec, std::size_t) {
    if (ec == http::error::end_of_stream)
        return socket_.shutdown(tcp::socket::shutdown_send, ec);

    if (ec) return;

    handle_request(std::move(req_));
}

void Connection::handle_request(http::request<http::string_body>&& req) {
    http::response<http::string_body> res{http::status::ok, req.version()};
    res.set(http::field::server, "cpp-sensors-project");
    res.set(http::field::content_type, "application/json");
    res.keep_alive(req.keep_alive());

    try {
        auto j = json::parse(req.body());

        // Пример: ожидаем JSON вида { "sensor_id": "id1", "value": 12.34, "timestamp": 1234567890 }
        SensorData data;
        data.sensor_id = j.at("sensor_id").get<std::string>();
        data.value = j.at("value").get<double>();

        // timestamp в секундах с эпохи
        auto ts = j.at("timestamp").get<int64_t>();
        data.timestamp = std::chrono::system_clock::time_point{std::chrono::seconds{ts}};

        queue_.push(std::move(data));

        res.body() = R"({"status":"ok"})";
        res.prepare_payload();
    }
    catch (const std::exception& e) {
        res.result(http::status::bad_request);
        res.body() = R"({"status":"error","message":")" + std::string(e.what()) + "\"}";
        res.prepare_payload();
    }

    send_response(std::move(res));
}

void Connection::send_response(http::response<http::string_body>&& res) {
    auto self = shared_from_this();
    http::async_write(socket_, res,
        [self](beast::error_code ec, std::size_t) {
            self->socket_.shutdown(tcp::socket::shutdown_send, ec);
        });
}
