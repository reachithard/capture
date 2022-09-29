find_package(LIBPCAP)

include(CMakePrintHelpers)
cmake_print_variables(LIBPCAP_FOUND)

function(pcap_deps Target)
    target_link_libraries(${Target} PRIVATE LibPcap::LibPcap)
endfunction(pcap_deps Target)