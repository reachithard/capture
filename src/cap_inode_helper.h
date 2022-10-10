#ifndef _CAP_INODE_HELPER_H_
#define _CAP_INODE_HELPER_H_

#include <set>
#include <string>
#include <unordered_map>

#include "utils/singleton.h"

namespace Capture {
class CapInodeHelper : public Singleton<CapInodeHelper> {
 public:
  int32_t Refresh(const std::set<pid_t>& pids);

 protected:
  int32_t RefreshToInodes();

  int32_t Parse(const std::string& file);

  int32_t ParseBuffer(const char* buffer, uint32_t size);

  int32_t RefreshToPids(const std::set<pid_t>& pids);

 private:
  std::unordered_map<std::string, uint32_t> netToInodes;
  std::unordered_map<uint32_t, pid_t> inodeToPids;
};
}  // namespace Capture

#endif  // _CAP_INODE_HELPER_H_