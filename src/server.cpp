#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <mutex>


using boost::asio::ip::tcp;

std::mutex console_mutex;

tcp::acceptor start_acceptor(boost::asio::io_context& io_context, const std::string& ip, unsigned short port) {
    tcp::endpoint endpoint(boost::asio::ip::make_address(ip), port);
    tcp::acceptor acceptor(io_context, endpoint);

    std::cout << "Server listening on " << ip << ":" << port << std::endl;

    return acceptor;
}


void client_session(tcp::socket socket) {
    {
        std::lock_guard<std::mutex> lock(console_mutex);
        std::cout << "Client connected from " << socket.remote_endpoint() << std::endl;
    }

    try {
        for (;;) {
            char data[512];
            boost::system::error_code ec;
            size_t length = socket.read_some(boost::asio::buffer(data), ec);

            if (ec == boost::asio::error::eof) {
                break;
            } else if (ec) {
                throw boost::system::system_error(ec);
            }

            // Echo back the data (optional here, just for testing)
            boost::asio::write(socket, boost::asio::buffer(data, length));
        }
    }
    catch (std::exception& e) {
        std::lock_guard<std::mutex> lock(console_mutex);
        std::cout << "Client session error: " << e.what() << std::endl;
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

            // Start a thread to handle this client
            std::thread(client_session, std::move(socket)).detach();
        }

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
