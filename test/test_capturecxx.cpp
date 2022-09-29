#include <gtest/gtest.h>

#include "capturecxx.h"

using namespace Capture;
TEST(test_capturecxx, ctor) {
  CaptureInitt config;
  config.logfile = "./logs/test_capturecxx.log";
  Singleton<CaptureCxx>::Get().Init(&config);
}