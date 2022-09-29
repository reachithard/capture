file(GLOB CaptureSource *.c *.cpp)

add_library(modcapture STATIC ${CaptureSource})
target_link_libraries(modcapture PRIVATE intf)
spdlog_deps(modcapture)