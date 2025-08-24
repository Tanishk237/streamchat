cmake_minimum_required(VERSION 3.15)
project(TcpChatApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Headers
include_directories(include)

# Common library
add_library(common
    src/common/message.cpp
    include/common/message.hpp
)

# Server
add_executable(server
    src/server/main.cpp
    src/server/server.cpp
)
target_link_libraries(server PRIVATE common)

# Client
add_executable(client
    src/client/main.cpp
    src/client/client.cpp
)
target_link_libraries(client PRIVATE common)

# Tests
enable_testing()
add_executable(test_message tests/test_message.cpp)
target_link_libraries(test_message PRIVATE common)
add_test(NAME MessageTest COMMAND test_message)
