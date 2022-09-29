#ifndef _CAPTURECXX_H_
#define _CAPTURECXX_H_

#include <cstdint>
#include <memory>
#include <vector>

#include "cap_device.h"
#include "cap_handle.h"
#include "export/capture_pod.h"
#include "utils/logger.hpp"
#include "utils/singleton.h"

namespace Capture {
class CaptureCxx : public Singleton<CaptureCxx> {
 public:
  int32_t Init(const CaptureInitt *config);

  int32_t Update();

  int32_t Shutdown();

 protected:
  void InitDevice();

 private:
  std::vector<std::unique_ptr<CapDevice>> devices;
  std::vector<std::unique_ptr<CapHandle>> handles;
};
}  // namespace Capture
#endif  // _CAPTURECXX_H_