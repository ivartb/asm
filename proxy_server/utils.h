#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <iostream>

namespace {

void handle(std::string msg)
{
	std::cerr << msg << std::endl;
	exit(-1);
}

void logger(std::string msg)
{
	std::cout << msg << std::endl;
}

}

#endif //UTILS_H
