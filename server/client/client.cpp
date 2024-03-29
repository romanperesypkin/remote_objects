#include "client.hpp"
#include "photo_editor.hpp"
#include "stream.hpp"
#include "server.hpp"
#include "util.hpp"

#include <boost/property_tree/json_parser.hpp>
#include <sstream>
#include <string>

namespace server {

Client::Client(std::shared_ptr<Acceptor> _acceptor,
    std::shared_ptr<Server> _server,
    boost::asio::io_service& _io_service)
: Transport(_acceptor, _io_service)
, server{_server}
{}

void Client::on_msg_received(const Message& msg) {
    boost::property_tree::ptree msg_tree;
    std::istringstream is (std::string(msg.body(), msg.body_length()));
    boost::property_tree::read_json (is, msg_tree);

    if (msg_tree.get<std::string>("methode") == "create") {
        create_new_object(msg_tree);
        return;
    }

    if (msg_tree.get<std::string>("object") == "PhotoEditor") {
        auto photo_editor = std::dynamic_pointer_cast<PhotoEditor>(server->objects_repository()->find(msg_tree.get<int>("arguments.uuid")));
        if (photo_editor)
            photo_editor->on_msg_received(shared_from_this(), msg);
    } else if (msg_tree.get<std::string>("object") == "Stream") {
        auto stream = std::dynamic_pointer_cast<Stream>(server->objects_repository()->find(msg_tree.get<int>("arguments.uuid")));
        if (stream)
            stream->on_msg_received(shared_from_this(), msg);
    }
}

void Client::create_new_object(const boost::property_tree::ptree& json_tree) {
    boost::property_tree::ptree reply_object;
    boost::property_tree::ptree reply_arguments;

    reply_object.put("methode", "create");
    int uuid = json_tree.get<int>("arguments.uuid");
    // TODO: fix double create
    if (json_tree.get<std::string>("object") == "PhotoEditor") {
        auto object = server->objects_repository()->find(uuid);
        if (object) {
            reply_arguments.put("uuid", object->id());
            printf("SERVER: PhotoEditor(%d) bind\n", uuid);
        } else {
            auto photo_editor = std::dynamic_pointer_cast<PhotoEditor>(server->objects_repository()->make_photo_editor());
            reply_arguments.put("uuid", photo_editor->id());
        }
        reply_object.put("object", "PhotoEditor");
        
    } else if (json_tree.get<std::string>("object") == "Stream") {
        auto object = server->objects_repository()->find(uuid);
        if (object) {
            reply_arguments.put("uuid", object->id());
            printf("SERVER: Server(%d) bind\n", uuid);
        } else {
            auto stream = std::dynamic_pointer_cast<Stream>(server->objects_repository()->make_stream());
            reply_arguments.put("uuid", stream->id());
        }        
        reply_object.put("object", "Stream");
    }

    reply_object.add_child("arguments", reply_arguments);
    send(to_msg(to_string(reply_object)));
}

} // namespace server

