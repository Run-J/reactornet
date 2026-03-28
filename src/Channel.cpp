#include "../inc/Channel.h"
#include "../inc/EventLoop.h"

Channel::Channel(EventLoop* loop, int fd)
    : loop(loop), fd(fd), events(0), revents(0) {

}


Channel::~Channel() {

}


int Channel::getFd() const {
    return fd;
}


uint32_t Channel::getEvents() const {
    return events;
}


void Channel::setRevents(uint32_t revents) {
    this->revents = revents;
}


void Channel::enableReading() {
    events |= EPOLLIN;
    loop->updateChannel(this);
}


void Channel::disableAll() {
    events = 0;
    loop->updateChannel(this);
}


void Channel::setReadCallback(EventCallback cb){
    readCallback = cb;
}


void Channel::handleEvent() {
    if ( (revents & EPOLLIN) && readCallback ) {
        readCallback();
    }
}
