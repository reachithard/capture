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

  int32_t Parse(uint64_t imemory, uint64_t ideltacpu);

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

  const std::string &GetName() const { return name; }

  const std::string &GetCmdline() const { return cmdline; }

  const std::string &GetUser() const { return user; }

  const std::string &GetGroup() const { return group; }

  pid_t GetPid() const { return pid; }

  uid_t GetUid() const { return uid; }

  gid_t GetGid() const { return gid; }

  uint32_t GetFdCnt() const { return fdCnt; }
  uint64_t GetMemory() const { return memory; }
  double GetCpuPercent() const { return cpuPercent; }
  double GetMemPercent() const { return memPercent; }

  uint64_t GetIoRead() const { return ioRead; }
  uint64_t GetIoWrite() const { return ioWrite; }
  uint64_t GetRecv() const { return recv; }
  uint64_t GetSend() const { return send; }

 protected:
  int32_t ParseProc(uint64_t imemory, uint64_t ideltacpu);

  int32_t ParseStatus();

  int32_t ParseCmdline();

  int32_t ParseIo();

  int32_t ParseUserAndGroup();

  int32_t ParseHandle();

  int32_t ParseStat();

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

  uint64_t cpu = 0;
  uint64_t deltaCpu = 0;
};
}  // namespace Capture

#endif  // _CAP_PROCESS_H_
