#include "client.h"
#include <sys/epoll.h>
#include "utils.h"

client::client(int fd, proxy& proxy_server) : socket(fd), task(socket.get(), 
		proxy_server.epoll,	EPOLLIN, 
		[this, &proxy_server](uint32_t events) 
		{try{
			if (events & EPOLLIN)
			{
				logger("Client in EPOLLIN");
				this->send_request(proxy_server);
			}
			else if (events & EPOLLOUT)
			{
				logger("Client in EPOLLOUT");
				this->receive_response();
			}
			else
			{
				erase(proxy_server);
			}
		} catch (...) {
			erase(proxy_server);
		}}),  task_server(nullptr), timer(*this, proxy_server, proxy_server.epoll){}
		
void client::send_request(proxy& proxy_server)
{
	timer.settime();
	std::string red = socket.read();
	if (red.length() == 0)
	{
		task.mod_task(EPOLLIN, task_wrap::REMOVE);
		return;
	}
	data.append(red);
	
	std::unique_ptr<http_request> cur_request(new http_request(data));
	if (cur_request->finished()) {
		task.mod_task(EPOLLIN, task_wrap::REMOVE);
		logger("Request finished. Host = " + cur_request->get_host());

		http_response *response = new http_response();
		this->response.reset(response);
		this->request.reset(new http_request(*cur_request.get()));
		
		if (task_server != nullptr)
		 {
			if (task_server->host == cur_request->get_host()) 
			{
				data = cur_request->get_data();
				task_server->append(data);
				task_server->task.mod_task(EPOLLOUT, task_wrap::INSERT);
				return;
			}
			else 
			{
				proxy_server.servers.erase(task_server->get_fd().get());
				task_server.reset(nullptr);
			}
		}
		
		data = cur_request->get_data();
		cur_request->set_fd(socket.get().get());
		proxy_server.reslver->push_request(std::move(cur_request));
	}
}

void client::match(server& serv)
{
	task_server.reset(&serv);
}

void client::send_to_server()
{
	task_server->append(data);
	data.clear();
}

void client::append(std::string& str)
{
	data.append(str);
}

void client::receive_response()
{
	timer.settime();
	size_t writ = socket.write(data);
	data.erase(0, writ);
	if (task_server != nullptr)
		task_server->send_to_client();
	if (data.size() == 0)
	{
		task.mod_task(EPOLLOUT, task_wrap::REMOVE);
		task.mod_task(EPOLLIN, task_wrap::INSERT);
	}
}

void client::erase(proxy& proxy_server)
{
	if (task_server != nullptr)
	{
		task_server->task_client = nullptr;	
		task_server.reset(nullptr);
	}
	proxy_server.clients.erase(socket.get().get());	
}
