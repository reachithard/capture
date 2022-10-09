#ifndef _CAP_HANDLE_H_
#define _CAP_HANDLE_H_

#include <pcap.h>

#include <functional>
#include <unordered_map>

#include "utils/nocopy.h"

namespace Capture {
enum CapPacketType {
  CAP_PACKET_ETHERNET,
  CAP_PACKET_PPP,
  CAP_PACKET_SLL,
  CAP_PACKET_IP,
  CAP_PACKET_IP6,
  CAP_PACKET_TCP,
  CAP_PACKET_UDP,
  CAP_PACKET_CNT
};

typedef struct CapHandleInit_s {
  /* data */
  const char* device = nullptr;
  int32_t snaplen = 512;
  int32_t promise = 0;
  int32_t ms = 100;
  const char* fliter = nullptr;
  char* errorbuf = nullptr;
} CapHandleInit_t;

typedef struct CtxIp4_s {
  in_addr ip_src;
  in_addr ip_dst;
} CtxIp4_t;

typedef struct CtxIp6_s {
  in6_addr ip6_src;
  in6_addr ip6_dst;
} CtxIp6_t;

union CtxAddr {
  CtxIp4_t ip4;
  CtxIp6_t ip6;
};

typedef struct PcapCtx_s {
  int32_t family = AF_UNSPEC;
  CtxAddr addr;
} PcapCtx_t;

#define CAP_ERRORBUFFER_SIZE PCAP_ERRBUF_SIZE

class CapHandle : public Nocopy {
 public:
  CapHandle();
  ~CapHandle();

  int32_t OpenLive(const CapHandleInit_t* config);

  void SetCallback(
      std::function<int32_t(u_char*, const struct pcap_pkthdr*, const u_char*)>
          callback);

  int32_t Dispatch(int32_t cnt, u_char* ctx);

  int32_t DataLink() const { return linktype; }

  int32_t SetNonBlock(int32_t fd, char* err);

  char* GetError();

  void SetDevice(const std::string& name) { device = name; }

  int32_t GetFd() const { return fd; }

  static void HandlePacket(u_char* ctx, const struct pcap_pkthdr* header,
                           const u_char* packet);

  void SetContext(in_addr src, in_addr dest) {
    context.family = AF_INET;
    context.addr.ip4.ip_src = src;
    context.addr.ip4.ip_dst = dest;
  }

  void SetContext(in6_addr src, in6_addr dest) {
    context.family = AF_INET6;
    context.addr.ip6.ip6_src = src;
    context.addr.ip6.ip6_dst = dest;
  }

  const PcapCtx_t& GetContext() const { return context; }

 private:
  pcap_t* handle = nullptr;
  int32_t linktype;
  std::string device;
  int32_t fd = -1;
  PcapCtx_t context;
  std::function<int32_t(u_char*, const struct pcap_pkthdr*, const u_char*)>
      callback = nullptr;
};
}  // namespace Capture

#endif  // _CAP_HANDLE_H_