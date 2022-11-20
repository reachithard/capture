cmake_minimum_required(VERSION 3.12)

project(capture)
set(CMAKE_CXX_STANDARD 14)


SET(CMAKE_POSITION_INDEPENDENT_CODE ON)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE} version")

# 设置基础变量
SET(BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
SET(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/output)

if(DOWNLOAD_THIRDPARTY)
    add_subdirectory(deps)
else(DOWNLOAD_THIRDPARTY)

# 设置cmake module path路径便于find_package
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# 链接interface 便于开发
INCLUDE(cmake/utils/intf.cmake)
INCLUDE(cmake/utils/gtest.cmake)
INCLUDE(cmake/utils/spdlog.cmake)
INCLUDE(cmake/utils/pcap.cmake)

set(CAPTURE_MAJOR_VERSION 0)
set(CAPTURE_MINOR_VERSION 0)
set(CAPTURE_PATCH_VERSION 1)
set(CAPTURE_VERSION
    ${CAPTURE_MAJOR_VERSION}.${CAPTURE_MINOR_VERSION}.${CAPTURE_PATCH_VERSION})
set(CAPTURE_VERSION_STRING "${CAPTURE_VERSION}")


add_subdirectory(src)
add_subdirectory(examples)

include(CTest)
enable_testing()
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --verbose)
message(STATUS "test")
add_subdirectory(test)
endif(DOWNLOAD_THIRDPARTY)