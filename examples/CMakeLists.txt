add_executable(exa_capture exa_capture.cpp)
target_link_libraries(exa_capture PRIVATE capture_deps intf)
spdlog_deps(exa_capture)
pcap_deps(exa_capture)