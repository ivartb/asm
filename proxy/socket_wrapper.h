#ifndef PROXY_SOCKET_WRAPPER_H
#define PROXY_SOCKET_WRAPPER_H


#include <bits/sockaddr.h>
#include <cstdint>
#include <netinet/in.h>
#include <string>
#include "fd_wrapper.h"
#include "utils.h"

class socket_wrapper {
public:
    socket_wrapper();

    socket_wrapper(int fd);

    socket_wrapper(socket_wrapper &&) = default;

    socket_wrapper &operator=(socket_wrapper &&) = default;

    void bind(uint16_t port);

    void listen();

    int accept();

    uint32_t get_flags();

    int get_available_bytes();

    void set_flags(uint32_t flags);

    fd_wrapper &get_fd();

    ~socket_wrapper();

    size_t write(const std::string &msg);

    std::string read(size_t buffer_size);

private:

    fd_wrapper fd;
};


#endif //PROXY_SOCKET_WRAPPER_H
