#ifndef _LOCAL_ADDR_H_
#define _LOCAL_ADDR_H_

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cassert>
#include <cstdint>
#include <cstring>

namespace Capture {
class LocalAddr {
 public:
  LocalAddr() {}
  ~LocalAddr() {}

  int32_t Init(in_addr_t inAddrV4) {
    addrV4 = inAddrV4;
    family = AF_INET;
    inet_ntop(AF_INET, &inAddrV4, strV4, 15);
    return 0;
  }

  int32_t Init(struct in6_addr inAddrV6) {
    addrV6 = inAddrV6;
    family = AF_INET6;
    inet_ntop(AF_INET6, &inAddrV6, strV6, 63);
    return 0;
  }

  const char *GetStr() const {
    if (family == AF_INET) {
      return strV4;
    }
    return strV6;
  }

  bool Contains(const struct in6_addr &n_addr) {
    if (family == AF_INET6) {
      if (memcmp(&addrV6, &n_addr, sizeof(struct in6_addr)) == 0) {
        return true;
      }
    }
    return false;
  }

  bool Contains(const in_addr_t &n_addr) {
    if ((family == AF_INET) && (n_addr == addrV4)) {
      return true;
    }

    return false;
  }

 private:
  in_addr_t addrV4;
  struct in6_addr addrV6;
  int32_t family = 0;
  char strV4[16] = {0};
  char strV6[64] = {0};
};
}  // namespace Capture

#endif  // _LOCAL_ADDR_H_
