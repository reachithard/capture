#ifndef _CAPTURE_POD_H_
#define _CAPTURE_POD_H_
extern "C" {

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
}

#endif  // _CAPTURE_POD_H_