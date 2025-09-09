#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <thread>

using boost::asio::ip::tcp;

void read_messages(tcp::socket& socket) {
    try {
        for (;;) {
            char data[512];
            boost::system::error_code ec;
            size_t length = socket.read_some(boost::asio::buffer(data), ec);

            if (ec == boost::asio::error::eof) {
                std::cout << "Disconnected from server." << std::endl;
                break;
            } else if (ec) {
                throw boost::system::system_error(ec);
            }
            std::cout << std::string(data, length);
        }
    } catch (std::exception& e) {
        std::cerr << "Read error: " << e.what() << std::endl;
    }
}

int main() {
    try {
        std::string server_ip;
        unsigned short server_port;
        std::string username;

        std::cout << "Enter server IP: ";
        std::cin >> server_ip;

        std::cout << "Enter server port: ";
        std::cin >> server_port;

        std::cout << "Enter your username: ";
        std::cin >> username;

        boost::asio::io_context io_context;
        tcp::socket socket(io_context);

        tcp::endpoint endpoint(boost::asio::ip::make_address(server_ip), server_port);
        socket.connect(endpoint);

        std::string username_msg = username + "\n";
        boost::asio::write(socket, boost::asio::buffer(username_msg));

        std::cout << "Connected to server as " << username << std::endl;

        std::thread reader_thread(read_messages, std::ref(socket));

        for (;;) {
            std::string message;
            std::getline(std::cin, message);
            if (message == "/bye") {
                boost::asio::write(socket, boost::asio::buffer(message + "\n"));
                break;
            }
            boost::asio::write(socket, boost::asio::buffer(message + "\n"));
        }

        socket.close();
        reader_thread.join();

    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
