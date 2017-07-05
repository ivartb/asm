#ifndef EPOLL_WRAP_H
#define EPOLL_WRAP_H

#include "fd_wrap.h"
#include "task_wrap.h"
#include <set>
#include <cstdint>

int const TIMEOUT = 10;
int const EPOLL_TASKS = 1000;

struct task_wrap;

struct epoll_wrap
{
	epoll_wrap();

	~epoll_wrap();

	void add_task(fd_wrap& fd, task_wrap& task, uint32_t events);

	void mod_task(fd_wrap& fd, task_wrap& task, uint32_t events);

	void del_task(fd_wrap& fd, task_wrap& task);

	void start();

private:
	int epoll_fd;
	std::set<task_wrap*> tasks;
	
	fd_wrap add_signalfd_to_epoll();
};

#endif //EPOLL_WRAP_H





