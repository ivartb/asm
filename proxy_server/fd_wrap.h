#ifndef FD_H
#define FD_H

#include <cstdint>

struct fd_wrap
{
	fd_wrap();

	fd_wrap(int fd);
	
	~fd_wrap();

	int& get();

	void set_non_block();
	
	uint32_t getfd();
	
	void setfd(uint32_t flags);

private:
	int fd;
};


#endif //FD_H
