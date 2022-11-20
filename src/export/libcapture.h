#ifndef _LIBCAPTURE_H_
#define _LIBCAPTURE_H_

#include "capture_pod.h"

#if __cplusplus
extern "C" {
#endif

CAPTURE_DSO_VISIBLE int32_t CaptureInit(const CaptureInitt *config,
                                        ProcessInfoCallback processCallback,
                                        PacketCallback packetCallback);

/*
由用户保证线程安全
*/
CAPTURE_DSO_VISIBLE int32_t CaptureUpdate(int32_t cnt);

CAPTURE_DSO_VISIBLE int32_t CaptureExit();
#if __cplusplus
}
#endif

#endif // _LIBCAPTURE_H_