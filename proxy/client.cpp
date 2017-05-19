#include "client.h"
#include <cassert>

client::client(int fd, proxy_server &proxyServer) : 
	socket(fd),
	event(proxyServer.get_epoll(), get_fd(), EPOLLIN,
		[&proxyServer, this](uint32_t events) mutable throw(std::runtime_error) {
			try {

				if (events & EPOLLIN) {
					log_msg("Client is EPOLLIN");
					this->read_request(proxyServer);
				}
				if (events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
					disconnect(proxyServer);
					return;
				}
				if (events & EPOLLOUT) {
					log_msg("Client is EPOLLOUT");
					write_response(proxyServer);
				}

			}
			catch (...) {
				log_msg("Client error");
				disconnect(proxyServer);
			}
		}),
	time(proxyServer.get_epoll(), *this, proxyServer),
	request_server(nullptr)
{
}

client::~client() {
	log_msg("Client destroyed " + std::to_string(get_fd().get_fd()));
}

fd_wrapper &client::get_fd() {
	return socket.get_fd();
}

fd_wrapper &client::get_server_fd() {
	assert(request_server);
	return request_server->get_fd();
}

bool client::has_server() {
	return request_server != nullptr;
}

std::string &client::get_buffer() {
	return buffer;
}

size_t client::get_buffer_size() {
	return buffer.size();
}

std::string client::get_host() {
	assert(request_server);
	return request_server->get_host();
}

size_t client::read() {
	try {
		std::string reads = socket.read(socket.get_available_bytes());
		buffer.append(reads);
		return reads.length();
	}
	catch (...) {
		return 0;
	}
}

size_t client::write() {
	try {
		size_t written_cnt = socket.write(buffer);
		buffer.erase(0, written_cnt);
		if (request_server) {
			flush_server_buffer();
		}
		return written_cnt;
	}
	catch (...) {
		return 0;
	}
}

void client::bind(server &new_server) {
	request_server.reset(&new_server);
	request_server->bind(*this);
}

void client::unbind() {
	if (request_server != nullptr) {
		request_server.reset(nullptr);
	}
	//std::cerr << "Reset " << socket.get_fd().get_fd() << "\n";
}

void client::flush_client_buffer() {
	assert(request_server);
	request_server->append(buffer);
	buffer.clear();
}

void client::flush_server_buffer() {
	assert(request_server);
	request_server->push_to_client();
}

void client::set_response(class http_response *rsp) {
	response.reset(rsp);
}

http_response *client::get_response() {
	return response.get();
}

void client::set_request(class http_request *rsp) {
	request.reset(rsp);
}

http_request *client::get_request() {
	return request.get();
}

void client::read_request(proxy_server &proxyServer) {
	if (socket.get_available_bytes() == 0) {
		event.remove_flag(EPOLLIN);
		return;
	}
	time.reset();
	read();
	log_msg("Read data from client " + std::to_string(get_fd().get_fd()));


	std::unique_ptr<http_request> cur_request(new http_request(get_buffer()));

	if (cur_request->get_stat() == http_request::BAD) {
		std::cout << "Bad request\n";
		std::cout << "=====================================================\n\n";
		std::cout << buffer;
		std::cout << "=====================================================\n\n";
		buffer = BAD_REQUEST;
		event.remove_flag(EPOLLIN);
		event.add_flag(EPOLLOUT);
		return;
	}

	if (cur_request->is_ended()) {
		log_msg("Request for host " + cur_request->get_host());

		http_response *response = new http_response();
		set_response(response);

		std::string cache_key = cur_request->get_host() + cur_request->get_relative_URI();
		if (!cur_request->is_validating() && proxyServer.get_cache().contains(cache_key)) {
			http_response cached_response = proxyServer.get_cache().get(cache_key);
			std::string etag = cached_response.get_header("ETag");;
			cur_request->set_header("If-None-Match", etag);
		}

		set_request(new http_request(*cur_request.get()));

		if (has_server()) {
			if (get_host() == cur_request->get_host()) {
				buffer = cur_request->get_data();
				flush_client_buffer();
				request_server->add_flag(EPOLLOUT);
				return;
			}
			else {
				proxyServer.erase_server(get_server_fd().get_fd());
				unbind();
			}
		}

		buffer = cur_request->get_data();
		cur_request->set_client_fd(get_fd().get_fd());
		proxyServer.add_task(std::move(cur_request));
	}
}

void client::write_response(proxy_server &proxyServer) {
	log_msg("Writing data to client " + std::to_string(get_fd().get_fd()));
	time.reset();
	write();
	if (get_buffer_size() == 0) {
		event.remove_flag(EPOLLOUT);
		event.add_flag(EPOLLIN);
	}
}


void client::disconnect(proxy_server &proxyServer) {
	log_msg("Disconnect client " + std::to_string(get_fd().get_fd()));

	if (has_server()) {
		//log_msg("Disconnect server " + std::to_string(get_server_fd().get_fd()));
		request_server->unbind();
		unbind();
	}
	proxyServer.erase_client(get_fd().get_fd());
}
