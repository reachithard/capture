#include "cap_packet.h"

#include "capturecxx.h"

namespace Capture {
int32_t CapPacket::Init(in_addr isip, unsigned short isport, in_addr idip,
                        unsigned short idport, u_int32_t ilen, timeval itime,
                        CapDir idir /*= Unknown*/) {
  sip = isip;
  sport = isport;
  dip = idip;
  dport = idport;
  len = ilen;
  time = itime;
  dir = idir;
  family = AF_INET;
  Hash();
  return 0;
}

int32_t CapPacket::Init(in6_addr isip, unsigned short isport, in6_addr idip,
                        unsigned short idport, u_int32_t ilen, timeval itime,
                        CapDir idir /*= Unknown*/) {
  sip6 = isip;
  sport = isport;
  dip6 = idip;
  dport = idport;
  len = ilen;
  time = itime;
  dir = idir;
  family = AF_INET6;
  Hash();
  return 0;
}

CapDir CapPacket::Invert() {
  if (dir == Incoming) {
    dir = Outgoing;
  } else if (dir == Outgoing) {
    dir = Incoming;
  }

  if (family == AF_INET) {
    std::swap(sip, dip);
    std::swap(sport, dport);
  } else if (family == AF_INET6) {
    std::swap(sip6, dip6);
    std::swap(sport, dport);
  }

  return dir;
}

bool CapPacket::IsOutgoing() {
  switch (dir) {
    case Outgoing:
      return true;
    case Incoming:
      return false;
    case Unknown:
      bool islocal;
      if (family == AF_INET) {
        islocal = Singleton<CaptureCxx>::Get().Contains(sip.s_addr);
      } else {
        islocal = Singleton<CaptureCxx>::Get().Contains(sip6);
      }
      if (islocal) {
        dir = Outgoing;
        return true;
      } else {
        dir = Incoming;
        return false;
      }
  }
  return false;
}

const std::string &CapPacket::Hash() {
  if (!hashstring.empty()) {
    return hashstring;
  }

  char local[50] = {0};
  char remote[50] = {0};
  if (family == AF_INET) {
    inet_ntop(family, &sip, local, 49);
    inet_ntop(family, &dip, remote, 49);
  } else {
    inet_ntop(family, &sip6, local, 49);
    inet_ntop(family, &dip6, remote, 49);
  }
  if (IsOutgoing()) {
    hashstring = std::string(local) + ":" + std::to_string(sport) + "-" +
                 std::string(remote) + ":" + std::to_string(dport);
    LOG_ERROR("outgoing packet:{}", hashstring);
  } else {
    hashstring = std::string(remote) + ":" + std::to_string(dport) + "-" +
                 std::string(local) + ":" + std::to_string(sport);
    LOG_ERROR("incoming packet:{}", hashstring);
  }
  return hashstring;
}

const std::string CapPacket::Link() {
  std::string linkName;
  char local[50] = {0};
  char remote[50] = {0};
  if (family == AF_INET) {
    inet_ntop(family, &sip, local, 49);
    inet_ntop(family, &dip, remote, 49);
  } else {
    inet_ntop(family, &sip6, local, 49);
    inet_ntop(family, &dip6, remote, 49);
  }

  linkName = std::string(local) + ":" + std::to_string(sport) + "-" +
             std::string(remote) + ":" + std::to_string(dport);
  LOG_ERROR("linkName going in packet:{}", linkName);

  return linkName;
}
}  // namespace Capture