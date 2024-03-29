#include "photo_editor.hpp"
#include "transport_sync.hpp"
#include "util.hpp"

#include <thread>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <sstream>

namespace client {

void PhotoEditor::create(int _uuid) {
    boost::property_tree::ptree object;
    boost::property_tree::ptree arguments;

    object.put("object", "PhotoEditor");
    object.put("methode", "create");
    arguments.put("uuid", _uuid);
    object.add_child("arguments", arguments);

    Message reply = send(to_msg(to_string(object)));

    boost::property_tree::ptree reply_tree;
    std::istringstream is (std::string(reply.body(), reply.body_length()));
    boost::property_tree::read_json (is, reply_tree);

    uuid = reply_tree.get<int>("arguments.uuid");
}

void PhotoEditor::upload(const std::string& image) {
    boost::property_tree::ptree object;
    boost::property_tree::ptree arguments;

    object.put("object", "PhotoEditor");
    object.put("methode", "upload");
    arguments.put("uuid", uuid);
    arguments.put("image", image);
    object.add_child("arguments", arguments);

    send(to_msg(to_string(object)));
}

void PhotoEditor::rotate(int degree) {
    boost::property_tree::ptree object;
    boost::property_tree::ptree arguments;

    object.put("object", "PhotoEditor");
    object.put("methode", "rotate");
    arguments.put("uuid", uuid);
    arguments.put("degree", degree);
    object.add_child("arguments", arguments);

    send(to_msg(to_string(object)));
}

std::shared_ptr<PhotoEditorInfo> PhotoEditor::info() {
    boost::property_tree::ptree object;
    boost::property_tree::ptree arguments;

    object.put("object", "PhotoEditor");
    object.put("methode", "info");
    arguments.put("uuid", uuid);
    object.add_child("arguments", arguments);

    Message reply = send(to_msg(to_string(object)));

    boost::property_tree::ptree reply_tree;
    std::istringstream is (std::string(reply.body(), reply.body_length()));
    boost::property_tree::read_json (is, reply_tree);   

    return std::make_shared<PhotoEditorInfo>(reply_tree.get<std::string>("arguments.image"),
        reply_tree.get<int>("arguments.degree"));
}

Message PhotoEditor::send(const Message& msg) {    
    TransportSync transport{"127.0.0.1", "5001"};
    return transport.send(msg);
}

} // namespace client
