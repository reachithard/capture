#ifndef _CAP_PROTOCOL_H_
#define _CAP_PROTOCOL_H_

#include "utils/nocopy.h"

namespace Capture {
class CapProtocol : public Nocopy {
 public:
  void Init();
};
}  // namespace Capture
#endif  // _CAP_PROTOCOL_H_