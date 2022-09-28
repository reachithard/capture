#include "device.h"

#include <ifaddrs.h>
#include <net/if.h>
#include <sys/socket.h>

#include <cstring>

#include "capture_errors.h"

namespace Capture {
Device::Device() {}

Device::~Device() {}

int Device::Init(ifaddrs *addr) {
  name =
      addr->ifa_name == nullptr ? "unknow device" : std::string(addr->ifa_name);
  flags = addr->ifa_flags;
  localAddr = std::make_unique<LocalAddr>();

  short int family = addr->ifa_addr->sa_family;
  if (family == AF_INET) {
    struct sockaddr_in *tmp = (struct sockaddr_in *)addr->ifa_addr;
    localAddr->Init(tmp->sin_addr.s_addr);
  } else if (family == AF_INET6) {
    struct sockaddr_in6 *tmp = (struct sockaddr_in6 *)addr->ifa_addr;
    localAddr->Init(tmp->sin6_addr);
  }
  return 0;
}

}  // namespace Capture