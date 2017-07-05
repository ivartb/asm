#include "server.h"
#include <sys/epoll.h>
#include <arpa/inet.h>
#include "utils.h"

server::server(proxy& proxy_server, sockaddr host, client& clt) :
		socket(::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)),
		task(socket.get(), proxy_server.epoll,	EPOLLOUT, 
		[this, &proxy_server](uint32_t events) 
		{try{
			if (events & EPOLLIN)
			{
				logger("Server in EPOLLIN");
				this->send_response();
			}
			else if (events & EPOLLOUT)
			{
				logger("Server in EPOLLOUT");
				this->receive_request();
			}
			else
			{
				erase(proxy_server);	
			}
		}catch (...) {
			erase(proxy_server);
		}
		}),  task_client(&clt), timer(*this, proxy_server, proxy_server.epoll)
{
	clt.match(*this);
	::connect(socket.get().get(), &host, sizeof(host));
}

void server::append(std::string& str)
{
	data.append(str);
}

void server::send_to_client()
{
	task_client->append(data);
	data.clear();
}

void server::receive_request()
{
	timer.settime();	
	size_t writ = socket.write(data);
	data.erase(0, writ);
	if (task_client != nullptr)
		task_client->send_to_server();
	if (data.size() == 0)
	{
		task.mod_task(EPOLLOUT, task_wrap::REMOVE);
		task.mod_task(EPOLLIN, task_wrap::INSERT);
	}
}

void server::send_response()
{
	timer.settime();
	std::string red = socket.read();
	if (red.length() == 0)
	{
		task.mod_task(EPOLLIN, task_wrap::REMOVE);
		return;
	}
	data.append(red);
	
	http_response *cur_response = task_client->response.get();
	cur_response->append(red);

	if (cur_response->finished()) 
	{
		task_client->append(data);
		data.clear();
		task_client->task.mod_task(EPOLLOUT, task_wrap::INSERT);	
	}
}

void server::erase(proxy& proxy_server)
{
	if (task_client != nullptr)
		task_client->task_server.reset(nullptr);	
	proxy_server.servers.erase(socket.get().get());
}
