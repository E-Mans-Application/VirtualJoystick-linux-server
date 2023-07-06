#include "Socket.hpp"

#include <arpa/inet.h> //inet_addr
#include <sys/socket.h>
#include <unistd.h> //close
#include <fcntl.h>

Socket::Socket() {
    _inner = check(::socket(AF_INET6, SOCK_STREAM | SOCK_NONBLOCK, 0));

    int value = 1;
    try_opt(::setsockopt(_inner, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value)), "set reuseaddr");

    value = 0;
    try_opt(::setsockopt(_inner, IPPROTO_IPV6, IPV6_V6ONLY, &value, sizeof(value)),
            "disable ipv6 only");
}

Socket::Socket(int fd) : _inner(fd) {
    int flags = fcntl(_inner, F_GETFL);
    try_opt(fcntl(_inner, F_SETFL, flags | O_NONBLOCK), "activate non-blocking mode");
}

void Socket::close() { ::close(_inner); }

void Socket::bind(const SockAddr &addr) { check(::bind(_inner, addr.raw(), addr.size())); }
void Socket::listen(int queue_size) { check(::listen(_inner, queue_size)); }
std::optional<SocketWithAddress> Socket::accept() {
    SockAddr addr(AF_INET6, 0);
    socklen_t socklen = addr.size();

    int client = ::accept4(_inner, addr.raw_mut(), &socklen, SOCK_NONBLOCK);
    if (client == -1 && errno == EWOULDBLOCK) {
        return std::optional<SocketWithAddress>();
    }
    check(client);
    return std::optional<SocketWithAddress>({client, addr});
}

bool Socket::connect(const SockAddr &addr) {
    int r = ::connect(_inner, addr.raw(), addr.size());
     if (r == -1 && (errno == EINPROGRESS || errno == EALREADY || errno == EWOULDBLOCK)) {
        return false;
    }
    check(r);
    return true;
}

void Socket::send(const std::string &data) {
    check(write(_inner, data.c_str(), data.size()));
}


size_t Socket::receive(char *buffer, int max_len) {  
    size_t r = check(read(_inner, buffer, max_len));
    if (r == 0) {
        throw IOException(ENODATA);
    }
    return r;
}