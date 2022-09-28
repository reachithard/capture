#ifndef _DEVICE_H_
#define _DEVICE_H_

#include <memory>
#include <string>

#include "ifaddrs.h"
#include "local_addr.h"

namespace Capture {
class Device {
 public:
  Device();
  ~Device();

  int Init(ifaddrs *addr);

  const char *GetName() const { return name.c_str(); }

 private:
  std::string name;

  unsigned int flags; /* Flags as from SIOCGIFFLAGS ioctl.  */

  /* Network address of this interface.  */
  std::unique_ptr<LocalAddr> localAddr;
};
}  // namespace Capture

#endif  // _DEVICE_H_
