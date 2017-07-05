#ifndef SOCKET_WRAP_H
#define SOCKET_WRAP_H

#include "fd_wrap.h"
#include <cstdint>
#include <string>

struct socket_wrap
{
	socket_wrap();
	
	socket_wrap(int fd);
	
	void bind(uint16_t port);
	
	void listen();
	
	int accept();
	
	fd_wrap& get();
	
	std::string read();
	
	size_t write(std::string& data);
	
private:
	fd_wrap fd;
};

#endif //SOCKET_WRAP_H
