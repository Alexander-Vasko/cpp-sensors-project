#pragma once
#include <utility>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>
#include "../queue/thread_safe_queue.h"
#include "../models/sensor_data.h"

namespace beast = boost::beast;         
namespace http = beast::http;           
namespace net = boost::asio;            
using tcp = net::ip::tcp;               

class Connection : public std::enable_shared_from_this<Connection> {
public:
    Connection(tcp::socket socket, ThreadSafeQueue<SensorData>& queue);

    void start();

private:
    void do_read();
    void on_read(beast::error_code ec, std::size_t bytes_transferred);
    void handle_request(http::request<http::string_body>&& req);
    void send_response(http::response<http::string_body>&& res);

    tcp::socket socket_;
    beast::flat_buffer buffer_;
    ThreadSafeQueue<SensorData>& queue_;
    http::request<http::string_body> req_;
};
