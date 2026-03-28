#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <unordered_map>
#include <vector>
#include <sys/epoll.h>

class Channel;

class EventLoop {

public:
    // Constructor
    EventLoop();

    // Destructor
    ~EventLoop();

    // Mutators
    void loop();
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);


private:
    int epollfd;
    bool running;
    std::unordered_map<int, Channel*> channels;  // Maps fd to Channel*
    std::vector<epoll_event> activeEvents; // Stores events returned by epoll_wait

}; 

#endif
