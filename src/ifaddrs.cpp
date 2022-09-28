#include "ifaddrs.h"

namespace Capture {
Ifaddrs::~Ifaddrs() {
  if (addr != nullptr) {
    freeifaddrs(addr);
  }
}
}  // namespace Capture