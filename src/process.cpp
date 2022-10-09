#include "process.h"

#include <dirent.h>
#include <grp.h>
#include <inttypes.h>
#include <pwd.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

#include "export/capture_errors.h"
#include "utils/logger.hpp"

namespace Capture {
int32_t Process::Parse() { return ParseProc(); }

int32_t Process::ParseProc() {
  if (pid == -1) {
    return CAP_PROCESS_INIT;
  }

  // /proc/pid/status
  int ret = 0;
  ret = ParseStatus();
  if (ret != 0) {
    return ret;
  }

  ret = ParseCmdline();
  if (ret != 0) {
    return ret;
  }

  ParseIo();  // io有可能失败 因为没有权限

  ret = ParseUserAndGroup();
  if (ret != 0) {
    return ret;
  }

  inodes.clear();
  ParseNetInfo();

  return ret;
}

int32_t Process::ParseStatus() {
  constexpr uint32_t size = 1024;
  char buffer[size] = {0};
  char filename[256] = {0};
  int ret = sprintf(filename, "/proc/%d/status", pid);
  if (ret <= 0) {
    LOG_DEBUG("format error:{}", pid);
    return CAP_PROCESS_FORMAT;
  }
  std::fstream s(filename, s.in);
  if (!s.is_open()) {
    LOG_DEBUG("open file error:{}", filename);
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
      ParseType temp;
      memset(&temp, 0, sizeof(ParseType));
      if (name.empty() && sscanf(buffer, "Name:       %s", temp.name) != 0) {
        name =
            (temp.name == nullptr ? "unknow process" : std::string(temp.name));
      }
      if (sscanf(buffer, "VmRSS:          %" PRId64 " kB", &temp.memory)) {
        LOG_INFO("init memory:{} buffer:{}", temp.memory, buffer);
        memory = temp.memory;
      }

      sscanf(buffer, "Uid: %u", &uid);
      sscanf(buffer, "Gid: %u", &gid);
    }
  }
  return 0;
}

int32_t Process::ParseCmdline() {
  char filename[256] = {0};
  int ret = sprintf(filename, "/proc/%d/cmdline", pid);
  if (ret <= 0) {
    LOG_DEBUG("format error:{}", pid);
    return CAP_PROCESS_FORMAT;
  }
  std::fstream s(filename, s.in);
  if (!s.is_open()) {
    LOG_DEBUG("open file error:{}", filename);
    return CAP_PROCESS_OPEN;
  } else {
    // parse
    s >> cmdline;
  }
  return 0;
}

int32_t Process::ParseIo() {
  constexpr uint32_t size = 1024;
  char buffer[size] = {0};
  char filename[256] = {0};
  int ret = sprintf(filename, "/proc/%d/io", pid);
  if (ret <= 0) {
    LOG_DEBUG("format error:{}", pid);
    return CAP_PROCESS_FORMAT;
  }
  std::fstream s(filename, s.in);
  if (!s.is_open()) {
    LOG_DEBUG("open file error:{}", filename);
    return CAP_PROCESS_OPEN;
  } else {
    // parse
    while (!s.eof()) {
      /* code */
      s.getline(buffer, size);
      if (strcmp(buffer, "") == 0) {
        continue;
      }
      ParseType temp;
      memset(&temp, 0, sizeof(ParseType));

      if (sscanf(buffer, "rchar: %" PRId64 "", &temp.ioRead) != 0) {
        ioRead = temp.ioRead;
      }

      if (sscanf(buffer, "wchar: %" PRId64 "", &temp.ioWrite) != 0) {
        ioWrite = temp.ioWrite;
      }
    }
  }
  return 0;
}

int32_t Process::ParseUserAndGroup() {
  struct passwd *pass;
  struct group *gr;

  pass = getpwuid(uid);
  gr = getgrgid(gid);
  if (pass && pass->pw_name) {
    user = std::string(pass->pw_name);
  } else {
    user = std::to_string(uid);
  }
  if (gr && gr->gr_name) {
    group = std::string(gr->gr_name);
  } else {
    user = std::to_string(gid);
  }

  return 0;
}

int32_t Process::ParseNetInfo() {
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
      inodes.push_back(inode);
    }
    cnt++;
  }
  fdCnt = cnt;
  closedir(dir);
  return 0;
}

void Process::Info() {
  std::cout << "name:" << name << " cmdline:" << cmdline << " user:" << user
            << " group:" << group << " pid:" << pid << " uid:" << uid
            << " gid:" << gid << " fdCnt:" << fdCnt << " memory:" << memory
            << " cpuPercent:" << cpuPercent << " memPercent:" << memPercent
            << " ioRead:" << ioRead << " ioWrite:" << ioWrite
            << " recv:" << recv << " send:" << send;

  std::cout << " inodes:";
  for (uint32_t idx = 0; idx < inodes.size(); idx++) {
    std::cout << inodes[idx] << ",";
  }
  std::cout << std::endl;
}
}  // namespace Capture