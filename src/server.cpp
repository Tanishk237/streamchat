#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include <memory>
#include <mutex>
#include <algorithm>

using boost::asio::ip::tcp;

struct Client {
    std::string username;
    std::shared_ptr<tcp::socket> socket;
};

std::vector<Client> clients;
std::mutex clients_mutex;

void safeWrite(std::shared_ptr<tcp::socket> socket, const std::string &msg) {
    boost::system::error_code ec;
    boost::asio::write(*socket, boost::asio::buffer(msg), ec);
    if (ec) {
        std::cerr << "Send error: " << ec.message() << std::endl;
    }
}

void broadcastMessage(const std::string &msg, std::shared_ptr<tcp::socket> sender) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (const auto &client : clients) {
        safeWrite(client.socket, msg);
    }
}

void handleClient(std::shared_ptr<tcp::socket> socket) {
    try {
        boost::asio::streambuf buf;
        std::string username;

        safeWrite(socket, "Please enter your username:\n");
        boost::asio::read_until(*socket, buf, "\n");
        std::istream is(&buf);
        std::getline(is, username);
        if (username.empty()) username = "Unknown";

        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.push_back({username, socket});
        }

        std::string join_msg = username + " joined the chat.\n";
        std::cout << join_msg;
        broadcastMessage(join_msg, socket);

        while (true) {
            boost::asio::streambuf read_buf;
            boost::system::error_code ec;
            size_t n = boost::asio::read_until(*socket, read_buf, "\n", ec);

            if (ec == boost::asio::error::eof || ec == boost::asio::error::connection_reset) break;
            if (ec) throw boost::system::system_error(ec);

            std::istream msg_stream(&read_buf);
            std::string message;
            std::getline(msg_stream, message);

            if (message == "/quit") break;

            std::string formatted = username + ": " + message + "\n";
            std::cout << formatted;
            broadcastMessage(formatted, socket);
        }
    } catch (const std::exception &e) {
        std::cerr << "Client error: " << e.what() << std::endl;
    }

    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.erase(std::remove_if(clients.begin(), clients.end(),
                                     [&](const Client& c){ return c.socket == socket; }),
                      clients.end());
    }
    socket->close();
    std::cout << "Client disconnected\n";
}

int main() {
    try {
        std::string ip;
        unsigned short port;
        std::cout << "Enter IP address to bind: ";
        std::cin >> ip;
        std::cout << "Enter port: ";
        std::cin >> port;

        boost::asio::io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint(boost::asio::ip::make_address(ip), port));
        std::cout << "Server listening on " << ip << ":" << port << "\n";

        while (true) {
            auto socket = std::make_shared<tcp::socket>(io_context);
            acceptor.accept(*socket);
            std::cout << "New client connected\n";
            std::thread(handleClient, socket).detach();
        }
    } catch (const std::exception &e) {
        std::cerr << "Server error: " << e.what() << std::endl;
    }
    return 0;
}
