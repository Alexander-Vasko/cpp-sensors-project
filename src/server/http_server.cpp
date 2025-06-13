#include "http_server.h"

HttpServer::HttpServer(boost::asio::io_context& ioc, unsigned short port,
                       ThreadSafeQueue<SensorData>& queue)
    : ioc_(ioc),
      acceptor_(ioc, {boost::asio::ip::tcp::v4(), port}),
      queue_(queue) {}

void HttpServer::run() {
    do_accept();
}

void HttpServer::do_accept() {
    acceptor_.async_accept(
        [self = shared_from_this()](boost::system::error_code ec,
                                   boost::asio::ip::tcp::socket socket) {
            if (!ec) {
                std::make_shared<Connection>(std::move(socket), self->queue_)->start();
            }
            self->do_accept();
        });
}
