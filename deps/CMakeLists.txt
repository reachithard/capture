cmake_minimum_required(VERSION 3.12)

project(deps)

set(CMAKE_CXX_STANDARD 11)

SET(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/../output)
SET(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty)
message("deps install ${CMAKE_INSTALL_PREFIX}")

INCLUDE(cmake/gtest.cmake)
INCLUDE(cmake/libpcap.cmake)