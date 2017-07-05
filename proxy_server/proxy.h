#ifndef PROXY_H
#define PROXY_H

#include "epoll_wrap.h"
#include "socket_wrap.h"
#include "client.h"
#include "server.h"
#include <cstdint>
#include <map>
#include <memory>
#include <vector>
#include <thread>
#include <condition_variable>
#include <queue>

struct proxy;

struct resolver
{
	resolver(size_t cnt, proxy& proxy_server);
	
	~resolver();
	
	void push_request(std::unique_ptr<http_request> request);

	void stop();	
	
private:
	proxy* proxy_server;
	std::vector<std::thread> threads;
	std::mutex mute;
	std::condition_variable cond_var;
	std::queue<std::unique_ptr<http_request>> requests;
	
	void resolve();
};
struct client;
struct server;

struct proxy
{
	friend struct client;
	friend struct server;
	friend struct resolver;
	
	proxy(int port);
	
	~proxy();
	
	void start();
	
	void handler(std::unique_ptr<http_request> request);
	
	epoll_wrap& get_epoll()
	{
		return epoll;
	}
	
private:
	epoll_wrap epoll;
	socket_wrap socket;
	resolver* reslver;
	std::map<int, std::unique_ptr<client>> clients;
	std::map<int, server*> servers;
	task_wrap* listener;
};

#endif //PROXY_H
