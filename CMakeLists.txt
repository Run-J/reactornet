cmake_minimum_required(VERSION 3.0)
project(reactornet LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_library(reactor
    src/Acceptor.cpp
    src/Channel.cpp
    src/EventLoop.cpp
    src/TcpConnection.cpp
)

target_include_directories(reactor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)

add_executable(server app/main.cpp)

target_link_libraries(server reactor)
