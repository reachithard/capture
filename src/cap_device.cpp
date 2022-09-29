#include "cap_device.h"

#include <ifaddrs.h>
#include <net/if.h>
#include <sys/socket.h>

#include <cstring>

#include "cap_device.h"
#include "utils/logger.hpp"

namespace Capture {
CapDevice::CapDevice() {}

CapDevice::~CapDevice() {}

int32_t CapDevice::Init(ifaddrs *addr) {
  name =
      addr->ifa_name == nullptr ? "unknow device" : std::string(addr->ifa_name);
  flags = addr->ifa_flags;
  localAddr = std::make_unique<LocalAddr>();

  int32_t family = addr->ifa_addr->sa_family;
  if (family == AF_INET) {
    struct sockaddr_in *tmp = (struct sockaddr_in *)addr->ifa_addr;
    localAddr->Init(tmp->sin_addr.s_addr);
  } else if (family == AF_INET6) {
    struct sockaddr_in6 *tmp = (struct sockaddr_in6 *)addr->ifa_addr;
    localAddr->Init(tmp->sin6_addr);
  }
  LOG_DEBUG("get device name:{} family:{} ip:{}", name, family,
            localAddr->GetStr());
  return 0;
}

bool CapDevice::UpRunning() {
  return !(flags & IFF_LOOPBACK) && (flags & IFF_UP) && (flags & IFF_RUNNING);
}

}  // namespace Capture