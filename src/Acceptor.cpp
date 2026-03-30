#include "../inc/Acceptor.h"
#include "../inc/Channel.h"
#include "../inc/EventLoop.h"

#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
#include <cstring>
#include <cstdlib>

Acceptor::Acceptor(EventLoop* loop, int port)
    : loop(loop), listenfd(-1), channel(nullptr) {
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        perror("socket");
        std::exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt");
        close(listenfd);
        std::exit(EXIT_FAILURE);
    }

    if (!setNonBlocking(listenfd)) {
        close(listenfd);
        std::exit(EXIT_FAILURE);
    }

    sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(listenfd, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
        perror("bind");
        close(listenfd);
        std::exit(EXIT_FAILURE);
    }

    if (::listen(listenfd, SOMAXCONN) == -1) {
        perror("listen");
        close(listenfd);
        std::exit(EXIT_FAILURE);
    }

    channel = new Channel(loop, listenfd);
    channel->setReadCallback(std::bind(&Acceptor::handleRead, this));
}


Acceptor::~Acceptor() {
    delete channel;
    close(listenfd);
}


void Acceptor::setNewConnectionCallback(NewConnectionCallback cb) {
    newConnectionCallback = cb;
}


void Acceptor::listen() {
    channel->enableReading();
}


bool Acceptor::setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL");
        return false;
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL");
        return false;
    }

    return true;
}


void Acceptor::handleRead() {
    while (true) {
        sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);

        int connfd = accept(listenfd, reinterpret_cast<sockaddr*>(&clientAddr), &clientLen);
        if (connfd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            perror("accept");
            break;
        }

        if (!setNonBlocking(connfd)) {
            close(connfd);
            continue;
        }

        std::cout << "New connection accepted, fd = " << connfd << std::endl;

        if (newConnectionCallback) {
            newConnectionCallback(connfd);
        }
    }
}
