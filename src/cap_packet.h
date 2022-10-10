#ifndef _CAP_PACKET_H_
#define _CAP_PACKET_H_

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <string>

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