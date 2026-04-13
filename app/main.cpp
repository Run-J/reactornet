#include "../inc/EventLoop.h"
#include "../inc/Acceptor.h"
#include "../inc/TcpConnection.h"

#include <iostream>


int main() {
    const int PORT = 8080;

    EventLoop loop;
    Acceptor acceptor(&loop, 8080);

    std::unordered_map<int, TcpConnection*> connections;

    acceptor.setNewConnectionCallback([&](int connfd) {
        TcpConnection* conn = new TcpConnection(&loop, connfd);
        connections[connfd] = conn;
    });

    std::cout << "Server started on port " << PORT << std::endl;

    acceptor.listen();
    loop.loop();

    return 0;
}
