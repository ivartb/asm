#include "fd_wrap.h"
#include "utils.h"
#include <unistd.h>
#include <fcntl.h>

fd_wrap::fd_wrap() : fd(-1) {}

fd_wrap::fd_wrap(int fd) : fd(fd) {}

fd_wrap::~fd_wrap()
{
	if (fd != -1)
		close(fd);	
}

int& fd_wrap::get()
{
	return fd;
}

void fd_wrap::set_non_block() {
	int flags;
    if ((flags = fcntl(fd, F_GETFL, 0)) == -1)
        flags = 0;
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
		handle("set_non_block failed");
}

uint32_t fd_wrap::getfd() 
{
    auto ans = fcntl(fd, F_GETFD);
    if (ans == -1)
        ans = 0;
    return ans;
}

void fd_wrap::setfd(uint32_t flags) 
{
    fcntl(fd, F_SETFD, flags);
}















