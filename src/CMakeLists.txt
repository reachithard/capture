file(GLOB CaptureSource *.c *.cpp)

add_library(modcapture STATIC ${CaptureSource})
target_link_libraries(modcapture PRIVATE intf)
spdlog_deps(modcapture)

add_library(capture_deps SHARED  ${CaptureSource})
target_link_libraries(capture_deps PRIVATE intf)
spdlog_deps(capture_deps)

install(TARGETS capture_deps
        LIBRARY DESTINATION lib)