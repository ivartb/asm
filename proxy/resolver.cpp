#include <csignal>
#include "resolver.h"

resolver::resolver(size_t thread_count) {
    working = true;
    try {
        for (size_t i = 0; i < thread_count; i++) {
            threads.push_back(std::thread([this]() {
                this->resolve();
            }));
        };
        log_msg("Resolver created with " + std::to_string(thread_count) + " threads");
    } catch (...) {
        working = false;
        for (size_t i = 0; i < threads.size(); i++) {
            if (threads[i].joinable()) {
                threads[i].join();
            }
        }
        throw;
    }
}

resolver::~resolver() {
	log_msg("Resolver destroyed");
    stop();
}

void resolver::resolve() {
    sigset_t mask;
    sigfillset(&mask);
    sigprocmask(SIG_BLOCK, &mask, nullptr);
    while (working) {

        std::unique_lock<std::mutex> lock{lk};
        condition.wait(lock, [&]() {
            return (!tasks.empty() || !working);
        });


        if (!working) {
            return;
        }
        log_msg("Resolve started");
        auto request = std::move(tasks.front());
        tasks.pop();

        lock.unlock();
        struct addrinfo hints, *res;
        memset(&hints, 0, sizeof(hints));

        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        int err_no = getaddrinfo(request->get_host().c_str(), request->get_port().c_str(), &hints, &res);

        sockaddr host;

        if (err_no != 0) {
            perror("Resolver: error while resolving");
        } else {
            host = *res->ai_addr;
            freeaddrinfo(res);
            lock.lock();
            lock.unlock();
        }
        request->set_resolved_host(host);
        log_msg("Host resolved " + request->get_host());

        lock.lock();
        resolved.push(std::move(request));
        lock.unlock();

        send();
        log_msg("Resolve finished");
    }
}


void resolver::send() {
    std::unique_lock<std::mutex> lock{lk};
    char tmp = 'a';
    ssize_t cnt = write(fd.get_fd(), &tmp, sizeof(tmp));
    if (cnt == -1) {
        lock.unlock();
        perror("Resolver: error while sending message to proxy server");
    }
}

void resolver::stop() {
    std::unique_lock<std::mutex> lock{lk};
    working = false;
    condition.notify_all();
    lock.unlock();
    for (size_t i = 0; i < threads.size(); i++) {
        if (threads[i].joinable()) {
            threads[i].join();
        }
    }
}

void resolver::add_task(std::unique_ptr<http_request> request) {
    if (!working) {
        throw_my_error("Resolver not works");
    }
    std::unique_lock<std::mutex> lock{lk};
    tasks.push(std::move(request));
    condition.notify_one();
}

std::unique_ptr<http_request> resolver::get_task() {
    std::unique_lock<std::mutex> lock{lk};
    auto request = std::move(resolved.front());
    resolved.pop();
    return request;
}

void resolver::set_fd(fd_wrapper given_fd) {
    fd = std::move(given_fd);
}

fd_wrapper &resolver::get_fd() {
    return fd;
}
