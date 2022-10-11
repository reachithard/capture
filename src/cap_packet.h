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
               CapDir idir = Unknown);

  int32_t Init(in6_addr isip, unsigned short isport, in6_addr idip,
               unsigned short idport, u_int32_t ilen, timeval itime,
               CapDir idir = Unknown);

  CapDir Invert();

  bool Sameinaddr(in_addr one, in_addr other) {
    return one.s_addr == other.s_addr;
  }

  bool Samein6addr(in6_addr one, in6_addr other) {
    return std::equal(one.s6_addr, one.s6_addr + 16, other.s6_addr);
  }

  bool IsOutgoing();

  const std::string &Hash();

  const std::string Link();

  uint32_t GetLen() const { return len; }

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