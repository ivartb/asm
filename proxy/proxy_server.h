#ifndef PROXY_PROXY_SERVER_H
#define PROXY_PROXY_SERVER_H

#include <map>
#include "fd_wrapper.h"
#include "socket_wrapper.h"
#include "epoll_wrapper.h"
#include "client.h"
#include "resolver.h"
#include "arpa/inet.h"
#include "lru_cache.h"
#include "http_request.h"
#include "http_response.h"

class client;
class server;
class proxy_server {
public:
    proxy_server(int port);

    proxy_server(proxy_server const& rhs) = delete;
    proxy_server& operator=(proxy_server const& rhs) = delete;

    ~proxy_server();

    epoll_wrapper& get_epoll();

    void run();
    void erase_server(int fd);
    void erase_client(int fd);
    void add_task(std::unique_ptr<http_request> request);
    lru_cache<std::string, http_response>& get_cache();


private:

    void connect_client();
    void resolver_handler();

    epoll_wrapper epoll;
    bool working;
    socket_wrapper epoll_socket;
    std::map<uintptr_t, std::unique_ptr<client> > clients;
    std::map<uintptr_t, server* > servers;
    lru_cache<std::string, http_response> cache;
    fd_wrapper pipe_fd;
    class resolver rslvr;
    std::unique_ptr<event_wrapper> listen_event;
    std::unique_ptr<event_wrapper> resolver_event;
};


#endif //PROXY_PROXY_SERVER_H
