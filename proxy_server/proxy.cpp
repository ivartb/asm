#include "proxy.h"
#include "utils.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <string.h>
#include <signal.h>

proxy::proxy(int port) : socket(::socket(AF_INET, SOCK_STREAM, 0))
{
	reslver = new resolver(8, *this);
	logger("Resolver created");
	const int set = 1;
    if (setsockopt(socket.get().get(), SOL_SOCKET, SO_REUSEPORT, &set, sizeof(set)) == -1)
        handle("Cannot create socket");
    socket.bind(port);
    socket.listen();
    socket.get().set_non_block();
    logger("Listen started");
    listener = new task_wrap(socket.get(), epoll, EPOLLIN,
			 [this](uint32_t)
			 {
			 	int client_fd = socket.accept();
			 	clients.insert(std::pair<int, std::unique_ptr<client>>
			 		(client_fd, std::unique_ptr<client>(new client(client_fd, *this))));
			 	logger("Added client at fd " + std::to_string(client_fd));
			 }); 
}

proxy::~proxy()
{
	reslver->~resolver();
}

void proxy::start()
{
	logger("Proxy started");
	epoll.start();
}

void proxy::handler(std::unique_ptr<http_request> cur_request)
{
	client* clt = clients[cur_request->get_fd()].get();
	if (clt == nullptr)
		return;
		
	struct sockaddr host = cur_request->get_ans_host();
	server* serv = new server(*this, host, *clt);
	
	serv->task.mod_task(EPOLLOUT, task_wrap::INSERT);
	servers.insert(std::pair<int, server*> (serv->get_fd().get(),serv));
	clt->timer.settime();
	clt->data = std::move(cur_request->get_data());
	logger("Sending data to server");
	clt->send_to_server();
}

resolver::resolver(size_t cnt, proxy& proxy_server) : proxy_server(&proxy_server)
{
	for (size_t i = 0; i != cnt; i++)
		threads.push_back(std::thread([this](){this->resolve();}));
}

resolver::~resolver()
{
	for (size_t i = 0; i != threads.size(); i++)
		if (threads[i].joinable())
			threads[i].join();
}

void resolver::push_request(std::unique_ptr<http_request> request)
{
	std::unique_lock<std::mutex> lock(mute);
	requests.push(std::move(request));
	cond_var.notify_one();
}

void resolver::resolve()
{
	while (true)
	{
		std::unique_lock<std::mutex> lock(mute);
		cond_var.wait(lock, [&]() {return !requests.empty();});
		
		logger("Host resolve started");
		auto request = std::move(requests.front());
		requests.pop();
		lock.unlock();
		
		sockaddr ans;
		struct addrinfo hints, *res;
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        if (getaddrinfo(request->get_host().c_str(), request->get_port().c_str(),
        				 &hints, &res) != 0)
        {
        	logger("Cannot resolve host: " + request->get_host());
        	proxy_server->clients.erase(request->get_fd());
        	continue;
        }
        else
        {
        	ans = *res->ai_addr;
        	freeaddrinfo(res);
        }

        request->set_ans_host(ans);
        logger("Host resolved: " + request->get_host());
        proxy_server->handler(std::move(request));
	}
}
