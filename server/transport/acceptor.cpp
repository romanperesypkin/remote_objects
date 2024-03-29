#include "acceptor.hpp"
#include "server.hpp"
#include "client.hpp"

#include <memory>
#include <string>

namespace server
{

Acceptor::Acceptor(std::shared_ptr<Server> _server,
    boost::asio::io_service& _io_service,
    const std::string _port)
: server{_server}
, io_service{_io_service}
, acceptor{_io_service,
    boost::asio::ip::tcp::endpoint{boost::asio::ip::tcp::v4(), std::stoi(_port)}}
, socket{_io_service}
{}

bool Acceptor::start() {
    do_accept();
    return true;
}

void Acceptor::register_transport(std::shared_ptr<Transport> transport) {
    transports.push_back(transport);
}

void Acceptor::unregister_transport(std::shared_ptr<Transport> transport) {
    /*
    transports.erase(std::remove(transports.begin(),
            transports.end(),
            [transport](auto val) { return val == transport; }),
        transports.end());
    */
}

void Acceptor::do_accept() {
    auto transport = std::make_shared<Client>(shared_from_this(), server, io_service);
    acceptor.async_accept(transport->socket_get(),
        [this, transport](boost::system::error_code ec) {
            if (!ec) {
                transport->start();
                register_transport(transport);            
            } else {
                printf("ERROR: do_accept(%s)\n", ec.message().c_str());
            }
            do_accept();
        });
}

}//namespace server