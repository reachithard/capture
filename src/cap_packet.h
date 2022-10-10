#ifndef _CAP_PACKET_H_
#define _CAP_PACKET_H_

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <string>

#include "capturecxx.h"

namespace Capture {
enum CapDir { Unknown, Incoming, Outgoing };
class CapPacket {
 public:
  CapPacket() {}
  ~CapPacket() {}

  int32_t Init(in_addr isip, unsigned short isport, in_addr idip,
               unsigned short idport, u_int32_t ilen, timeval itime,
               CapDir idir = Unknown) {
    sip = isip;
    sport = isport;
    dip = idip;
    dport = idport;
    len = ilen;
    time = itime;
    dir = idir;
    family = AF_INET;
  }

  int32_t Init(in6_addr isip, unsigned short isport, in6_addr idip,
               unsigned short idport, u_int32_t ilen, timeval itime,
               CapDir idir = Unknown) {
    sip6 = isip;
    sport = isport;
    dip6 = idip;
    dport = idport;
    len = ilen;
    time = itime;
    dir = idir;
    family = AF_INET6;
  }

  CapDir Invert() {
    if (dir == Incoming) {
      dir = Outgoing;
    } else if (dir == Outgoing) {
      dir = Incoming;
    }

    if (family == AF_INET) {
      std::swap(sip, dip);
      std::swap(sport, dport);
    } else if (family == AF_INET6) {
      std::swap(sip6, dip6);
      std::swap(sport, dport);
    }

    return dir;
  }

  bool Sameinaddr(in_addr one, in_addr other) {
    return one.s_addr == other.s_addr;
  }

  bool Samein6addr(in6_addr one, in6_addr other) {
    return std::equal(one.s6_addr, one.s6_addr + 16, other.s6_addr);
  }

  bool IsOutgoing() {
    switch (dir) {
      case Outgoing:
        return true;
      case Incoming:
        return false;
      case Unknown:
        bool islocal;
        if (family == AF_INET) {
          islocal = Singleton<CaptureCxx>::Get().Contains(sip.s_addr);
        } else {
          islocal = Singleton<CaptureCxx>::Get().Contains(sip6);
        }
        if (islocal) {
          dir = Outgoing;
          return true;
        } else {
          dir = Incoming;
          return false;
        }
    }
    return false;
  }

  const std::string &Hash() {
    if (!hashstring.empty()) {
      return hashstring;
    }

    char local[50] = {0};
    char remote[50] = {0};
    if (family == AF_INET) {
      inet_ntop(family, &sip, local, 49);
      inet_ntop(family, &dip, remote, 49);
    } else {
      inet_ntop(family, &sip6, local, 49);
      inet_ntop(family, &dip6, remote, 49);
    }
    if (IsOutgoing()) {
      hashstring = std::string(local) + ":" + std::to_string(sport) + "-" +
                   std::string(remote) + ":" + std::to_string(dport);
    } else {
      hashstring = std::string(remote) + ":" + std::to_string(dport) + "-" +
                   std::string(local) + ":" + std::to_string(sport);
    }
    return hashstring;
  }

 private:
  in6_addr sip6;
  in6_addr dip6;
  in_addr sip;
  in_addr dip;
  uint32_t sport;
  uint32_t dport;
  uint32_t len;
  timeval time;
  CapDir dir;
  int32_t family;
  std::string hashstring;
};
}  // namespace Capture
#endif  // _CAP_PACKET_H_