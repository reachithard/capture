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

 private:
  pcap_t* handle = nullptr;
  int32_t linktype;
  u_char* userdata;
  int32_t userdataSize;
  std::string device;
  int32_t fd = -1;
  std::function<int32_t(u_char*, const struct pcap_pkthdr*, const u_char*)>
      callback = nullptr;
};
}  // namespace Capture

#endif  // _CAP_HANDLE_H_