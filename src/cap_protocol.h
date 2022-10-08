#ifndef _CAP_PROTOCOL_H_
#define _CAP_PROTOCOL_H_

#include <pcap.h>

#include "cap_handle.h"
#include "utils/nocopy.h"

namespace Capture {
class CapProtocol : public Nocopy {
 public:
  void Init();

  int32_t Parse(u_char *u_handle, const struct pcap_pkthdr *header,
                const u_char *packet);

 protected:
  void ProtocolIp(CapHandle *ctx, const struct pcap_pkthdr *header,
                  const u_char *packet);

  void ProtocolEN10MB(CapHandle *ctx, const struct pcap_pkthdr *header,
                      const u_char *packet);

  void ProtocolIp4(CapHandle *ctx, const struct pcap_pkthdr *header,
                   const u_char *packet);

  void ProtocolIp6(CapHandle *ctx, const struct pcap_pkthdr *header,
                   const u_char *packet);
};
}  // namespace Capture
#endif  // _CAP_PROTOCOL_H_