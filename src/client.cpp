#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <string>
#include <atomic>
#include <limits>

using boost::asio::ip::tcp;

std::atomic<bool> running(true);
std::string global_username;

void sendMessages(tcp::socket &socket) {
    try {
        while (running) {
            std::cout << global_username << ": ";
            std::string msg;
            std::getline(std::cin, msg);

            if (msg == "/quit") {
                running = false;
                socket.close();
                break;
            }

            std::cout << global_username << ": " << msg << std::endl;

            msg += "\n";
            boost::asio::write(socket, boost::asio::buffer(msg));
        }
    } catch (...) {}
}

void receiveMessages(tcp::socket &socket) {
    try {
        boost::asio::streambuf buf;
        while (running) {
            boost::system::error_code ec;
            size_t n = boost::asio::read_until(socket, buf, "\n", ec);

            if (ec == boost::asio::error::eof || ec == boost::asio::error::connection_reset) {
                std::cout << "\nDisconnected from server." << std::endl;
                break;
            }
            if (ec) break;

            std::istream is(&buf);
            std::string message;
            std::getline(is, message);

            if (!message.empty() &&
                message.substr(0, global_username.size() + 2) == global_username + ": ") {
                continue;
            }
            std::cout << message << std::endl;
        }
    } catch (...) {}
}

int main() {
    try {
        std::string server_ip;
        unsigned short server_port;

        std::cout << "Enter server IP: ";
        std::cin >> server_ip;
        std::cout << "Enter server Port: ";
        std::cin >> server_port;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        boost::asio::io_context io_context;
        tcp::socket socket(io_context);

        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve(server_ip, std::to_string(server_port));
        boost::asio::connect(socket, endpoints);

        boost::asio::streambuf buf;
        boost::asio::read_until(socket, buf, "\n");
        std::istream is(&buf);
        std::string prompt;
        std::getline(is, prompt);
        std::cout << prompt << std::endl;

        std::cout << "Enter Username: ";
        std::getline(std::cin, global_username);
        boost::asio::write(socket, boost::asio::buffer(global_username + "\n"));

        std::thread sender(sendMessages, std::ref(socket));
        std::thread receiver(receiveMessages, std::ref(socket));
        sender.join();
        receiver.join();
    } catch (const std::exception &e) {
        std::cerr << "Connection failed: " << e.what() << std::endl;
    }
    return 0;
}
