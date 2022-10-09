#include <gtest/gtest.h>

#include <iostream>
#include <memory>

#include "process.h"
#include "utils/logger.hpp"

using namespace Capture;

TEST(test_process, parse) {
  std::unique_ptr<Process> ptr = std::make_unique<Process>(414673);
  int ret = ptr->Parse();
  std::cout << "ret:" << ret << std::endl;
  ptr->Info();
}