#include "http_response.h"
#include "utils.h"
#include <string>
#include <algorithm>

http_response::http_response() : http_protocol("") {}

http_response::http_response(std::string data) : http_protocol(data) {
	parse_data();
}

std::string http_response::get_start_line() {
	return protocol + " " + status;
}

void http_response::parse_start_line(std::string start_line) {
	auto first_space = std::find_if(start_line.begin(), start_line.end(), [](char a) { return a == ' '; });
	auto second_space = std::find_if(first_space + 1, start_line.end(), [](char a) { return a == ' '; });
	protocol = { start_line.begin(), first_space };
	status = { first_space + 1, second_space };
	if (status <= "100" || status >= "999" || (protocol != "HTTP/1.0" && protocol != "HTTP/1.1")) {
		data = BAD_GETAWAY;
		parse_data();
		incorrect = true;
		return;
	}
	bool is_valid_status = status >= "100" && status <= "999";
	bool is_valid_protocol = protocol == "HTTP/1.0" || protocol == "HTTP/1.1";

	state = (!is_valid_status || !is_valid_protocol) ? BAD : START_LINE;
}

bool http_response::check_cache_control() {
	auto hdr = get_header("cache-control");
	if (hdr == "") {
		return true;
	}
	return hdr.find("private") == std::string::npos
		&& hdr.find("no-cache") == std::string::npos
		&& hdr.find("no-store") == std::string::npos;
}

bool http_response::is_cacheable() {
	return state == FULL && status == "200" && check_cache_control() && get_header("ETag") != "" && get_header("Vary") == "";
}