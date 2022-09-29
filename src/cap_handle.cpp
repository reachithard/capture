#include "cap_handle.h"

#include "export/capture_errors.h"
#include "utils/logger.hpp"

namespace Capture {
CapHandle::CapHandle() {}

CapHandle::~CapHandle() {
  if (handle != nullptr) {
    pcap_close(handle);
    handle = nullptr;
  }
}

int32_t CapHandle::OpenLive(const CapHandleInit_t* config) {
  struct bpf_program program;
  bpf_u_int32 mask;
  bpf_u_int32 ip;

  handle = pcap_open_live(config->device, config->snaplen, config->promise,
                          config->ms, config->errorbuf);
  if (handle == nullptr) {
    LOG_ERROR("open pcap error:{}", config->errorbuf);
    return CAP_OPEN_DEVICE;
  }

  if (config->fliter != nullptr) {
    pcap_lookupnet(config->device, &ip, &mask, config->errorbuf);

    if (pcap_compile(handle, &program, config->fliter, 1, ip) == -1) {
      LOG_ERROR("compile filter error on device:{} error:{}", config->device,
                pcap_geterr(handle));
      handle = nullptr;
      return CAP_COMPILE;
    }

    if (pcap_setfilter(handle, &program) == -1) {
      LOG_ERROR("set filter error on device:{} error:{}", config->device,
                pcap_geterr(handle));
      handle = nullptr;
      return CAP_FILTER;
    }
  }
  linktype = pcap_datalink(handle);
  switch (linktype) {
    case (DLT_EN10MB):
      LOG_WARN("Ethernet link detected");
      break;
    case (DLT_PPP):
      LOG_WARN("PPP link detected");
      break;
    case (DLT_LINUX_SLL):
      LOG_WARN("Linux Cooked Socket link detected");
      break;
    default:
      LOG_WARN("No PPP or Ethernet link:{}", linktype);
      break;
  }

  fd = pcap_get_selectable_fd(handle);
  return 0;
}

void CapHandle::AddCallback(
    CapPacketType type,
    std::function<int32_t(int32_t, const struct pcap_pkthdr*, const u_char*)>
        callback) {
  callbacks[type] = callback;
}

int32_t CapHandle::Dispatch(int32_t cnt, u_char* user, int32_t size,
                            pcap_handler callback, u_char* ctx) {
  if (handle == nullptr) {
    return CAP_HANDLE_INIT;
  }
  userdata = user;
  userdataSize = size;
  return pcap_dispatch(handle, cnt, callback, ctx);
}

int32_t CapHandle::SetNonBlock(int32_t fd, char* err) {
  if (handle == nullptr) {
    return CAP_HANDLE_INIT;
  }
  return pcap_setnonblock(handle, fd, err);
}

char* CapHandle::GetError() {
  if (handle == nullptr) {
    return nullptr;
  }
  return pcap_geterr(handle);
}
}  // namespace Capture