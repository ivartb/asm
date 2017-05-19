#ifndef PROXY_HTTP_REQUEST_H
#define PROXY_HTTP_REQUEST_H

#include "http.h"

class http_request : public http_protocol {
public:

	http_request(std::string);

	std::string get_host();

	std::string get_port();

	sockaddr get_resolved_host();

	void set_resolved_host(sockaddr rh);

	std::string get_relative_URI();

	bool is_validating();

	int get_client_fd() {
		return client_fd;
	}

	void set_client_fd(int fd) {
		client_fd = fd;
	}

private:
	void parse_start_line(std::string) override;

	std::string get_start_line() override;

	std::string port = "", host = "", URI, method;
	sockaddr resolved_host;
	bool host_is_resolved = false;
	int client_fd;
};

#endif // PROXY_HTTP_REQUEST_H
