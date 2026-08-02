#include "server/net/event_loop.hpp"

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <cerrno>
#include <cstring>
#include <unistd.h>
#endif

namespace hypercom::server {
namespace {

constexpr std::size_t MAX_EVENTS_PER_WAIT = 256;

} // namespace

event_loop::event_loop() : epoll_descriptor_{} {}

bool event_loop::open_loop(std::string &error_out)
{
#if defined(_WIN32)
    (void)error_out;
    return true;
#else
    unique_descriptor created{::epoll_create1(EPOLL_CLOEXEC)};
    if (created.get_value() < 0) {
        error_out = std::string{"epoll_create1() : "} + std::strerror(errno);
        return false;
    }
    epoll_descriptor_ = std::move(created);
    return true;
#endif
}

bool event_loop::watch_descriptor(int descriptor, bool watch_write,
                                  bool update_existing)
{
#if defined(_WIN32)
    (void)update_existing;
    watched_[descriptor] = watched_socket{watch_write};
    return true;
#else
    epoll_event event{};
    event.events = EPOLLIN | EPOLLRDHUP;
    if (watch_write) {
        event.events |= EPOLLOUT;
    }
    event.data.fd = descriptor;
    int const operation = update_existing ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
    return ::epoll_ctl(epoll_descriptor_.get_value(), operation, descriptor,
                       &event)
        == 0;
#endif
}

void event_loop::forget_descriptor(int descriptor)
{
#if defined(_WIN32)
    watched_.erase(descriptor);
#else
    static_cast<void>(::epoll_ctl(epoll_descriptor_.get_value(),
                                  EPOLL_CTL_DEL, descriptor, nullptr));
#endif
}

int event_loop::wait_for_events(int timeout_milliseconds,
                                std::vector<epoll_event> &out)
{
    out.clear();
#if defined(_WIN32)
    if (watched_.empty()) {
        ::Sleep(static_cast<DWORD>(timeout_milliseconds > 0 ? timeout_milliseconds : 10));
        return 0;
    }
    std::vector<WSAPOLLFD> fds;
    fds.reserve(watched_.size());
    for (auto const &[fd, info] : watched_) {
        WSAPOLLFD pfd{};
        pfd.fd = static_cast<SOCKET>(fd);
        pfd.events = POLLRDNORM;
        if (info.watch_write) {
            pfd.events |= POLLWRNORM;
        }
        fds.push_back(pfd);
    }
    int const count = ::WSAPoll(fds.data(), static_cast<ULONG>(fds.size()), timeout_milliseconds);
    if (count <= 0) {
        return count;
    }
    for (WSAPOLLFD const &pfd : fds) {
        if (pfd.revents != 0) {
            epoll_event ev{};
            ev.data.fd = static_cast<int>(pfd.fd);
            ev.events = 0;
            if ((pfd.revents & (POLLRDNORM | POLLIN)) != 0) {
                ev.events |= EPOLLIN;
            }
            if ((pfd.revents & (POLLWRNORM | POLLOUT)) != 0) {
                ev.events |= EPOLLOUT;
            }
            if ((pfd.revents & POLLHUP) != 0) {
                ev.events |= EPOLLHUP;
            }
            if ((pfd.revents & POLLERR) != 0) {
                ev.events |= EPOLLERR;
            }
            out.push_back(ev);
        }
    }
    return static_cast<int>(out.size());
#else
    out.resize(MAX_EVENTS_PER_WAIT);
    int const count = ::epoll_wait(epoll_descriptor_.get_value(), out.data(),
                                   static_cast<int>(out.size()),
                                   timeout_milliseconds);
    if (count < 0) {
        out.clear();
        return errno == EINTR ? 0 : -1;
    }
    out.resize(static_cast<std::size_t>(count));
    return count;
#endif
}

} // namespace hypercom::server
