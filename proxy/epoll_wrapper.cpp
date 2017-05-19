#include <sys/epoll.h>
#include "epoll_wrapper.h"
#include <csignal>
#include "sys/signalfd.h"

epoll_wrapper::epoll_wrapper() {
    epoll_fd = ::epoll_create(MAX_EPOLL_EVENTS_COUNT);
    if (epoll_fd == -1) {
        throw_my_error("Error epoll_create()");
    }

}


void epoll_wrapper::run() {

    working = true;
    fd_wrapper signal_fd = create_signal_fd();
    event_wrapper signal_event(*this, signal_fd, EPOLLIN, [this](uint32_t) {
        perror("Signal caught\n");
        this->working = false;
    });
    epoll_event events[MAX_EPOLL_EVENTS_COUNT];
    log_msg("Epoll_events created");
    while (working) {
        log_msg("Doing epoll_wait");
        auto events_count = epoll_wait(epoll_fd, events, MAX_EPOLL_EVENTS_COUNT, DEFAULT_EPOLL_TIMEOUT);
        if (events_count < 0) {
            if (errno != EINTR) {
                throw_my_error("Error occured during epoll_wait");
            } else {
                break;
            }
        }else {
            log_msg("Found " + std::to_string(events_count) + " events");
        }

        for (auto i = 0; i < events_count; i++) {
            auto &ev = events[i];
            event_wrapper *x = (event_wrapper *) (ev.data.ptr);
            if (available.find(x) != available.end()) {
                //log_msg("Callback in");
                x->callback(ev.events);
                //log_msg("Callback out");
            } else {
                log_msg("Event_wrapper " + std::to_string(x->get_fd().get_fd()) + " is dead");
            }
        }
    }

}

void epoll_wrapper::add(fd_wrapper &fd, event_wrapper &event, uint32_t flags) {
    available.insert(&event);
    operate(EPOLL_CTL_ADD, fd.get_fd(), event, flags);

}

void epoll_wrapper::remove(fd_wrapper &fd, event_wrapper &event, uint32_t flags) {
    available.erase(&event);
    operate(EPOLL_CTL_DEL, fd.get_fd(), event, 0);

}

void epoll_wrapper::modify(fd_wrapper &fd, event_wrapper &event, uint32_t flags) {
    operate(EPOLL_CTL_MOD, fd.get_fd(), event, flags);

}

epoll_wrapper::~epoll_wrapper() {
    close(epoll_fd);
	log_msg("Epoll destroyed");
}


void epoll_wrapper::operate(int op, int fd, event_wrapper &event, uint32_t flags) {
    struct epoll_event e_event;

    e_event.data.ptr = &event;
    e_event.events = flags;

    if (epoll_ctl(epoll_fd, op, fd, &e_event) == -1) {
        throw_my_error("Error during operation on epoll_wrapper event " + std::to_string(fd));
    }

}


int &epoll_wrapper::get_fd() {
    return epoll_fd;
}


fd_wrapper epoll_wrapper::create_signal_fd() {
    sigset_t mask;
    sigemptyset(&mask);
    if (sigaddset(&mask, SIGINT) == -1) {
        log_msg("Invalid signal to block" + std::to_string(SIGINT));
    }
    if (sigaddset(&mask, SIGTERM) == -1) {
		log_msg("Invalid signal to block" + std::to_string(SIGTERM));
    }

    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1) {
        throw_my_error("Error during sigprocmask");
    }

    int signal_fd = signalfd(-1, &mask, SFD_CLOEXEC | SFD_NONBLOCK);
    if (signal_fd == -1) {
        throw_my_error("Error during creating signal_fd");
    }
	log_msg("Signal fd created " + std::to_string(signal_fd));
    return fd_wrapper(signal_fd);
}


