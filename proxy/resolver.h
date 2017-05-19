#ifndef PROXY_RESOLVER_H
#define PROXY_RESOLVER_H


#include <cstdio>
#include <condition_variable>
#include <thread>
#include <queue>
#include "http_request.h"
#include "fd_wrapper.h"

class resolver {
public:
    resolver(size_t thread_count = 2);
    ~resolver();

    void resolve();
    void send();
    void stop();

    void add_task(std::unique_ptr<http_request>);
    std::unique_ptr<http_request> get_task();


    void set_fd(fd_wrapper fd);
    fd_wrapper& get_fd();

private:
    bool working = false;
    std::mutex lk;
    std::condition_variable condition;
    std::vector<std::thread> threads;
    std::queue<std::unique_ptr<http_request> > tasks, resolved;
    fd_wrapper fd;
};


#endif //PROXY_RESOLVER_H
