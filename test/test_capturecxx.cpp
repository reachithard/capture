#include <gtest/gtest.h>

#include "capturecxx.h"
#include "utils/logger.hpp"

using namespace Capture;
TEST(test_capturecxx, ctor) {
  CaptureInitt config;
  memset(&config, 0, sizeof(CaptureInitt));
  config.logfile = "./logs/test_capturecxx.log";
  config.ms = 100;
  config.snaplen = 100;
  Singleton<CaptureCxx>::Get().Init(&config);

  for (int idx = 0; idx < 3; idx++) {
    LOG_DEBUG("start update");
    Singleton<CaptureCxx>::Get().Update(-1);
    sleep(1);
  }
}