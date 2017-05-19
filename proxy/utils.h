#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <stdexcept>
#include <vector>
#include <mutex>
#include <iostream>
#include <execinfo.h>
#include <stdlib.h>


inline void throw_my_error(std::string msg) {
	throw std::runtime_error(msg);
}

inline void log_msg(std::string msg) {
	std::cerr << msg << std::endl;
}

#endif // UTILS_H