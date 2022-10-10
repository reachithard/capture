#ifndef _CAP_PROCESS_H_
#define _CAP_PROCESS_H_

#include <memory>
#include <string>
#include <vector>

#include "cap_packet.h"

namespace Capture {

union ParseType {
  /* data */
  char name[128];
  uint64_t memory;
  uint64_t ioRead;
  uint64_t ioWrite;
};

class CapProcess {
 public:
  explicit CapProcess(pid_t ipid) : pid(ipid) {}

  int32_t Parse();

  void Reset() {
    name.clear();
    cmdline.clear();
    user.clear();
    pid = -1;
    uid = 0;
    memory = 0;
    cpuPercent = 0.0;
    memPercent = 0.0;
    fdCnt = 0;
    ioRead = 0;
    ioWrite = 0;
    recv = 0;
    send = 0;
  }

  void Info();

  void AddPacket(const std::unique_ptr<CapPacket> &ptr);

 protected:
  int32_t ParseProc();

  int32_t ParseStatus();

  int32_t ParseCmdline();

  int32_t ParseIo();

  int32_t ParseUserAndGroup();

  int32_t ParseHandle();

 private:
  std::string name;
  std::string cmdline;
  std::string user;
  std::string group;
  pid_t pid = -1;
  uid_t uid = 0;
  gid_t gid = 0;
  uint32_t fdCnt = 0;
  uint64_t memory = 0;
  double cpuPercent = 0.0;
  double memPercent = 0.0;

  uint64_t ioRead = 0;
  uint64_t ioWrite = 0;
  uint64_t recv = 0;
  uint64_t send = 0;
};
}  // namespace Capture

#endif  // _CAP_PROCESS_H_
