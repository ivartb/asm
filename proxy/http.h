#ifndef PROXY_HTTP_H
#define PROXY_HTTP_H

#include <stdio.h>
#include <string>
#include <netdb.h>
#include <unordered_map>

const std::string BAD_REQUEST = std::string("HTTP/1.1 400 Bad Request\r\n"\
	"Server: ivartb proxy-server\r\n"\
	"Content-Type: text/html; charset=utf-8\r\n"\
	"Content-Length: 163\r\n"\
	"Connection: close\r\n\r\n"\
	"<html>\r\n"\
	"<head><title>400 Bad Request</title></head>\r\n"\
	"<body bgcolor=\"white\">\r\n"\
	"<center><h1>400 Bad Request</h1></center>\r\n"\
	"<hr><center>proxy</center>\r\n"\
	"</body>\r\n"\
	"</html>");

const std::string BAD_GETAWAY = std::string("HTTP/1.1 502 Bad Getaway\r\n"\
	"Server: ivartb proxy-server\r\n"\
	"Content-Type: text/html; charset=utf-8\r\n"\
	"Content-Length: 163\r\n"\
	"Connection: close\r\n\r\n"\
	"<html>\r\n"\
	"<head><title>502 Bad Getaway</title></head>\r\n"\
	"<body bgcolor=\"white\">\r\n"\
	"<center><h1>502 Bad Getaway</h1></center>\r\n"\
	"<hr><center>proxy</center>\r\n"\
	"</body>\r\n"\
	"</html>");



class http_protocol {

public:

    enum state_t {
        FULL, BAD, START_LINE, HEADERS, EMPTY, PARTIAL
    };

	http_protocol();

    http_protocol(std::string);

    virtual ~http_protocol() {}

    std::string get_data();

    state_t get_stat() {
        return state;
    }

    void set_header(std::string key, std::string val);

    std::string get_header(std::string key);

    void append(std::string &);

    bool is_ended();

    std::string get(){
        return data;
    }

protected:
    void parse_data();

    virtual void parse_start_line(std::string) = 0;

    void parse_headers(std::string);

    void check_body();

    virtual std::string get_start_line() = 0;

    std::string get_headers();

    std::string get_body();


	std::string protocol, data;
	state_t state;
	bool incorrect = 0;
	size_t body_begin_pos;
    std::unordered_map<std::string, std::string> headers;
};

#endif // PROXY_HTTP_H
