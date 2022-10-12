#include "capturecxx.h"

#include <dirent.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#include <fstream>

#include "cap_ifaddrs.h"
#include "cap_inode_helper.h"
#include "cap_packet.h"
#include "export/capture_errors.h"

namespace Capture {
int32_t CaptureCxx::Init(const CaptureInitt *config,
                         ProcessInfoCallback ipcocessCallback /*= nullptr*/,
                         PacketCallback ipcaketCallback /*= nullptr*/) {
  processCallback = ipcocessCallback;
  packetCallback = ipcaketCallback;
  Singleton<Logger>::Get().Init(config->logfile);
  Singleton<Logger>::Get().SetLevel(spdlog::level::err);
  LOG_DEBUG("init capture begin");
  InitDevice();
  GetMemTotal();
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
    ptr->Parse(memory, deltaCpu);
    processes[pid] = std::move(ptr);
  }
  return 0;
}

int32_t CaptureCxx::Update(int32_t cnt, bool icaptureAll /* = falase*/) {
  // 对于映射以及进程数据进行更新 四元组->inode->pid的更新
  packetIdx = 0;
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
  int tmp = GetCpuTotal();
  if (tmp != 0) {
    return tmp;
  }

  if (pids.size() != processes.size()) {
    // 因为两个都按pid排序了 所以找不一样的 然后回收 一般来讲
    // 删除的都是processes里面的 直接触发一下删除回调 即回调行为为remove
    auto pidIt = pids.begin();
    auto processesIt = processes.begin();

    // 使用迭代器删除 注意迭代器失效问题
    for (; pidIt != pids.end() && processesIt != processes.end();) {
      if (*pidIt != processesIt->first) {
        // processes 删除
        if (processCallback != nullptr) {
          Process_t data;
          memset(&data, 0, sizeof(Process_t));
          uint32_t size = 1;
          data.pid = processesIt->first;
          processCallback(ACTION_REMOVE, &data, &size);
        }
        processesIt = processes.erase(processesIt);
      } else {
        processesIt->second->Parse(memory, deltaCpu);
        pidIt++;
        processesIt++;
      }
    }
  }

  if (processCallback != nullptr) {
    // 进行数据回调 回调到上层 通知数据
    // 如果为了内存 可以保留 只改变里面的内容值 就不用频繁的clear了

    // 说明少了 要进行插入新的
    uint32_t idx = 0;
    auto it = processes.begin();
    for (; it != processes.end() && idx < processData.size(); it++, idx++) {
      // 数据的赋值
      memset(processData.data() + idx, 0, sizeof(Process_t));

      strncpy(processData[idx].name, it->second->GetName().c_str(),
              CAPTURE_COMMAN_SIZE - 1);
      strncpy(processData[idx].cmdline, it->second->GetCmdline().c_str(),
              CAPTURE_CMDLINE_SIZE - 1);
      strncpy(processData[idx].user, it->second->GetUser().c_str(),
              CAPTURE_COMMAN_SIZE - 1);
      strncpy(processData[idx].group, it->second->GetGroup().c_str(),
              CAPTURE_COMMAN_SIZE - 1);

      processData[idx].pid = it->second->GetPid();
      processData[idx].uid = it->second->GetUid();
      processData[idx].gid = it->second->GetGid();
      processData[idx].fdCnt = it->second->GetFdCnt();

      processData[idx].memory = it->second->GetMemory();
      processData[idx].cpuPercent = it->second->GetCpuPercent();
      processData[idx].memPercent = it->second->GetMemPercent();
      processData[idx].ioRead = it->second->GetIoRead();

      processData[idx].ioWrite = it->second->GetIoWrite();
      processData[idx].recv = it->second->GetRecv();
      processData[idx].send = it->second->GetSend();
    }
    if (processData.size() < processes.size()) {
      for (; it != processes.end(); it++) {
        Process_t data;
        memset(&data, 0, sizeof(Process_t));

        // 数据的赋值
        strncpy(data.name, it->second->GetName().c_str(),
                CAPTURE_COMMAN_SIZE - 1);
        strncpy(data.cmdline, it->second->GetCmdline().c_str(),
                CAPTURE_CMDLINE_SIZE - 1);
        strncpy(data.user, it->second->GetUser().c_str(),
                CAPTURE_COMMAN_SIZE - 1);
        strncpy(data.group, it->second->GetGroup().c_str(),
                CAPTURE_COMMAN_SIZE - 1);

        data.pid = it->second->GetPid();
        data.uid = it->second->GetUid();
        data.gid = it->second->GetGid();
        data.fdCnt = it->second->GetFdCnt();

        data.memory = it->second->GetMemory();
        data.cpuPercent = it->second->GetCpuPercent();
        data.memPercent = it->second->GetMemPercent();
        data.ioRead = it->second->GetIoRead();

        data.ioWrite = it->second->GetIoWrite();
        data.recv = it->second->GetRecv();
        data.send = it->second->GetSend();
        processData.push_back(data);
      }
    }

    uint32_t size = processes.size();
    processCallback(ACTION_UPDATE, processData.data(), &size);
  }

  if (packetCallback != nullptr && ret > 0 && !packetData.empty()) {
    uint32_t size = ret;
    packetCallback(packetData.data(), &size);
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

int32_t CaptureCxx::GetCpuTotal() {
  constexpr uint32_t size = 1024;
  char buffer[size] = {0};
  std::fstream s("/proc/stat", s.in);
  if (!s.is_open()) {
    LOG_DEBUG("open file error:/proc/stat");
    return CAP_PROCESS_OPEN;
  } else {
    // parse
    while (!s.eof()) {
      /* code */
      s.getline(buffer, size);
      if (strcmp(buffer, "") == 0) {
        continue;
      }
      LOG_DEBUG("get buffer:{}", buffer);

      uint64_t utime = 0;
      uint64_t ntime = 0;
      uint64_t stime = 0;
      uint64_t itime = 0;
      uint64_t iowtime = 0;
      uint64_t irqtime = 0;
      uint64_t sirqtime = 0;
      uint64_t now = 0;

      if (sscanf(buffer, "cpu  %lu %lu %lu %lu %lu %lu %lu", &utime, &ntime,
                 &stime, &itime, &iowtime, &irqtime, &sirqtime)) {
        now = utime + ntime + stime + itime + iowtime + irqtime + sirqtime;
        deltaCpu = now - cpu;
        cpu = now;
        break;
      }
    }
  }
  return 0;
}

int32_t CaptureCxx::GetMemTotal() {
  constexpr uint32_t size = 1024;
  char buffer[size] = {0};
  std::fstream s("/proc/meminfo", s.in);
  if (!s.is_open()) {
    LOG_DEBUG("open file error:/proc/meminfo");
    return CAP_PROCESS_OPEN;
  } else {
    // parse
    while (!s.eof()) {
      /* code */
      s.getline(buffer, size);
      if (strcmp(buffer, "") == 0) {
        continue;
      }
      LOG_DEBUG("get buffer:{}", buffer);
      uint64_t tmpMem = 0;
      if (sscanf(buffer, "MemTotal:        %" PRId64 " kB", &tmpMem)) {
        LOG_INFO("init memory:{} buffer:{}", tmpMem, buffer);
        memory = tmpMem;
        break;
      }
    }
  }
  return 0;
}

int32_t CaptureCxx::ProcessTcp(const PcapCtx_t &context,
                               const struct pcap_pkthdr *header,
                               const u_char *packet) {
  // 进行数据解析 以及ip四元组 到到inode映射， inode到pid pid->process映射
  // 如果pid没有找到 则进行创造
  // new一个packet对象 然后加入进去
  // TODO 做成对象池
  LOG_DEBUG("process tcp");
  // 对packet数据进行处理
  struct tcphdr *tcp = (struct tcphdr *)packet;
  std::unique_ptr<CapPacket> cap = nullptr;
  switch (context.family) {
    case AF_INET:
      cap = std::make_unique<CapPacket>();
      cap->Init(context.addr.ip4.ip_src, ntohs(tcp->source),
                context.addr.ip4.ip_dst, ntohs(tcp->dest), header->len,
                header->ts);
      break;
    case AF_INET6:
      cap = std::make_unique<CapPacket>();
      cap->Init(context.addr.ip6.ip6_src, ntohs(tcp->source),
                context.addr.ip6.ip6_dst, ntohs(tcp->dest), header->len,
                header->ts);
      break;
    default:
      LOG_ERROR("unknow ip protocol", context.family);
      return CAP_NET_UNKNOW;
  }

  pid_t pid = -1;
  int32_t ret = Singleton<CapInodeHelper>::Get().GetPidByIp(cap->Hash(), pid);
  if (ret != 0) {
    return ret;
  }

  // 查找process 如果找不到则进行加入
  // 将packet加入链接里面
  auto processIt = processes.find(pid);
  if (processIt == processes.end()) {
    // 说明是新加入的 进行更新
    std::unique_ptr<CapProcess> ptr = std::make_unique<CapProcess>(pid);
    ptr->Parse(memory, deltaCpu);
    ptr->AddPacket(cap);
    processes[pid] = std::move(ptr);
  } else {
    processIt->second->AddPacket(cap);
  }

  if (packetCallback != nullptr) {
    if (packetIdx < packetData.size()) {
      if (packetData[packetIdx].packet != nullptr) {
        free(packetData[packetIdx].packet);
      }
      memset(packetData.data() + packetIdx, 0, sizeof(Packet_t));

      packetData[packetIdx].family = P_TCP;
      strncpy(packetData[packetIdx].hash, cap->Link().c_str(),
              CAPTURE_COMMAN_SIZE - 1);
      packetData[packetIdx].packetSize = header->len;
      if (captureAll) {
        // 进行包的复制
        u_char *bag = (u_char *)malloc(header->len);
        memcpy(bag, packet, header->len);
        packetData[packetIdx].packet = bag;
      }
      packetIdx++;
    } else {
      Packet_t temp;
      memset(&temp, 0, sizeof(Packet_t));

      temp.family = P_TCP;
      strncpy(temp.hash, cap->Link().c_str(), CAPTURE_COMMAN_SIZE - 1);
      temp.packetSize = header->len;
      if (captureAll) {
        // 进行包的复制
        u_char *bag = (u_char *)malloc(header->len);
        memcpy(bag, packet, header->len);
        temp.packet = bag;
      }

      packetData.push_back(temp);
      packetIdx++;
    }
  }

  return 0;
}

int32_t CaptureCxx::ProcessUdp(const PcapCtx_t &context,
                               const struct pcap_pkthdr *header,
                               const u_char *packet) {
  LOG_DEBUG("process udp");
  // 进行数据解析 以及ip四元组 到到inode映射， inode到pid pid->process映射
  // 如果pid没有找到 则进行创造
  // new一个packet对象 然后加入进去
  // TODO 做成对象池
  // 对packet数据进行处理
  struct udphdr *udp = (struct udphdr *)packet;
  std::unique_ptr<CapPacket> cap = nullptr;
  switch (context.family) {
    case AF_INET:
      cap = std::make_unique<CapPacket>();
      cap->Init(context.addr.ip4.ip_src, ntohs(udp->source),
                context.addr.ip4.ip_dst, ntohs(udp->dest), header->len,
                header->ts);
      break;
    case AF_INET6:
      cap = std::make_unique<CapPacket>();
      cap->Init(context.addr.ip6.ip6_src, ntohs(udp->source),
                context.addr.ip6.ip6_dst, ntohs(udp->dest), header->len,
                header->ts);
      break;
    default:
      LOG_ERROR("unknow ip protocol", context.family);
      return CAP_NET_UNKNOW;
  }

  pid_t pid = -1;
  int32_t ret = Singleton<CapInodeHelper>::Get().GetPidByIp(cap->Hash(), pid);
  if (ret != 0) {
    return ret;
  }

  // 查找process 如果找不到则进行加入
  // 将packet加入链接里面
  auto processIt = processes.find(pid);
  if (processIt == processes.end()) {
    // 说明是新加入的 进行更新
    std::unique_ptr<CapProcess> ptr = std::make_unique<CapProcess>(pid);
    ptr->Parse(memory, deltaCpu);
    ptr->AddPacket(cap);
    processes[pid] = std::move(ptr);
  } else {
    processIt->second->AddPacket(cap);
  }

  if (packetCallback != nullptr) {
    if (packetIdx < packetData.size()) {
      if (packetData[packetIdx].packet != nullptr) {
        free(packetData[packetIdx].packet);
      }
      memset(packetData.data() + packetIdx, 0, sizeof(Packet_t));

      packetData[packetIdx].family = P_TCP;
      strncpy(packetData[packetIdx].hash, cap->Link().c_str(),
              CAPTURE_COMMAN_SIZE - 1);
      packetData[packetIdx].packetSize = header->len;
      if (captureAll) {
        // 进行包的复制
        u_char *bag = (u_char *)malloc(header->len);
        memcpy(bag, packet, header->len);
        packetData[packetIdx].packet = bag;
      }
      packetIdx++;
    } else {
      Packet_t temp;
      memset(&temp, 0, sizeof(Packet_t));

      temp.family = P_UDP;
      strncpy(temp.hash, cap->Link().c_str(), CAPTURE_COMMAN_SIZE - 1);
      temp.packetSize = header->len;
      if (captureAll) {
        // 进行包的复制
        u_char *bag = (u_char *)malloc(header->len);
        memcpy(bag, packet, header->len);
        temp.packet = bag;
      }

      packetData.push_back(temp);
      packetIdx++;
    }
  }

  return 0;
}
}  // namespace Capture