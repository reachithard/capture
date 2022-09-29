#include "capturecxx.h"

#include "cap_ifaddrs.h"
#include "export/capture_errors.h"

namespace Capture {
int32_t CaptureCxx::Init(const CaptureInitt *config) {
  Singleton<Logger>::Get().Init(config->logfile);
  LOG_DEBUG("init capture begin");
  InitDevice();
  if (devices.empty()) {
    return CAP_DEVICE_INIT;
  }

  int32_t ret = 0;
  CapHandleInit_t init;
  init.fliter = config->fliter;
  init.ms = config->ms;
  init.promise = config->promise;
  init.snaplen = config->snaplen;

  protocol = std::make_unique<CapProtocol>();
  for (uint32_t idx = 0; idx < devices.size(); idx++) {
    char buffer[CAP_ERRORBUFFER_SIZE] = {0};
    std::unique_ptr<CapHandle> handle = std::make_unique<CapHandle>();
    if (handle != nullptr) {
      // 进行callback插入
      init.device = devices[0]->GetName();
      init.errorbuf = buffer;
      if (handle->OpenLive(&init) != 0) {
        continue;
      }

      std::function<int32_t(u_char *, const struct pcap_pkthdr *,
                            const u_char *)>
          callback = [this](u_char *ctx, const struct pcap_pkthdr *head,
                            const u_char *packet) -> int32_t {
        protocol->Init();
        return 0;
      };

      handle->SetCallback(callback);
      ret = handle->SetNonBlock(1, buffer);
      if (ret != 0) {
        continue;
      }
      handles.push_back(std::move(handle));
    }
  }

  return 0;
}

int32_t CaptureCxx::Update(int32_t cnt) {
  int32_t ret = 0;
  for (uint32_t idx = 0; idx < handles.size(); idx++) {
    LOG_DEBUG("start dispatch");
    ret = handles[idx]->Dispatch(cnt, (u_char *)(handles[idx].get()));
    if (ret < 0) {
      LOG_ERROR("error dispatcher");
    } else if (ret != 0) {
      LOG_DEBUG("get data size:{}", ret);
    } else {
      LOG_DEBUG("get zero");
    }
  }
  return 0;
}

int32_t CaptureCxx::Shutdown() { return 0; }

void CaptureCxx::InitDevice() {
  std::unique_ptr<CapIfaddrs> ptr = std::make_unique<CapIfaddrs>();
  if (ptr->Init() != 0) {
    return;
  }
  struct ifaddrs *it;
  for (it = ptr->addr; it != nullptr; it = it->ifa_next) {
    if (it->ifa_addr == nullptr) {
      LOG_DEBUG("get a error device");
      continue;
    }
    std::unique_ptr<CapDevice> device = std::make_unique<CapDevice>();
    if (device->Init(it) != 0 || !device->UpRunning()) {
      LOG_DEBUG("get a error device");
      continue;
    }
    LOG_DEBUG("get a device:{}", device->GetName());
    devices.push_back(std::move(device));
  }
  return;
}
}  // namespace Capture