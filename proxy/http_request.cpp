#include "http_request.h"
#include "utils.h"
#include <string>
#include <algorithm>


http_request::http_request(std::string data) : http_protocol(data) {
	parse_data();
}

void http_request::parse_start_line(std::string start_line) {
	auto first_space = std::find_if(start_line.begin(), start_line.end(), [](char a) { return a == ' '; });
	auto second_space = std::find_if(first_space + 1, start_line.end(), [](char a) { return a == ' '; });
	auto crlf = std::find_if(second_space + 1, start_line.end(), [](char a) { return a == '\r'; });

	method = { start_line.begin(), first_space };
	URI = { first_space + 1, second_space };
	protocol = { second_space + 1, crlf };

	bool is_valid_method = method == "GET" || method == "POST" || method == "CONNECT";
	bool is_valid_URI = URI != "";
	bool is_valid_protocol = protocol == "HTTP/1.0" || protocol == "HTTP/1.1";

	state = (!is_valid_method || !is_valid_URI || !is_valid_protocol) ? BAD : START_LINE;
}

std::string http_request::get_start_line() {
	return method + " " + get_relative_URI() + " " + protocol;
}

std::string http_request::get_relative_URI() {
	std::string relative_URI = URI;
	if (relative_URI.substr(0, 7) == "http://") {
		relative_URI = relative_URI.substr(7);
	}
	if (relative_URI.substr(0, get_host().length()) == get_host()) {
		relative_URI = relative_URI.substr(get_host().length());
	}

	return relative_URI;
}

std::string http_request::get_port() {
	if (port.empty()) {
		get_host();
	}
	return port;
}

std::string http_request::get_host() {
	if (host == "") {
		if (get_header("host") == "") {
			host = port = "";
		}
		else {
			host = get_header("host");

			// "hostname.com:80" -> "hostname.com"
			port = (host.find(":") == std::string::npos) ? "80" : host.substr(host.find(":") + 1);
			host = (host.find(":") == std::string::npos) ? host : host.substr(0, host.find(":"));
		}
	}

	return host;
}

sockaddr http_request::get_resolved_host() {
	if (!host_is_resolved) {
		throw_my_error("Host not resolved");
	}
	return resolved_host;
}

void http_request::set_resolved_host(sockaddr rh) {
	host_is_resolved = true;
	resolved_host = rh;
}

bool http_request::is_validating() {
	return get_header("if-match") != ""
		|| get_header("if-modified-since") != ""
		|| get_header("if-none-match") != ""
		|| get_header("if-range") != ""
		|| get_header("if-unmodified-since") != "";
}