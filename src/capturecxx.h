#ifndef _CAPTURECXX_H_
#define _CAPTURECXX_H_

#include <cstdint>
#include <functional>
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
  int32_t Init(const CaptureInitt *config,
               ProcessInfoCallback ipcocessCallback = nullptr,
               PacketCallback ipcaketCallback = nullptr);

  // -1表示无限捕获
  int32_t Update(int32_t cnt = -1, bool icaptureAll = false);

  int32_t Shutdown();

  int32_t ProcessTcp(const PcapCtx_t &context, const struct pcap_pkthdr *header,
                     const u_char *packet);

  int32_t ProcessUdp(const PcapCtx_t &context, const struct pcap_pkthdr *header,
                     const u_char *packet);

  const std::vector<std::unique_ptr<CapDevice>> &GetDevices() const {
    return devices;
  }

  bool Contains(const struct in6_addr &n_addr) {
    for (uint32_t idx = 0; idx < devices.size(); idx++) {
      if (devices[idx].get()->Contains(n_addr)) {
        return true;
      }
    }
    return false;
  }

  bool Contains(const in_addr_t &n_addr) {
    for (uint32_t idx = 0; idx < devices.size(); idx++) {
      if (devices[idx].get()->Contains(n_addr)) {
        return true;
      }
    }
    return false;
  }

 protected:
  void InitDevice();

  int32_t UpdatePids();

  int32_t GetCpuTotal();

  int32_t GetMemTotal();

 private:
  std::vector<std::unique_ptr<CapDevice>> devices;
  std::vector<std::unique_ptr<CapHandle>> handles;
  std::unique_ptr<CapProtocol> protocol;
  std::set<pid_t> pids;
  std::map<pid_t, std::unique_ptr<CapProcess>> processes;
  std::function<void(CaptureAction action, const Process_t *processes,
                     uint32_t *size)>
      processCallback;

  std::function<void(const Packet_t *packets, uint32_t *size)> packetCallback;

  std::vector<Process_t> processData;
  std::vector<Packet_t> packetData;
  uint64_t packetIdx = 0;
  bool captureAll = false;

  uint64_t memory = 0;
  uint64_t cpu = 0;
  uint64_t deltaCpu = 0;
};
}  // namespace Capture
#endif  // _CAPTURECXX_H_