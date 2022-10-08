#include "cap_protocol.h"

// ether_header声明
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <sys/types.h>

#include "utils/logger.hpp"

namespace Capture {
void CapProtocol::Init() { LOG_DEBUG("get init"); }

int32_t CapProtocol::Parse(u_char *u_handle, const struct pcap_pkthdr *header,
                           const u_char *packet) {
  CapHandle *ctx = reinterpret_cast<CapHandle *>(u_handle);
  // https://www.tcpdump.org/linktypes.html
  switch (ctx->DataLink()) {
    case DLT_EN10MB:
      ProtocolEN10MB(ctx, header, packet);
      break;
    case DLT_RAW:
      /* code */
      ProtocolIp(ctx, header, packet);
      break;
    default:
      LOG_INFO("unknow link type:{}", ctx->DataLink());
      break;
  }
  return 0;
}

void CapProtocol::ProtocolIp(CapHandle *ctx, const struct pcap_pkthdr *header,
                             const u_char *packet) {
  // 需要找到是ipv4 还是ipv6的
  if (header->len < 1) {
    LOG_INFO("truncated ip length:{}", header->len);
    return;
  }
  uint8_t version = ((uint8_t)(*(packet)));
  switch (version & 0XF0) {
    case 0X40:
      /* code */
      LOG_DEBUG("get a ipv4 packet");
      ProtocolIp4(ctx, header, packet);
      break;
    case 0X60:
      LOG_DEBUG("get a ipv6 packet");
      ProtocolIp6(ctx, header, packet);
      break;
    default:
      LOG_DEBUG("get a unknow version");
      break;
  }
}

void CapProtocol::ProtocolEN10MB(CapHandle *ctx,
                                 const struct pcap_pkthdr *header,
                                 const u_char *packet) {
  const struct ether_header *ethernet = (struct ether_header *)packet;
  u_char *payload = const_cast<u_char *>(packet) + sizeof(struct ether_header);

  u_int16_t protocol = 0;
  protocol = ntohs(ethernet->ether_type);
  switch (protocol) {
    case ETHERTYPE_IP:
      ProtocolIp4(ctx, header, payload);
      break;
    case ETHERTYPE_IPV6:
      ProtocolIp6(ctx, header, payload);
      break;
    default:
      LOG_INFO("unsupport protocol:{}", protocol);
      break;
  }
}

void CapProtocol::ProtocolIp4(CapHandle *ctx, const struct pcap_pkthdr *header,
                              const u_char *packet) {
  const struct ip *ip = (struct ip *)packet;
  u_char *payload = (u_char *)packet + sizeof(struct ip);

  switch (ip->ip_p) {
    case IPPROTO_TCP:
      LOG_DEBUG("get a tcp");
      break;
    case IPPROTO_UDP:
      LOG_DEBUG("get a udp");
      break;
    default:
      LOG_INFO("unsupport protocol:{}", ip->ip_p);
      break;
  }
}

void CapProtocol::ProtocolIp6(CapHandle *ctx, const struct pcap_pkthdr *header,
                              const u_char *packet) {
  const struct ip6_hdr *ip6 = (struct ip6_hdr *)packet;
  u_char *payload = (u_char *)packet + sizeof(struct ip6_hdr);

  switch ((ip6->ip6_ctlun).ip6_un1.ip6_un1_nxt) {
    case IPPROTO_TCP:
      LOG_DEBUG("get a tcp");
      break;
    case IPPROTO_UDP:
      LOG_DEBUG("get a udp");
      break;
    default:
      LOG_INFO("unsupport protocol:{}", (ip6->ip6_ctlun).ip6_un1.ip6_un1_nxt);
      break;
  }
}
}  // namespace Capture
