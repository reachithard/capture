#include "libcapture.h"

#include "capturecxx.h"

int32_t CaptureInit(const CaptureInitt *config,
                    ProcessInfoCallback processCallback,
                    PacketCallback packetCallback) {
  using namespace Capture;
  return Singleton<CaptureCxx>::Get().Init(config);
}

/*
由用户保证线程安全
*/
int32_t CaptureUpdate(int32_t cnt) {
  using namespace Capture;
  return Singleton<CaptureCxx>::Get().Update(cnt);
}

int32_t CaptureExit() {
  using namespace Capture;
  return Singleton<CaptureCxx>::Get().Shutdown();
}