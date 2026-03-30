#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include <functional>


class EventLoop;
class Channel;

class Acceptor {
    
public:
    using NewConnectionCallback = std::function<void(int)>;

    Acceptor(EventLoop* loop, int port);
    ~Acceptor();

    void setNewConnectionCallback(NewConnectionCallback cb);
    void listen();


private:
    EventLoop* loop;
    int listenfd;
    Channel* channel;
    NewConnectionCallback newConnectionCallback;

    void handleRead();
    bool setNonBlocking(int fd);

};



#endif
