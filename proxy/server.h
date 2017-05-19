#ifndef PROXY_SERVER_H
#define PROXY_SERVER_H


#include <netinet/in.h>
#include <string>
#include "fd_wrapper.h"
#include "socket_wrapper.h"
#include "client.h"
#include "proxy_server.h"
#include "event_wrapper.h"
#include <arpa/inet.h>
#include "timer.h"

class proxy_server;


class server {
    friend class proxy_server;

    friend class timer<server>;

public:
    server(const server &) = delete;

    server &operator=(const server &) = delete;

    server(sockaddr sadddr, proxy_server &proxyServer, client &cl);

    ~server();

    fd_wrapper &get_fd();

    fd_wrapper &get_client_fd();

    void bind(client &client);

    void unbind();

    void append(std::string &);

    std::string &get_buffer();

    size_t get_buffer_size();

    std::string read();

    size_t write();

    void push_to_client();

    void push_to_server();

    void set_host(std::string const &);

    void add_flag(uint32_t flag);

    std::string get_host();

private:
    void disconnect(proxy_server &proxyServer);

    void read_response(proxy_server &proxyServer);

    void write_request(proxy_server &proxyServer);

    std::string buffer, host;
    socket_wrapper socket;
    std::string request;
    event_wrapper event;
    client *paired_client;
    timer<server> time;
};


#endif //PROXY_SERVER_H
