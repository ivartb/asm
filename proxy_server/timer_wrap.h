#ifndef TIMER_WRAP_H
#define TIMER_WRAP_H

#include "utils.h"
#include "epoll_wrap.h"
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <string>

struct proxy;

template<typename type_t>
struct timer_wrap
{
	timer_wrap(type_t& type, proxy& proxy_server, epoll_wrap& epoll) : 
		fd(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)), type(type),
		task(fd, epoll, EPOLLIN, 
			[this, &proxy_server, &type](uint32_t)
			{
				logger("Timer expired for fd " + std::to_string(type.get_fd().get()));
				type.erase(proxy_server);
			}) 
	{
		settime();
	}
	
	void settime()
	{
		struct itimerspec val;
		val.it_value.tv_sec = TIMEOUT;
		val.it_value.tv_nsec = 0;
		val.it_interval.tv_sec = 0;
		val.it_interval.tv_nsec = 0;
		
		timerfd_settime(fd.get(), 0, &val, NULL);
	}
	
private:
	fd_wrap fd;
	type_t& type;
	task_wrap task;
	
	const int TIMEOUT = 60;
};

#endif //TIMER_WRAP_H
