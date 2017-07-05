#ifndef CLIENT_H
#define CLIENT_H

#include "proxy.h"
#include "task_wrap.h"
#include "socket_wrap.h"
#include "server.h"
#include "http.h"
#include "timer_wrap.h"
#include <memory>
#include <string>

struct proxy;
struct server;

struct client
{
	friend struct server;
	friend struct proxy;
	friend struct timer_wrap<client>;
	
	client(int fd, proxy& proxy_server);
	
	void send_request(proxy& proxy_server);
	
	void receive_response();
	
	void match(server& serv);
	
	void send_to_server();
	
	void append(std::string& str);
	
	void erase(proxy& proxy_server);
	
	fd_wrap& get_fd()
	{
		return socket.get();
	}
	
private:
	std::string data;
	socket_wrap socket;
	task_wrap task;
	std::unique_ptr<server> task_server;
	std::unique_ptr<http_response> response;
    std::unique_ptr<http_request> request;
    timer_wrap<client> timer;
};

#endif //CLIENT_H
