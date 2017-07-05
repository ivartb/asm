#ifndef TASK_WRAP_H
#define TASK_WRAP_H

#include "epoll_wrap.h"
#include "fd_wrap.h"
#include <cstdint>
#include <functional>

struct epoll_wrap;

struct task_wrap
{
	enum action {INSERT, REMOVE};
	
	task_wrap(fd_wrap& fd, epoll_wrap& epoll, uint32_t events, std::function<void (uint32_t)> handler);
	
	~task_wrap();
	
	void mod_task(uint32_t events, action act);
	
	fd_wrap& get();

private:
	fd_wrap& fd;
	epoll_wrap& epoll;
	uint32_t events;
	std::function<void (uint32_t)> handler;
	
	friend class epoll_wrap;
};

#endif //TASK_WRAP_H



