#ifndef _CAPTURE_POD_H_
#define _CAPTURE_POD_H_

#include <stdint.h>

extern "C" {

#define CAPTURE_DSO_VISIBLE __attribute__((visibility("default")))
#define CAPTURE_DSO_HIDDEN __attribute__((visibility("hidden")))

#define CAPTURE_COMMAN_SIZE 128
#define CAPTURE_CMDLINE_SIZE 512

enum LoopType {
  SELECT,
  EPOLL,
  NONE,
};

typedef struct CaptureInits {
  const char* logfile;
  int32_t ms;
  int32_t snaplen;
  int32_t promise;
  const char* fliter;
  char* errorbuf;
  LoopType loop;
} CaptureInitt;

typedef struct Process_s {
  char name[CAPTURE_COMMAN_SIZE];
  char cmdline[CAPTURE_CMDLINE_SIZE];
  char user[CAPTURE_COMMAN_SIZE];
  char group[CAPTURE_COMMAN_SIZE];
  pid_t pid;
  uid_t uid;
  gid_t gid;
  uint32_t fdCnt;
  uint64_t memory;
  double cpuPercent;
  double memPercent;

  uint64_t ioRead;
  uint64_t ioWrite;
  uint64_t recv;
  uint64_t send;
} Process_t;

enum PacketType { P_TCP, P_UDP };

typedef struct Packet_s {
  char hash[CAPTURE_COMMAN_SIZE];
  PacketType family;
  u_char* packet;
  uint32_t packetSize;
} Packet_t;

enum CaptureAction { ACTION_UPDATE, ACTION_REMOVE };

typedef void (*ProcessInfoCallback)(CaptureAction action,
                                    const Process_t* processes, uint32_t* size);

typedef void (*PacketCallback)(const Packet_t* packets, uint32_t* size);
}

#endif  // _CAPTURE_POD_H_