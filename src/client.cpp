#include <boost/asio.hpp>
#include <iostream>

using boost::asio::ip::tcp;

int main() {
    try {
        boost::asio::io_context io;

        // Connect to localhost:12345
        tcp::socket socket(io);
        tcp::resolver resolver(io);
        auto endpoints = resolver.resolve("127.0.0.1", "12345");
        boost::asio::connect(socket, endpoints);

        // Read greeting
        char buffer[128];
        size_t len = socket.read_some(boost::asio::buffer(buffer));
        std::cout << "Server says: " << std::string(buffer, len) << std::endl;
    }
    catch (std::exception& e) {
        std::cerr << "Client error: " << e.what() << std::endl;
    }
}
