#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H


class EventLoop;
class Channel;

class TcpConnection {

public:
    TcpConnection(EventLoop* loop, int connfd);
    ~TcpConnection();

private:
    void handleRead();

private:
    EventLoop* loop;
    int connfd;
    Channel* channel;
};


#endif
