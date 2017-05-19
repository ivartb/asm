#include "server.h"
#include "utils.h"

#include <cassert>

server::server(struct sockaddr addr, proxy_server &proxyServer, client &cl) : 
	socket(socket_wrapper(::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0))),
	event(event_wrapper(proxyServer.get_epoll(), socket.get_fd(), EPOLLOUT, 
		[this, &proxyServer](uint32_t events)mutable throw(std::runtime_error) {
			try {
				log_msg("Processing " + get_host() + " at fd " + std::to_string(get_fd().get_fd()));
				if (events & EPOLLIN) {
					log_msg("Server is EPOLLIN");
					read_response(proxyServer);
				}
				if (events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
					log_msg("Disconnecting " + std::to_string(get_fd().get_fd()));
					disconnect(proxyServer);
					return;
				}
				if (events & EPOLLOUT) {
					log_msg("Server is EPOLLOUT");
					write_request(proxyServer);
				}
			}
			catch (std::runtime_error &e) {
				log_msg("Error occured.\nDisconnecting from server ...");
				disconnect(proxyServer);
			}
		})), 
	time(proxyServer.get_epoll(), *this, proxyServer)
{
	if (connect(socket.get_fd().get_fd(), &addr, sizeof(addr)) == -1) {
		if (errno != EINPROGRESS) {
			perror("Error while connecting to server occurred");
		}
	}
}

fd_wrapper &server::get_fd() {
	return socket.get_fd();
}

fd_wrapper &server::get_client_fd() {
	assert(paired_client);
	return paired_client->get_fd();
}

void server::set_host(const std::string &host) {
	this->host = host;
}

std::string server::get_host() {
	return host;
}


void server::append(std::string &data) {
	buffer.append(data);
}

std::string &server::get_buffer() {
	return buffer;
}

size_t server::get_buffer_size() {
	return buffer.size();
}

size_t server::write() {
	try {
		size_t written_cnt = socket.write(buffer);
		buffer.erase(0, written_cnt);
		if (paired_client) {
			this->push_to_server();
		}
		return written_cnt;
	}
	catch (...) {
		return 0;
	}
}

std::string server::read() {
	assert(paired_client);
	log_msg("Paired client " + std::to_string(paired_client->get_fd().get_fd()));
	try {
		std::string data = socket.read(socket.get_available_bytes());
		buffer.append(data);
		return data;
	}
	catch (...) {
		return "";
	}
}

void server::push_to_server() {
	assert(paired_client);
	paired_client->flush_client_buffer();
}


void server::push_to_client() {
	assert(paired_client);
	paired_client->get_buffer().append(buffer);
	buffer.clear();

}


server::~server() {
	log_msg("Server destroyed");
	//proxyServer.erase_server( get_fd().get_fd());
}

void server::disconnect(proxy_server &proxyServer) {
	//std::cerr << "Disconnect server " << get_fd().get_fd() << get_host() << "\n";
	int sfd = get_fd().get_fd();
	proxyServer.erase_server(sfd);
	if (paired_client->request_server.get() == this) {
		paired_client->unbind();
	}
}

void server::read_response(proxy_server &proxyServer) {
	log_msg("Reading data from server " + std::to_string(get_fd().get_fd()));
	if (socket.get_available_bytes() == 0) {
		event.remove_flag(EPOLLIN);
		return;
	}

	time.reset();

	std::string data = read();

	http_response *cur_response = paired_client->get_response();

	cur_response->append(data);

	//std::cerr << "State: " << cur_response->get_stat() << "\n";


	if (cur_response->get_stat() == http_response::BAD) {
		std::cout << "Bad response " << get_host() << "\n";
		std::cout << "Response============================================\n\n";
		std::cout << cur_response->get_data();
		std::cout << "====================================================\n\n";
		buffer = BAD_REQUEST;
		push_to_client();
		paired_client->event.add_flag(EPOLLOUT);
		return;
	}
	///std::cerr<<"Response:\n";
//
	
	if (cur_response->is_ended()) {
		std::string cache_key = paired_client->get_request()->get_host() + paired_client->get_request()->get_relative_URI();
		// check cache hit
		std::cerr << cur_response->get_status() << '\n';
		if (cur_response->get_status() == "400") {
			std::cout << "Request============================================\n\n";
			std::cout << request << "\n\n";
			std::cout << "===================================================\n\n";
		}
		if (cur_response->get_status() == "304" && proxyServer.get_cache().contains(cache_key)) {
			log_msg("Cache hit for URI " + cache_key);

			http_response cached_response = proxyServer.get_cache().get(cache_key);
			get_buffer() = cached_response.get_data();
		}

		// try cache
		if (cur_response->is_cacheable() && !proxyServer.get_cache().contains(cache_key)) {
			proxyServer.get_cache().put(cache_key, *cur_response);
			log_msg("Response added to cache for " + cache_key);
		}

		push_to_client();
		paired_client->event.add_flag(EPOLLOUT);
		log_msg("Client " + std::to_string(get_client_fd().get_fd()) + " must write");
	}
}

void server::write_request(proxy_server &proxyServer) {
	log_msg("Writing data to server " + std::to_string(get_fd().get_fd()));
	time.reset();

	int error;
	socklen_t length = sizeof(error);
	if (getsockopt(get_fd().get_fd(), SOL_SOCKET, SO_ERROR, &error, &length) == -1 || error != 0) {
		perror("Error while connecting to server. Disconnecting...");
		disconnect(proxyServer);
		return;
	}
	request = buffer;
	write();
	if (get_buffer_size() == 0) {
		event.add_flag(EPOLLIN);
		event.remove_flag(EPOLLOUT);
		//paired_client->event.add_flag(EPOLLOUT);
	}
}


void server::add_flag(uint32_t flag) {
	event.add_flag(flag);
}

void server::bind(client &new_client) {
	paired_client = &new_client;
}

void server::unbind() {
	paired_client = nullptr;
}

