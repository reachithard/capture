#include "cap_ifaddrs.h"

namespace Capture {
CapIfaddrs::~CapIfaddrs() {
  if (addr != nullptr) {
    freeifaddrs(addr);
  }
}

int32_t CapIfaddrs::Init() { return getifaddrs(&addr); }
}  // namespace Capture