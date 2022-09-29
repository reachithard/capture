get_filename_component(spdlog_ROOT ${BASE_DIR}/output ABSOLUTE)

message("spdlog_ROOT ${spdlog_ROOT}")

find_package(spdlog)

include(CMakePrintHelpers)
cmake_print_variables(spdlog_FOUND)

function(spdlog_deps Target)
    target_link_libraries(${Target} PRIVATE spdlog::spdlog)
endfunction(spdlog_deps Target)