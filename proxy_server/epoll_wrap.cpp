#include "epoll_wrap.h"
#include "utils.h"
#include "fd_wrap.h"
#include "task_wrap.h"
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <signal.h>
#include <errno.h>


epoll_wrap::epoll_wrap()
{
	if ((epoll_fd = epoll_create(EPOLL_TASKS)) == -1)
		handle("Cannot create epoll");
}

epoll_wrap::~epoll_wrap()
{
	close(epoll_fd);
}

void epoll_wrap::add_task(fd_wrap& fd, task_wrap& task, uint32_t events)
{
	struct epoll_event event;
	event.events = events;
	event.data.ptr = &task;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd.get(), &event) == -1)
		logger("Error in epoll_ctl_add");
	tasks.insert(&task);
}

void epoll_wrap::mod_task(fd_wrap& fd, task_wrap& task, uint32_t events)
{
	struct epoll_event event;
	event.events = events;
	event.data.ptr = &task;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd.get(), &event) == -1)
		logger("Error in epoll_ctl_mod");
}

void epoll_wrap::del_task(fd_wrap& fd, task_wrap& task)
{
	struct epoll_event event;
	event.events = 0;
	event.data.ptr = &task;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd.get(), &event) == -1)
		logger("Error in epoll_ctl_del");
	tasks.erase(&task);
}

void epoll_wrap::start()
{
	fd_wrap signal_fd = epoll_wrap::add_signalfd_to_epoll();
	task_wrap sigtask(signal_fd, *this, EPOLLIN, [this](uint32_t){logger("Signal found");exit(0);});
	epoll_event events[EPOLL_TASKS];
	logger("Epoll started");
        
	while (true)
	{
		int epoll_count = epoll_wait(epoll_fd, events, EPOLL_TASKS, TIMEOUT * 1000);
		if (epoll_count == -1)
		{
			if (errno == EINTR)
				continue;
			else
				handle("Error in epoll_wait");
		}
		logger("Found events: " + std::to_string(epoll_count));
        for (int i = 0; i < epoll_count; i++)
        {
			epoll_event& event = events[i];
			task_wrap* task = static_cast<task_wrap*>(event.data.ptr);
			if (tasks.find(task) == tasks.end())
			{
				logger("Bad task at fd " + std::to_string(task->get().get()));
				del_task(task->get(), *task);
			}
			else
				task->handler(event.events);
		}
	}
}

fd_wrap epoll_wrap::add_signalfd_to_epoll()
{
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);

    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1)
        handle("Error in sigprocmask");

    int sigfd = signalfd(-1, &mask, SFD_NONBLOCK);
    if (sigfd == -1)
        handle("Cannot create signalfd");

    return fd_wrap(sigfd);
}








