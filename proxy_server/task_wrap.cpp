#include "task_wrap.h"

task_wrap::task_wrap(fd_wrap& fd, epoll_wrap& epoll, uint32_t events,
					 std::function<void (uint32_t)> handler) : fd(fd), epoll(epoll),
					 events(events), handler(handler)
{
	epoll.add_task(fd, *this, events);
}

task_wrap::~task_wrap()
{
	epoll.del_task(fd, *this);
}

void task_wrap::mod_task(uint32_t new_events, action act)
{
	if (act == INSERT)
		events = events | new_events;
	else
		events = events & (~new_events);
	epoll.mod_task(fd, *this, events);
}
	
fd_wrap& task_wrap::get()
{
	return fd;
}
