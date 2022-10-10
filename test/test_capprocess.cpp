#include <gtest/gtest.h>

#include <iostream>
#include <memory>

#include "cap_process.h"
#include "utils/logger.hpp"

using namespace Capture;

TEST(test_capprocess, parse) {
  std::unique_ptr<CapProcess> ptr = std::make_unique<CapProcess>(414673);
  int ret = ptr->Parse();
  std::cout << "ret:" << ret << std::endl;
  ptr->Info();
}