#ifndef PROXY_FILE_DESCRIPTOR_WRAPPER_H
#define PROXY_FILE_DESCRIPTOR_WRAPPER_H

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <cstdint>
#include "utils.h"
#include "string.h"


struct fd_wrapper {

public:

    fd_wrapper &operator=(const fd_wrapper &rhs) = delete;

    fd_wrapper(const fd_wrapper &) = delete;


    fd_wrapper();

    fd_wrapper(int fd);

    fd_wrapper(fd_wrapper &&fd);

    fd_wrapper &operator=(fd_wrapper &&rhs);

    ~fd_wrapper();

    uint32_t get_flags();

    void set_flags(uint32_t nex_flags);

    int get_available_bytes();

    void make_non_blocking();

    void close();

    int &get_fd();

private:

    int fd;
};


#endif //PROXY_FILE_DESCRIPTOR_WRAPPER_H
