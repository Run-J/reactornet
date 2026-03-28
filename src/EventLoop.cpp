#include "../inc/EventLoop.h"
#include "../inc/Channel.h"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>
#include <cstring>


EventLoop::EventLoop()
    : running(false), activeEvents(16) {

    epollfd = epoll_create1(0);
    if (epollfd == -1) {
        perror("epoll_create1");
        std::exit(EXIT_FAILURE);
    }

}


EventLoop::~EventLoop() {
    close(epollfd);
}


void EventLoop::loop() {
    running = true;

    while (running) {

        int numEvents = epoll_wait(epollfd, activeEvents.data(),
                                   static_cast<int>(activeEvents.size()), -1);

        if (numEvents == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("epoll_wait");
            break;
        }

        if (numEvents == static_cast<int>(activeEvents.size()) ) { // Expand the buffer if needed
            activeEvents.resize(activeEvents.size() * 2);
        }

        
        for (int i = 0; i < numEvents; i++) {
            int fd = activeEvents[i].data.fd;
            auto it = channels.find(fd);
            if (it != channels.end()) {
                Channel* channel = it->second;
                channel->setRevents(activeEvents[i].events);
                channel->handleEvent();
            }

        }

    }
}


void EventLoop::updateChannel(Channel* channel) {
    epoll_event ev;
    std::memset(&ev, 0, sizeof(ev));
    ev.events = channel->getEvents();
    ev.data.fd = channel->getFd(); 

    int fd = channel->getFd();

    if (channels.find(fd) == channels.end()) { // The channel is not registered yet
        
       if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
            perror("epoll_ctl ADD");
            return;
        }
        channels[fd] = channel;

    } else { // The channel is already registered
        
        if (channel->getEvents() == 0) { // The channel no longer wants to listen to any events 
            if (epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, nullptr) == -1) {
                perror("epoll_ctl DEL");
            }

            channels.erase(fd);
        } else {
            if (epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev) == -1) {
                perror("epoll_ctl MOD");
            }
        }

    }

}


void EventLoop::removeChannel(Channel* channel) {
    int fd = channel->getFd();

    if (channels.find(fd) != channels.end()) {
        if (epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, nullptr) == -1) {
            perror("epoll_ctl DEL"); 
        }
        channels.erase(fd);
    }
}






