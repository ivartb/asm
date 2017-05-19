#include <iostream>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <csignal>
#include "proxy_server.h"
#include "utils.h"

proxy_server::proxy_server(int port) : epoll_socket(::socket(AF_INET, SOCK_STREAM, 0)),
                                       rslvr(8/*sysconf(_SC_NPROCESSORS_ONLN)*/) {
    log_msg("Proxy server created at port: " + std::to_string(port));
    int one = 1;
    setsockopt(epoll_socket.get_fd().get_fd(), SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int));
    epoll_socket.bind(port);
    epoll_socket.listen();
    epoll_socket.get_fd().make_non_blocking();
    log_msg("Listening started");
    epoll_socket.set_flags(epoll_socket.get_flags() | EPOLLIN);
    int fd[2];
    pipe(fd);
    pipe_fd = fd_wrapper(fd[0]);
    fd_wrapper resolver_fd(fd[1]);

    pipe_fd.make_non_blocking();
    resolver_fd.make_non_blocking();
    rslvr.set_fd(std::move(resolver_fd));
    log_msg("Pipe fds: " + std::to_string(rslvr.get_fd().get_fd()) + " " + std::to_string(pipe_fd.get_fd()));
    resolver_event = std::unique_ptr<event_wrapper>(new event_wrapper(epoll, pipe_fd, EPOLLIN,
                                                [this](uint32_t events)mutable throw(std::runtime_error) {
                                                    log_msg("Resolver handler working");
                                                    this->resolver_handler();
                                                }));
    listen_event = std::unique_ptr<event_wrapper>(new event_wrapper(epoll, epoll_socket.get_fd(), EPOLLIN,
                                                          [this](uint32_t events)mutable throw(std::runtime_error) {
                                                              connect_client();
                                                          }));
    log_msg("Listener added to epoll");
}

proxy_server::~proxy_server() {
    log_msg("Proxy server stopped.");
    rslvr.stop();
}

void proxy_server::run() {
    log_msg("Proxy server running");
    epoll.run();
}


void proxy_server::connect_client() {
    auto client_fd = this->epoll_socket.accept();
    log_msg("Client socket matched to fd: " + std::to_string(client_fd));
    clients[client_fd] = std::unique_ptr<client>(new client(client_fd, *this));
    log_msg(std::to_string(clients.size()) + " clients now in epoll.");
}


epoll_wrapper &proxy_server::get_epoll() {
    return epoll;
}

void proxy_server::resolver_handler() {
    char tmp;
    if (read(pipe_fd.get_fd(), &tmp, sizeof(tmp)) == -1) {
        perror("Reading from resolver failed");
    }

    std::unique_ptr<http_request> cur_request = rslvr.get_task();
    log_msg("Resolver called to resolve host " + cur_request->get_host());

    log_msg("Find client fd " + std::to_string(cur_request->get_client_fd()));
    client *cur_client = clients[cur_request->get_client_fd()].get();

    if (cur_client == nullptr) {
        //throw_my_error("No client");
        return;
    }

    server *srvr;

    try {
        struct sockaddr result = cur_request->get_resolved_host();
        srvr = new server(result, *this, *cur_client);
    } catch (...) {
        throw_my_error("Error connecting to server");
    }

    cur_client->time.reset();

    srvr->set_host(cur_request->get_host());
    cur_client->bind(*srvr);
    log_msg("Server with fd = " + std::to_string(srvr->get_fd().get_fd()) +
			" matched to client with fd = " + std::to_string(cur_client->get_fd().get_fd()));
    srvr->add_flag(EPOLLOUT);
    servers[srvr->get_fd().get_fd()] = srvr;
    log_msg(std::to_string(servers.size()) + " servers now in epoll");
    cur_client->get_buffer() = std::move(cur_request->get_data());
    cur_client->flush_client_buffer();
}

void proxy_server::erase_server(int fd) {
    //log_msg("Erasing server, fd = " + std::to_string(fd) + " host = " + servers[fd]->get_host().c_str());
    servers.erase(fd);
    log_msg(std::to_string(servers.size()) +  " servers left in epoll");
}


void proxy_server::add_task(std::unique_ptr<http_request> request) {
	rslvr.add_task(std::move(request));
}

void proxy_server::erase_client(int fd) {
	//log_msg("Erasing client, fd = " + std::to_string(fd) + " host = " + clients[fd]->get_host().c_str());
	clients.erase(fd);
	log_msg(std::to_string(clients.size()) + " servers left in epoll");
}


lru_cache<std::string, http_response> &proxy_server::get_cache() {
    return cache;
}
