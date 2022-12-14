#ifndef _CAP_DEVICE_H_
#define _CAP_DEVICE_H_

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cstdint>
#include <memory>
#include <string>

#include "cap_ifaddrs.h"
#include "local_addr.h"

namespace Capture {
class CapDevice {
 public:
  CapDevice();
  ~CapDevice();

  int32_t Init(ifaddrs *addr);

  const char *GetName() const { return name.c_str(); }

  bool UpRunning();

  std::string GetLocalAddr() {
    if (localAddr) {
      return localAddr->GetStr();
    }
    return "";
  }

  bool Contains(const struct in6_addr &n_addr) {
    return localAddr->Contains(n_addr);
  }

  bool Contains(const in_addr_t &n_addr) { return localAddr->Contains(n_addr); }

 private:
  std::string name;

  uint32_t flags; /* Flags as from SIOCGIFFLAGS ioctl.  */

  /* Network address of this interface.  */
  std::unique_ptr<LocalAddr> localAddr;
};
}  // namespace Capture

#endif  // _CAP_DEVICE_H_
