#ifndef PROXY_EVENT_H
#define PROXY_EVENT_H


#include "fd_wrapper.h"
#include "epoll_wrapper.h"

class epoll_wrapper;

class event_wrapper {
    friend class epoll_wrapper;

public:


    event_wrapper(epoll_wrapper &service, fd_wrapper &fd, uint32_t flags, std::function<void(uint32_t)> handler);


    ~event_wrapper();

    fd_wrapper &get_fd() {
        return fd;
    }

    void add_flag(uint32_t flag);

    void remove_flag(uint32_t flag);

    std::function<void(uint32_t)> callback;

private:
    epoll_wrapper &service;
    fd_wrapper &fd;
    uint32_t flags;
};


#endif //PROXY_EVENT_H
