#[=======================================================================[.rst:
FindLibPcap
-----------

.. versionadded:: 3.14

Find the LIBPCAP libraries, v3

IMPORTED targets
^^^^^^^^^^^^^^^^

This module defines the following :prop_tgt:`IMPORTED` target:

``LibPcap::LibPcap``

Result variables
^^^^^^^^^^^^^^^^

This module will set the following variables if found:

``LIBPCAP_INCLUDE_DIRS``
  where to find LibPcap.h, etc.
``LIBPCAP_LIBRARIES``
  the libraries to link against to use LIBPCAP.
``LIBPCAP_FOUND``
  TRUE if found

#]=======================================================================]

set(SEARCH_PATH "${CMAKE_INSTALL_PREFIX}")
message("search ${SEARCH_PATH}")

# Look for the necessary header
find_path(LIBPCAP_INCLUDE_DIR NAMES pcap.h
        PATHS  ${SEARCH_PATH}/include)
mark_as_advanced(LIBPCAP_INCLUDE_DIR)
message("search ${LIBPCAP_INCLUDE_DIR}")


# Look for the necessary library
find_library(LIBPCAP_LIBRARY NAMES libpcap.so
        PATHS ${SEARCH_PATH}/lib)
mark_as_advanced(LIBPCAP_LIBRARY)

# # Extract version information from the header file
# if(LIBPCAP_INCLUDE_DIR)
#     file(STRINGS ${LIBPCAP_INCLUDE_DIR}/LIBPCAP.h _ver_line
#          REGEX "^#define LIBPCAP_VERSION  *\"[0-9]+\\.[0-9]+\\.[0-9]+\""
#          LIMIT_COUNT 1)
#     string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+"
#            LIBPCAP_VERSION "${_ver_line}")
#     unset(_ver_line)
# endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LIBPCAP
    REQUIRED_VARS LIBPCAP_INCLUDE_DIR LIBPCAP_LIBRARY)

    message("search ${LIBPCAP_FOUND}")

# Create the imported target
if(LIBPCAP_FOUND)
    set(LIBPCAP_INCLUDE_DIRS ${LIBPCAP_INCLUDE_DIR})
    set(LIBPCAP_LIBRARIES ${LIBPCAP_LIBRARY})
    if(NOT TARGET LibPcap::LibPcap)
        add_library(LibPcap::LibPcap UNKNOWN IMPORTED)
        set_target_properties(LibPcap::LibPcap PROPERTIES
            IMPORTED_LOCATION             "${LIBPCAP_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${LIBPCAP_INCLUDE_DIR}")
    endif()
endif()