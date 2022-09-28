#ifndef _IFADDRS_H_
#define _IFADDRS_H_

#include <ifaddrs.h>
#include <net/if.h>

namespace Capture {
class Ifaddrs {
 public:
  Ifaddrs() : addr(nullptr) {}
  ~Ifaddrs();

  struct ifaddrs* addr = nullptr;
};
}  // namespace Capture

#endif  // _IFADDRS_H_
