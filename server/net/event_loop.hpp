#pragma once

#if defined(_WIN32)
#include <winsock2.h>
#include <cstdint>
#include <map>

struct epoll_data_t {
    int fd;
};

struct epoll_event {
    std::uint32_t events;
    epoll_data_t data;
};

#ifndef EPOLLIN
#define EPOLLIN 0x001
#endif
#ifndef EPOLLOUT
#define EPOLLOUT 0x004
#endif
#ifndef EPOLLERR
#define EPOLLERR 0x008
#endif
#ifndef EPOLLHUP
#define EPOLLHUP 0x010
#endif
#ifndef EPOLLRDHUP
#define EPOLLRDHUP 0x200
#endif

#else
#include <sys/epoll.h>
#endif

#include <string>
#include <vector>

#include "server/net/unique_descriptor.hpp"

namespace hypercom::server {

class event_loop {
public:
    event_loop();

    [[nodiscard]] bool open_loop(std::string &error_out);

    [[nodiscard]] bool watch_descriptor(int descriptor, bool watch_write,
                                        bool update_existing);

    void forget_descriptor(int descriptor);

    [[nodiscard]] int wait_for_events(int timeout_milliseconds,
                                      std::vector<epoll_event> &out);

private:
    unique_descriptor epoll_descriptor_;
#if defined(_WIN32)
    struct watched_socket {
        bool watch_write;
    };
    std::map<int, watched_socket> watched_;
#endif
};

} // namespace hypercom::server
