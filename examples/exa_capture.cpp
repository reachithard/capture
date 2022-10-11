#include <unistd.h>

#include <cstring>
#include <iostream>

#include "libcapture.h"

static void Process(CaptureAction action, const Process_t* processes,
                    uint32_t* size) {
  uint32_t tmp = *size;
  if (action == ACTION_REMOVE) {
    std::cout << "remove pid:" << processes[0].pid << std::endl;
  } else {
    for (uint32_t idx = 0; idx < tmp; idx++) {
      std::cout << "name:" << processes[idx].name
                << " cmdline:" << processes[idx].cmdline
                << " user:" << processes[idx].user
                << " group:" << processes[idx].group
                << " pid:" << processes[idx].pid
                << " uid:" << processes[idx].uid
                << " gid:" << processes[idx].gid
                << " fdCnt:" << processes[idx].fdCnt
                << " memory:" << processes[idx].memory
                << " cpuPercent:" << processes[idx].cpuPercent
                << " memPercent:" << processes[idx].memPercent
                << " ioRead:" << processes[idx].ioRead
                << " ioWrite:" << processes[idx].ioWrite
                << " recv:" << processes[idx].recv
                << " send:" << processes[idx].send;
      std::cout << std::endl;
    }
  }
}

static void Packet(const Packet_t* packets, uint32_t* size) {
  uint32_t tmp = *size;
  // 打印包内容
  for (uint32_t idx = 0; idx < tmp; idx++) {
    std::cout << "hash:" << packets[idx].hash
              << " family:" << packets[idx].family
              << " packetSize:" << packets[idx].packetSize;
    std::cout << std::endl;
  }
}

int main() {
  CaptureInitt config;
  memset(&config, 0, sizeof(CaptureInitt));
  config.logfile = "./logs/exa_capture.log";
  config.ms = 100;
  config.snaplen = 100;

  int32_t ret = CaptureInit(&config, &Process, &Packet);
  while (true) {
    /* code */
    CaptureUpdate(-1);
    sleep(3);
  }

  return 0;
}