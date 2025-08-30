#include <boost/asio.hpp>
#include <iostream>

using boost::asio::ip::tcp;

int main() {
    try {
        boost::asio::io_context io;

        // Listen on port 12345
        tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), 12345));
        std::cout << "Server started. Waiting for client..." << std::endl;

        // Accept one client
        tcp::socket socket(io);
        acceptor.accept(socket);
        std::cout << "Client connected!" << std::endl;

        // Send greeting
        std::string message = "Welcome to StreamChat!\n";
        boost::asio::write(socket, boost::asio::buffer(message));
    }
    catch (std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
    }
}
