#include "http.h"
#include "utils.h"
#include <string>
#include <algorithm>

http_protocol::http_protocol() : state(EMPTY) {}

http_protocol::http_protocol(std::string content) : data(content), state(EMPTY) {}

void http_protocol::set_header(std::string key, std::string val) {
    log_msg("Key: " + key);
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
	log_msg("Key: " + key);
	headers[key] = val;
}


std::string http_protocol::get_header(std::string header) {
    return headers.find(header) != headers.end() ? headers[header] : "";
}

void http_protocol::parse_data() {
    auto start_line_end = data.begin();
    if (state == EMPTY && data.find("\r\n") != std::string::npos) {
        auto first_space = std::find_if(data.begin(), data.end(), [](char a) { return a == ' '; });
        auto second_space = std::find_if(first_space + 1, data.end(), [](char a) { return a == ' '; });
        start_line_end = std::find_if(second_space + 1, data.end(), [](char a) { return a == '\r'; });
        if (first_space == data.end() || second_space == data.end() || start_line_end == data.end()) {
            state = BAD;
        } else {
            parse_start_line({data.begin(), start_line_end});
        }
    }

    size_t start_line_end_pos = std::distance(data.begin(), start_line_end);
    size_t _body_begin_pos = data.find("\r\n\r\n", start_line_end_pos);
    if (state == START_LINE && _body_begin_pos != std::string::npos) {
        std::string text_headers = data.substr(start_line_end_pos + 2, _body_begin_pos - start_line_end_pos);
        parse_headers(text_headers);
    }

    if (state == HEADERS || state == PARTIAL) {
        body_begin_pos = _body_begin_pos + 4;
        check_body();
    }
}

void http_protocol::parse_headers(std::string text_headers) {
    if (text_headers == "") {
        log_msg("No headers found");
        state = BAD;
    } else {
        auto headers_it = text_headers.begin();

        while (headers_it != text_headers.end() && *headers_it != '\r') {
            auto space = std::find_if(headers_it, text_headers.end(), [](char a) { return a == ':'; });
            auto shift = 1;
            while (*(space + shift) == ' ')shift++;
            auto crlf = std::find_if(space + shift, text_headers.end(), [](char a) { return a == '\r'; });
            std::string cur_header = {headers_it, space};
            std::transform(cur_header.begin(), cur_header.end(), cur_header.begin(), ::tolower);
            headers.insert({cur_header, {space + shift, crlf}});
            headers_it = crlf + 2;
        };
        state = HEADERS;
    }
}

void http_protocol::check_body() {
    if (get_header("content-length") != "") {
        if (get_body().size() == static_cast<size_t>(std::stoi(get_header("content-length")))) {
            state = FULL;
        } else {
            state = PARTIAL;
        }
    } else if (get_header("transfer-encoding") == "chunked") {
        if (get_body().size() >= 5 && get_body().substr(get_body().size() - 5) == "0\r\n\r\n") {
            state = FULL;
        } else {
            state = PARTIAL;
        }
    } else {
        state = get_body().size() == 0 ? FULL : BAD;
    }
}

bool http_protocol::is_ended() {
    return state == FULL;
}

std::string http_protocol::get_data() {
    return get_start_line() + "\r\n" + get_headers() + "\r\n" + get_body();
}

std::string http_protocol::get_headers() {
    std::string all_headers = "";
    for (auto header : headers) {
        all_headers += header.first + ": " + header.second + "\r\n";
    }
    return all_headers;
}

std::string http_protocol::get_body() {
    return data.substr(body_begin_pos);
}

void http_protocol::append(std::string &content) {
    data.append(content);
    parse_data();
}
