#ifndef _CAP_IFADDRS_H_
#define _CAP_IFADDRS_H_

#include <ifaddrs.h>
#include <net/if.h>

namespace Capture {
class CapIfaddrs {
 public:
  CapIfaddrs() : addr(nullptr) {}
  ~CapIfaddrs();

  int32_t Init();

  struct ifaddrs* addr = nullptr;
};
}  // namespace Capture

#endif  // _CAP_IFADDRS_H_