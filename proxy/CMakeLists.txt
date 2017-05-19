cmake_minimum_required(VERSION 2.8)
project(proxy)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Werror=pedantic -O2 -pthread")

set(SOURCE_FILES main.cpp
        fd_wrapper.cpp fd_wrapper.h
        socket_wrapper.cpp socket_wrapper.h
        utils.h
        proxy_server.cpp proxy_server.h
        epoll_wrapper.cpp epoll_wrapper.h
        event_wrapper.cpp event_wrapper.h
        http.h http.cpp
	http_request.h http_request.cpp
	http_response.h http_response.cpp
        lru_cache.h
        resolver.cpp resolver.h
        client.h client.cpp
        server.h server.cpp
        timer.h)
add_executable(proxy ${SOURCE_FILES})
