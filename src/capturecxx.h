#ifndef _CAPTURECXX_H_
#define _CAPTURECXX_H_

#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include "cap_device.h"
#include "cap_handle.h"
#include "cap_process.h"
#include "cap_protocol.h"
#include "export/capture_pod.h"
#include "utils/logger.hpp"
#include "utils/singleton.h"

namespace Capture {
class CaptureCxx : public Singleton<CaptureCxx> {
 public:
  int32_t Init(const CaptureInitt *config);

  // -1表示无限捕获
  int32_t Update(int32_t cnt = -1);

  int32_t Shutdown();

  int32_t ProcessTcp(const PcapCtx_t &context, const struct pcap_pkthdr *header,
                     const u_char *packet);

  int32_t ProcessUdp(const PcapCtx_t &context, const struct pcap_pkthdr *header,
                     const u_char *packet);

  const std::vector<std::unique_ptr<CapDevice>> &GetDevices() const {
    return devices;
  }

 protected:
  void InitDevice();

  int32_t UpdatePids();

 private:
  std::vector<std::unique_ptr<CapDevice>> devices;
  std::vector<std::unique_ptr<CapHandle>> handles;
  std::unique_ptr<CapProtocol> protocol;
  std::set<pid_t> pids;
  std::map<pid_t, std::unique_ptr<CapProcess>> processes;
};
}  // namespace Capture
#endif  // _CAPTURECXX_H_