#include "proxy.h"
#include <iostream>

int main()
{
	std::cout << "Введите номер порта:\n";
	int port;
	std::cin >> port;
	(new proxy(port))->start();
}
