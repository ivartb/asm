#include "fd_wrapper.h"

fd_wrapper::fd_wrapper() : fd(-1) {

}


fd_wrapper::fd_wrapper(int fd) : fd(fd) {
}

fd_wrapper::~fd_wrapper() {
   // std::cerr<<"Fd "<<get_fd()<<"\n";
    close();
}

void fd_wrapper::make_non_blocking() {
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
        throw_my_error("Error during fcntl nonblocking!");
    }
}

void fd_wrapper::close() {
    if (fd == -1) {
        return;
    }
    if (::close(fd) == -1) {
        if (errno != EAGAIN) {
            throw_my_error("Error during closing file descriptor: " + std::to_string(fd));
        }
    }

}

int &fd_wrapper::get_fd() {
    return fd;
}


uint32_t fd_wrapper::get_flags() {
    auto result = fcntl(fd, F_GETFD);
    if (result == -1) {
        throw_my_error("Error during getting flags:" + std::to_string(fd));
    }

    return result;
}

void fd_wrapper::set_flags(uint32_t flags) {
    if (fcntl(fd, F_SETFD, flags) == -1) {
        throw_my_error("Error during setting flags: " + std::to_string(fd));
    }
}

int fd_wrapper::get_available_bytes() {
    int avail_bytes;
    if (ioctl(fd, FIONREAD, &avail_bytes) == 1) {
        throw_my_error("Error during getting availible bytes");
    }
    return avail_bytes;
}

fd_wrapper::fd_wrapper(fd_wrapper &&fd) : fd(fd.fd) {
    fd.fd = -1;
}

fd_wrapper &fd_wrapper::operator=(fd_wrapper &&rhs) {
    fd = rhs.fd;
    rhs.fd = -1;

    return *this;
}
