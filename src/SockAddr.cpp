#include "SockAddr.hpp"

#include <regex>

static const std::string IPV4_REGEX = "([0-9]{1,3}\\.){3}[0-9]{1,3}";
static const std::string IPV6_REGEX =
    "((([a-fA-F0-9]{0,4}:){1,7}[a-fA-F0-9]{0,4})|(::[fF]{4}:" + IPV4_REGEX + "))";
static const std::string OPT_PORT = "(:\\d+)?";

static const std::regex ipv4("^" + IPV4_REGEX + OPT_PORT + "$");
static const std::regex ipv6("^(" + IPV6_REGEX + "|((\\[" + IPV6_REGEX + "\\])" + OPT_PORT + "))$");

SockAddr::SockAddr(const std::string &addr_string) {

    std::string host_part;
  
    std::smatch __m;
    if (std::regex_match(addr_string, __m, ipv6)) {
        protocol_raw() = AF_INET6;
    } else if (std::regex_match(addr_string, __m, ipv4)) {
        protocol_raw() = AF_INET;
    } else {
        throw IOException(EPROTONOSUPPORT);
    }

    const size_t n = addr_string.rfind(protocol() == AF_INET ? ":" : "]:");
    const size_t i = protocol() == AF_INET6 && n != std::string::npos ? 1 : 0;
    host_part = addr_string.substr(i, n-i);

    uint16_t port = 0;
    if (n != std::string::npos) {
        unsigned long temp = std::stoul(addr_string.substr(n + 1 + i));
        if (temp > 65535) {
            throw IOException(ERANGE);
        }
        port = (uint16_t)temp;
    }
    port_raw() = htons(port);

    int result;
    result = check(inet_pton(protocol(), host_part.c_str(), in_addr_raw()));

    check(result);
    if (result == 0) {
        throw IOException(EINVAL);
    }
}

SockAddr::SockAddr(sa_family_t protocol, uint16_t port) {
    if (protocol != AF_INET) {
        protocol = AF_INET6;
    }
    protocol_raw() = protocol;
    port_raw() = htons(port);
}

constexpr inline sockaddr_in *SockAddr::get_v4() { return (sockaddr_in *)_raw; }
constexpr inline sockaddr_in6 *SockAddr::get_v6() { return (sockaddr_in6 *)_raw; }

constexpr inline sa_family_t &SockAddr::protocol_raw() { return *(sa_family_t *)_raw; }
constexpr inline uint16_t &SockAddr::port_raw() {
    return protocol() == AF_INET ? get_v4()->sin_port : get_v6()->sin6_port;
}
constexpr inline void *SockAddr::in_addr_raw() {
    return protocol() == AF_INET ? (void *)&get_v4()->sin_addr : (void *)&get_v6()->sin6_addr;
};

constexpr sa_family_t SockAddr::protocol() const {
    return const_cast<SockAddr *>(this)->protocol_raw();
}

const sockaddr *SockAddr::raw() const { return const_cast<SockAddr *>(this)->raw_mut(); }

sockaddr *SockAddr::raw_mut() { return (sockaddr *)_raw; }

socklen_t SockAddr::size() const {
    if (protocol() == AF_INET) {
        return sizeof(sockaddr_in);
    } else {
        return sizeof(sockaddr_in6);
    }
}

const uint16_t SockAddr::port() const { return htons(const_cast<SockAddr *>(this)->port_raw()); }

const std::string SockAddr::ip_repr() const {
    char buffer[INET6_ADDRSTRLEN] = {0};
    inet_ntop(protocol(), const_cast<SockAddr *>(this)->in_addr_raw(), buffer, INET6_ADDRSTRLEN);
    return std::string(buffer);
}