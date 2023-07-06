#ifndef _SOCKADDR_HPP_
#define _SOCKADDR_HPP_

#include "Fallible.hpp"

#include <optional>
#include <arpa/inet.h> //inet_addr

class SockAddr: Fallible {
  private:
    char _raw[sizeof(sockaddr_in6)] = {0};

    constexpr inline sockaddr_in *get_v4();
    constexpr inline sockaddr_in6 *get_v6();

    constexpr inline sa_family_t &protocol_raw();
    constexpr inline uint16_t &port_raw();
    constexpr inline void *in_addr_raw();   

  public:
    SockAddr(const std::string &addr);
    SockAddr(sa_family_t protocol, uint16_t port);

    constexpr sa_family_t protocol() const;
    const sockaddr *raw() const;
    sockaddr *raw_mut();
    socklen_t size() const;

    const uint16_t port() const;
    const std::string ip_repr() const;

};

#endif