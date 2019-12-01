#include "transport.hpp"

#include <iostream>
#include <string>


namespace client
{

Transport::Transport(boost::asio::io_service& _io_service
, const std::string& _ip
, const std::string& _port)
: io_service(_io_service)
, socket(_io_service)
, ip(_ip)
, port(_port)
{}

void Transport::send(const Message& _data) {
    boost::asio::async_write(socket,
        boost::asio::buffer(_data.data(), _data.length()),
        [this](boost::system::error_code ec, std::size_t) {
            if (ec) {
                printf("ERROR: async_write (%s)\n", ec.message().c_str());
                socket.close();
                exit(1);
            }
        });
}

bool Transport::start() {
    boost::system::error_code ec;
    socket.connect(boost::asio::ip::tcp::endpoint{boost::asio::ip::address::from_string(ip),
            std::stoi(port)},
        ec);
    if (ec) {
        printf("ERROR: connect(%s)\n", ec.message().c_str());
        return false;
    }

    do_read_header();
    return true;
}

void Transport::do_read_header() {
    boost::asio::async_read(socket,
        boost::asio::buffer(read_msg.data(), Message::header_length),
        [this](boost::system::error_code ec, std::size_t /*length*/) {
            if (!ec && read_msg.decode_header())
                do_read_body();
            else {
                printf("ERROR: do_read_header(%s)\n", ec.message().c_str());
                socket.close();
                exit(1);
            }
        });
}

void Transport::do_read_body() {
    boost::asio::async_read(socket,
        boost::asio::buffer(read_msg.body(), read_msg.body_length()),
        [this](boost::system::error_code ec, std::size_t /*length*/) {
            if (!ec) {
                std::cout.write(read_msg.body(), read_msg.body_length());
                std::cout << "\n";
                do_read_header();
            } else {
                printf("ERROR: do_read_body(%s)\n", ec.message().c_str());
                socket.close();
                exit(1);
            }
        });
}

void Transport::stop() {
    socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
    socket.close();
}

}//namespace client