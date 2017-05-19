#ifndef PROXY_HTTP_RESPONSE_H
#define PROXY_HTTP_RESPONSE_H

#include "http.h"

class http_response : public http_protocol {
public:
	http_response();

	http_response(std::string);

	bool check_cache_control();

	bool is_cacheable();

	std::string get_status() {
		return status;
	}

private:
	void parse_start_line(std::string) override;

	std::string get_start_line() override;

	std::string status;
};

#endif // PROXY_HTTP_RESPONSE_H