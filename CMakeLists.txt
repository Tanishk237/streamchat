cmake_minimum_required(VERSION 3.10)
project(StreamChat)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost 1.70 REQUIRED)

# Server
add_executable(server src/server.cpp)
target_link_libraries(server PRIVATE Boost::boost)

# Client
add_executable(client src/client.cpp)
target_link_libraries(client PRIVATE Boost::boost)
