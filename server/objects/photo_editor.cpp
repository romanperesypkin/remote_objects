#include "object.hpp"
#include "message.hpp"
#include "photo_editor.hpp"
#include "util.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <sstream>


namespace server {

void PhotoEditor::on_msg_received(std::shared_ptr<Transport> trasnport, const Message& msg) {
    boost::property_tree::ptree msg_tree;
    std::istringstream is (std::string(msg.body(), msg.body_length()));
    boost::property_tree::read_json (is, msg_tree);

    if (msg_tree.get<std::string>("methode") == "upload")
        on_upload(trasnport, msg_tree.get<std::string>("arguments.image"));
    else if (msg_tree.get<std::string>("methode") == "rotate")
        on_rotate(trasnport, msg_tree.get<int>("arguments.degree"));
    else if (msg_tree.get<std::string>("methode") == "info")
        on_info(trasnport);
}

void PhotoEditor::on_upload(std::shared_ptr<Transport> transport, const std::string& _image) {
    image_mutex.lock();
    image = _image;
    image_mutex.unlock();

    printf("SERVER: PhotoEditor(%d) upload \"%s\"\n", id(), image.c_str());

    boost::property_tree::ptree reply_object;
    boost::property_tree::ptree reply_arguments;

    reply_object.put("object", "PhotoEditor");
    reply_object.put("methode", "upload");
    reply_arguments.put("uuid", id());
    reply_object.add_child("arguments", reply_arguments);

    transport->send(to_msg(to_string(reply_object)));
}

void PhotoEditor::on_rotate(std::shared_ptr<Transport> transport, int _degree) {
    degree = _degree;
    printf("SERVER: PhotoEditor(%d) rotate \"%d\"\n", id(), degree.load());

    boost::property_tree::ptree reply_object;
    boost::property_tree::ptree reply_arguments;

    reply_object.put("object", "PhotoEditor");
    reply_object.put("methode", "rotated");
    reply_arguments.put("uuid", id());
    reply_object.add_child("arguments", reply_arguments);

    transport->send(to_msg(to_string(reply_object)));    
}

void PhotoEditor::on_info(std::shared_ptr<Transport> transport) {
    boost::property_tree::ptree reply_object;
    boost::property_tree::ptree reply_arguments;

    image_mutex.lock();
    std::string tmp_image = image;
    image_mutex.unlock();

    printf("SERVER: PhotoEditor(%d) info \"%s %d\"\n", id(), tmp_image.c_str(), degree.load());

    reply_object.put("object", "PhotoEditor");
    reply_object.put("methode", "info");
    reply_arguments.put("uuid", id());
    reply_arguments.put("image", tmp_image);
    reply_arguments.put("degree", degree.load());
    reply_object.add_child("arguments", reply_arguments);

    transport->send(to_msg(to_string(reply_object)));    
}

} // namespace server
