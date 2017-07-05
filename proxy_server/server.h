#ifndef SERVER_H
#define SERVER_H

#include "proxy.h"
#include "task_wrap.h"
#include "socket_wrap.h"
#include "client.h"
#include "http.h"
#include "timer_wrap.h"
#include <memory>
#include <string>

struct proxy;
struct client;

struct server
{
	friend struct proxy;
	friend struct client;
	friend struct timer_wrap<server>;
	
	server(proxy& proxy_server, sockaddr host, client& clt);
	
	void send_response();
	
	void receive_request();
	
	fd_wrap& get_fd()
	{
		return socket.get();
	}
	
	void append(std::string& str);
	
	void send_to_client();
	
	void erase(proxy& proxy_server);
	
private:
	std::string data, host;
	socket_wrap socket;
	task_wrap task;
	client* task_client;
	timer_wrap<server> timer;
};

#endif //SERVER_H
