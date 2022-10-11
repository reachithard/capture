#include "cap_inode_helper.h"

#include <arpa/inet.h>
#include <dirent.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <fstream>

#include "capturecxx.h"
#include "export/capture_errors.h"
#include "utils/logger.hpp"

namespace Capture {
int32_t CapInodeHelper::Refresh(const std::set<pid_t> &pids) {
  // 1. 进行清除
  netToInodes.clear();
  inodeToPids.clear();  // 进行映射的清除

  // 建立映射关系
  int32_t ret = RefreshToInodes();
  if (ret != 0) {
    return ret;
  }

  ret = RefreshToPids(pids);
  for (auto it = netToInodes.begin(); it != netToInodes.end(); it++) {
    LOG_DEBUG("get ip:{} inodes:{}", it->first, it->second);
  }
  return ret;
}

int32_t CapInodeHelper::GetPidByIp(const std::string &hash, pid_t &pid) {
  auto inodeIt = netToInodes.find(hash);
  if (inodeIt == netToInodes.end()) {
    LOG_ERROR("can't find ip:{} to inodes", hash);
    return CAP_FIND_IP;
  }

  auto pidIt = inodeToPids.find(inodeIt->second);
  if (pidIt == inodeToPids.end()) {
    LOG_ERROR("can't find inode:{} to pid", inodeIt->second);
    return CAP_FIND_INODE;
  }
  pid = pidIt->second;
  LOG_DEBUG("find pid:{}", pid);
  return 0;
}

int32_t CapInodeHelper::RefreshToInodes() {
  int32_t ret = 0;
  // parse /proc/net/tcp
  ret = Parse("/proc/net/tcp");
  if (ret != 0) {
    return ret;
  }
  // parse /proc/net/tcp6
  ret = Parse("/proc/net/tcp6");
  if (ret != 0) {
    return ret;
  }
  // parse /proc/net/udp
  ret = Parse("/proc/net/udp");
  if (ret != 0) {
    return ret;
  }
  // parse /proc/net/udp6
  ret = Parse("/proc/net/udp6");
  if (ret != 0) {
    return ret;
  }
  return ret;
}

int32_t CapInodeHelper::Parse(const std::string &file) {
  constexpr uint32_t size = 1024;
  char buffer[size] = {0};
  std::fstream s(file, s.in);
  int ret = 0;
  if (!s.is_open()) {
    LOG_DEBUG("open file error:{}", file);
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
      ParseBuffer(buffer, size);
    }
  }
  return ret;
}

void CapInodeHelper::ParseBuffer(const char *buffer, uint32_t size) {
  LOG_DEBUG("parse buffer:{}", buffer);
  short int family;
  struct in6_addr localRet = {};
  struct in6_addr remoteRet = {};

  char remAddr[128] = {0};
  char locAddr[128] = {0};
  int locPort = 0;
  int remPort = 0;
  struct in6_addr localIn = {};
  struct in6_addr remoteIn = {};

  unsigned long inode;

  int matches = sscanf(buffer,
                       "%*d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %*X "
                       "%*X:%*X %*X:%*X %*X %*d %*d %ld %*512s\n",
                       locAddr, &locPort, remAddr, &remPort, &inode);

  if (matches != 5) {
    LOG_ERROR("can't match format{}", buffer);
    return;
  }

  if (inode == 0) {
    LOG_INFO("get inode 0 and return");
    return;
  }

  if (strlen(locAddr) > 8) {
    sscanf(locAddr, "%08X%08X%08X%08X", &localIn.s6_addr32[0],
           &localIn.s6_addr32[1], &localIn.s6_addr32[2], &localIn.s6_addr32[3]);
    sscanf(remAddr, "%08X%08X%08X%08X", &remoteIn.s6_addr32[0],
           &remoteIn.s6_addr32[1], &remoteIn.s6_addr32[2],
           &remoteIn.s6_addr32[3]);

    if ((localIn.s6_addr32[0] == 0x0) && (localIn.s6_addr32[1] == 0x0) &&
        (localIn.s6_addr32[2] == 0xFFFF0000)) {
      localRet.s6_addr32[0] = localIn.s6_addr32[3];
      remoteRet.s6_addr32[0] = remoteIn.s6_addr32[3];
      family = AF_INET;
    } else {
      localRet = localIn;
      remoteRet = remoteIn;
      family = AF_INET6;
    }
  } else {
    /* this is an IPv4-style row */
    sscanf(locAddr, "%X", (unsigned int *)&localRet);
    sscanf(remAddr, "%X", (unsigned int *)&remoteRet);
    family = AF_INET;
  }

  char local[50] = {0};
  char remote[50] = {0};
  inet_ntop(family, &localRet, local, 49);
  inet_ntop(family, &remoteRet, remote, 49);

  std::string hash = std::string(local) + ":" + std::to_string(locPort) + "-" +
                     std::string(remote) + std::to_string(remPort);

  netToInodes[hash] = inode;

  // device
  const std::vector<std::unique_ptr<CapDevice>> &devices =
      Singleton<CaptureCxx>::Get().GetDevices();
  for (std::vector<std::unique_ptr<CapDevice>>::const_iterator it =
           devices.begin();
       it != devices.end(); it++) {
    hash = it->get()->GetLocalAddr() + ":" + std::to_string(locPort) + "-" +
           std::string(remote) + ":" + std::to_string(remPort);
    netToInodes[hash] = inode;
  }
}

int32_t CapInodeHelper::RefreshToPids(const std::set<pid_t> &pids) {
  for (auto pid : pids) {
    char dirname[128] = {0};
    sprintf(dirname, "/proc/%d/fd", pid);

    DIR *dir = opendir(dirname);
    if (!dir) {
      return CAP_PROCESS_OPEN_DIR;
    }

    dirent *entry;
    uint32_t cnt = 0;
    while ((entry = readdir(dir))) {
      if (entry->d_type != DT_LNK) {
        continue;
      }

      std::string fromName = std::string(dirname) + "/" + entry->d_name;
      constexpr int32_t linklen = 80;
      char linkname[linklen] = {0};
      int usedlen = readlink(fromName.c_str(), linkname, linklen - 1);
      if (usedlen == -1) {
        continue;
      }
      LOG_DEBUG("link:{}", linkname);
      uint32_t inode = 0;
      if (sscanf(linkname, "socket:[ %" PRId32 "]", &inode)) {
        inodeToPids[inode] = pid;
      }
      cnt++;
    }
    closedir(dir);
  }

  return 0;
}
}  // namespace Capture