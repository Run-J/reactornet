#include "../inc/EventLoop.h" 
#include "../inc/TcpConnection.h" 
#include "../inc/Channel.h"

#include <cerrno>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>

TcpConnection::TcpConnection(EventLoop* loop, int connfd)
    : loop(loop), connfd(connfd) {

    channel = new Channel(loop, connfd);
    channel->setReadCallback(std::bind(&TcpConnection::handleRead, this));
    channel->enableReading();
}


TcpConnection::~TcpConnection() {
    loop->removeChannel(channel);
    close(connfd);
    delete channel;
    std::cout << "Connection closed, fd = " << connfd << std::endl;
}


void TcpConnection::handleRead() {
    char buffer[1024];

    while (true) {
        ssize_t n = read(connfd, buffer, sizeof(buffer));

        if (n > 0) {
            std::cout << "Received from fd " << connfd << ": ";
            std::cout.write(buffer, n);
            std::cout << std::endl;
            write(connfd, buffer, n);
        } else if (n == 0) {
            delete this;
            break;
        } else {
            if (errno == EINTR) continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            // error
            perror("read");
            delete this;
            break;
        }
    }
}

















