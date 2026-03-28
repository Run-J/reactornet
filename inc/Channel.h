#ifndef CHANNEL_H
#define CHANNEL_H

#include <sys/epoll.h>
#include <functional>

class EventLoop;

class Channel {

public:
    using EventCallback = std::function<void()>;

    // Constructor
    Channel (EventLoop* loop, int fd);

    // Destructor
    ~Channel();

    // Getters
    int getFd() const;
    uint32_t getEvents() const; // Events this channel is interested in (e.g., EPOLLIN)
    
    // Mutators
    void setRevents(uint32_t revents); // Set the actual events returned by epoll

    void enableReading(); // Enable read event (EPOLLIN)
    void disableAll(); // Disable all events

    void setReadCallback(EventCallback cb);

    void handleEvent();


private:
    EventLoop* loop; // Associated EventLoop, used to update epoll events
    int fd;
    uint32_t events; // Interested events (registered to epoll)
    uint32_t revents; // Actual events returned by epoll
    EventCallback readCallback; // Callback for read event (EPOLLIN)

};

#endif
