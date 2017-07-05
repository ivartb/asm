#include "http.h"
#include "utils.h"
#include <algorithm>
#include <string>

http_request::http_request(std::string data) : st(NEW), data(data)
{
	auto pos = data.find("Host: ");
	if (pos == std::string::npos)
	{
		st = BAD;
		return;
	}
	st = READY;
	host = data.substr(pos + 6, data.find("\r\n", pos + 6) - (pos + 6));
	pos = host.find(":");
	if (pos == std::string::npos)
	{
		port = "80";
	}
	else
	{
		port = host.substr(pos + 1);
		host = host.substr(0, pos);
	}
}

State http_request::get_state()
{
	return st;
}
	
std::string http_request::get_host()
{
	return host;
}

std::string http_request::get_port()
{
	return port;
}

std::string http_request::get_data()
{
	return data;
}

bool http_request::finished()
{
	if (st == BAD) return true;
	if (st == NEW) return false;
	auto pos = data.find("\r\n\r\n");
	if (pos == std::string::npos)
		return false;
	pos = data.find("Transfer-encoding: chunked");
	if (pos != std::string::npos)
	{
		return data.find("\r\n0\r\n\r\n") != std::string::npos;
	}
	pos = data.find("Content-length: ");
	if (pos == std::string::npos)
		return true;
	return (int)data.substr(data.find("\r\n\r\n") + 4).size() == 
		std::stoi(data.substr(pos + 16, data.find("\r\n", pos + 16) - (pos + 16)));
}

int http_request::get_fd()
{
	return fd;
}

void http_request::set_fd(int fd)
{
	this->fd = fd;
}

void http_request::set_ans_host(sockaddr host)
{
	ans_host = host;
}	
	
sockaddr http_request::get_ans_host()
{
	return ans_host;
}


http_response::http_response() : data(""){}
	
http_response::http_response(std::string str) : data(str) {}
	
void http_response::append(std::string str)
{
	data.append(str);
}
	
bool http_response::finished()
{
	auto pos = data.find("\r\n\r\n");
	if (pos == std::string::npos)
		return false;
	pos = data.find("Transfer-encoding: chunked");
	if (pos != std::string::npos)
	{
		return data.find("\r\n0\r\n\r\n") != std::string::npos;
	}
	pos = data.find("Content-length: ");
	if (pos == std::string::npos)
	{
		return true;
	}
	return (int)data.substr(data.find("\r\n\r\n") + 4).size() == 
		std::stoi(data.substr(pos + 16, data.find("\r\n", pos + 16) - (pos + 16)));
}
