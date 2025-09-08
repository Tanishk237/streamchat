#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <memory>

using boost::asio::ip::tcp;

std::mutex console_mutex;
std::mutex clients_mutex;

struct ClientInfo {
    tcp::socket socket;
    std::string username;

    ClientInfo(tcp::socket&& sock, const std::string& user)
        : socket(std::move(sock)), username(user) {}
};

std::unordered_map<int, std::shared_ptr<ClientInfo>> clients;
int next_client_id = 0;

tcp::acceptor start_acceptor(boost::asio::io_context& io_context, const std::string& ip, unsigned short port) {
    tcp::endpoint endpoint(boost::asio::ip::make_address(ip), port);
    tcp::acceptor acceptor(io_context, endpoint);

    std::cout << "Server listening on " << ip << ":" << port << std::endl;

    return acceptor;
}

void broadcast_message(const std::string& message, int sender_id) {
    std::lock_guard<std::mutex> lock(clients_mutex);

    for (auto& [id, client] : clients) {
        if (id != sender_id) {
            boost::asio::write(client->socket, boost::asio::buffer(message));
        }
    }
}

std::string read_username(tcp::socket& socket) {
    boost::asio::streambuf buf;
    boost::asio::read_until(socket, buf, "\n");
    std::istream is(&buf);
    std::string username;
    std::getline(is, username);
    return username;
}

void client_session(tcp::socket socket) {
    try {
        std::string username = read_username(socket);

        int client_id;
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            client_id = next_client_id++;
            clients[client_id] = std::make_shared<ClientInfo>(std::move(socket), username);
        }

        {
            std::lock_guard<std::mutex> lock(console_mutex);
            std::cout << "User '" << username << "' connected from " << clients[client_id]->socket.remote_endpoint() << std::endl;
        }

        auto& client_socket = clients[client_id]->socket;

        for (;;) {
            char data[512];
            boost::system::error_code ec;
            size_t length = client_socket.read_some(boost::asio::buffer(data), ec);

            if (ec == boost::asio::error::eof) {
                break; 
            } else if (ec) {
                throw boost::system::system_error(ec);
            }

            std::string message(data, length);

            if (message == "/bye\n" || message == "/bye\r\n" || message == "/bye") {
                boost::asio::write(client_socket, boost::asio::buffer("Goodbye!\n"));
                break;
            }

            std::string broadcast_text = username + ": " + message;
            broadcast_message(broadcast_text, client_id);
        }

        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.erase(client_id);
        }

        {
            std::lock_guard<std::mutex> lock(console_mutex);
            std::cout << "User '" << username << "' disconnected." << std::endl;
        }

    } catch (std::exception& e) {
        std::lock_guard<std::mutex> lock(console_mutex);
        std::cout << "Exception in client session: " << e.what() << std::endl;
    }
}

int main() {
    try {
        std::string ip;
        unsigned short port;

        std::cout << "Enter IP to listen on: ";
        std::cin >> ip;

        std::cout << "Enter port to listen on: ";
        std::cin >> port;

        boost::asio::io_context io_context;

        tcp::acceptor acceptor = start_acceptor(io_context, ip, port);

        for (;;) {
            tcp::socket socket(io_context);
            acceptor.accept(socket);

            std::thread(client_session, std::move(socket)).detach();
        }

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
