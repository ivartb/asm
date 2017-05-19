#ifndef PROXY_EPOLL_H
#define PROXY_EPOLL_H


#include <set>
#include <memory>
#include "fd_wrapper.h"
#include "event_wrapper.h"

const int DEFAULT_EPOLL_TIMEOUT = 10000;
const int MAX_EPOLL_EVENTS_COUNT = 1000;


class event_wrapper;

class epoll_wrapper {
    friend class event_wrapper;

public:
    epoll_wrapper();

    epoll_wrapper(epoll_wrapper const &) = delete;

    ~epoll_wrapper();

    int &get_fd();

    void run();

    void add(fd_wrapper &fd, event_wrapper &event, uint32_t flags);

    void remove(fd_wrapper &fd, event_wrapper &event, uint32_t flags);

    void modify(fd_wrapper &fd, event_wrapper &event, uint32_t flags);

private:
    fd_wrapper create_signal_fd();

    void operate(int op, int fd, event_wrapper &event, uint32_t flags);

    bool working;
    int epoll_fd;
    std::set<event_wrapper *> available;

};


#endif //PROXY_EPOLL_H
