#include "capturecxx.h"

#include <dirent.h>

#include "cap_ifaddrs.h"
#include "cap_inode_helper.h"
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
  char buffer[CAP_ERRORBUFFER_SIZE] = {0};
  init.errorbuf = buffer;

  protocol = std::make_unique<CapProtocol>();
  for (uint32_t idx = 0; idx < devices.size(); idx++) {
    std::unique_ptr<CapHandle> handle = std::make_unique<CapHandle>();
    if (handle != nullptr) {
      // 进行callback插入
      init.device = devices[idx]->GetName();

      if (handle->OpenLive(&init) != 0) {
        continue;
      }

      std::function<int32_t(u_char *, const struct pcap_pkthdr *,
                            const u_char *)>
          callback = [this](u_char *ctx, const struct pcap_pkthdr *head,
                            const u_char *packet) -> int32_t {
        return protocol->Parse(ctx, head, packet);
      };

      handle->SetCallback(callback);
      ret = handle->SetNonBlock(1, buffer);
      if (ret != 0) {
        continue;
      }
      handles.push_back(std::move(handle));
    }
  }

  // 进行初始化
  UpdatePids();
  Singleton<CapInodeHelper>::Get().Refresh(pids);
  for (auto pid : pids) {
    std::unique_ptr<CapProcess> ptr = std::make_unique<CapProcess>(pid);
    ptr->Parse();
    processes[pid] = std::move(ptr);
  }
  return 0;
}

int32_t CaptureCxx::Update(int32_t cnt) {
  // 对于映射以及进程数据进行更新 四元组->inode->pid的更新
  UpdatePids();
  Singleton<CapInodeHelper>::Get().Refresh(pids);
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

  // 如果进程数据不一致 则说明有进程被杀了 所以没必要再进行监控了 直接删除
  if (pids.size() != processes.size()) {
    // 因为两个都按pid排序了 所以找不一样的 然后回收 一般来讲
    // 删除的都是processes里面的
  }

  // 进行数据回调 回调到上层 通知数据
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

int32_t CaptureCxx::UpdatePids() {
  pids.clear();
  DIR *proc = opendir("/proc");
  if (proc == 0) {
    LOG_ERROR("open dir /proc error");
    return CAP_OPEN_DIR;
  }
  dirent *entry;

  while ((entry = readdir(proc))) {
    if (entry->d_type != DT_DIR) {
      continue;
    }

    std::string name =
        (entry->d_name == nullptr) ? "" : std::string(entry->d_name);
    LOG_DEBUG("get name:{} {}", name, entry->d_name);
    if (!std::all_of(name.begin(), name.end(),
                     [](unsigned char c) -> bool { return std::isdigit(c); })) {
      continue;
    }

    pids.insert(std::stol(name));
  }
  closedir(proc);
  return 0;
}

int32_t CaptureCxx::ProcessTcp(const PcapCtx_t &context,
                               const struct pcap_pkthdr *header,
                               const u_char *packet) {
  // 进行数据解析 以及ip四元组 到到inode映射， inode到pid pid->process映射
  // 如果pid没有找到 则进行创造
  // new一个packet对象 然后加入进去
  LOG_DEBUG("process tcp");
  return 0;
}

int32_t CaptureCxx::ProcessUdp(const PcapCtx_t &context,
                               const struct pcap_pkthdr *header,
                               const u_char *packet) {
  LOG_DEBUG("process udp");
  return 0;
}
}  // namespace Capture