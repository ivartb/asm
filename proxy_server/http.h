#ifndef HTTP_H
#define HTTP_H

#include "utils.h"
#include <arpa/inet.h>
#include <string>
#include <algorithm>
#include <unordered_map>

enum State {NEW, BAD, READY};

struct http_request
{
	http_request(std::string data);
	
	State get_state();
	
	std::string get_host();
	
	std::string get_port();
	
	std::string get_data();
	
	bool finished();
	
	int get_fd();
	
	void set_fd(int fd);
	
	void set_ans_host(sockaddr host);
	
	sockaddr get_ans_host();
	
private:
	State st;
	std::string data, host, port;
	sockaddr ans_host;
	int fd;	
};

struct http_response
{
	http_response();
	
	http_response(std::string str);
	
	void append(std::string str);
	
	bool finished();

private:
	std::string data;
};


#endif //HTTP_H
