#include "socket_wrap.h"
#include "utils.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <unistd.h>

socket_wrap::socket_wrap() : fd() {}

socket_wrap::socket_wrap(int fd) : fd(fd) 
{
	this->fd.setfd(this->fd.getfd() | SOCK_STREAM | SOCK_NONBLOCK);
}

void socket_wrap::bind(uint16_t port)
{
    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    if ((::bind(fd.get(), (struct sockaddr *) &server, sizeof(server))) == -1)
        handle("Bind error");
}

void socket_wrap::listen() 
{
    if ((::listen(fd.get(), SOMAXCONN)) == -1)
        handle("Listen error");
}

int socket_wrap::accept()
{
	int client_fd = ::accept(fd.get(), NULL, NULL);
    if (client_fd == -1)
        handle("Client accept error");
	return client_fd;
}

fd_wrap& socket_wrap::get()
{
	return fd;
}

std::string socket_wrap::read()
{
	char buf[4096];
	ssize_t red = ::read(fd.get(), buf, 4096);
	if (red == -1) 
		return "";
	return std::string(buf, buf + red);
}

size_t socket_wrap::write(std::string& data)
{
	ssize_t writ = ::write(fd.get(), data.c_str(), data.size());
	if (writ == -1)
		return 0;
	return writ;
}











