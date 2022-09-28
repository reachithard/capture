ADD_LIBRARY(intf INTERFACE)

# TARGET_INCLUDE_DIRECTORIES(intf INTERFACE ${BASE_DIR})
#
# TARGET_COMPILE_OPTIONS(intf INTERFACE 
#						-Wall
#						$<$<CONFIG:RELEASE>:-O3>
#						$<$<CONFIG:DEBUG>:-O0 -g -fsanitize=address>
#						$<$<BOOL:${ENABLE_COV}>:-fprofile-arcs -ftest-coverage>
#						$<$<BOOL:${ENABLE_PROF}>:-pg>)
#
# TARGET_COMPILE_DEFINITIONS(intf INTERFACE 
#						$<$<CONFIG:RELEASE>:CFG_BUILD_NDEBUG>
#						$<$<CONFIG:DEBUG>:CFG_BUILD_DEBUG>)
#
# TARGET_LINK_OPTIONS(intf INTERFACE 
#						$<$<CONFIG:DEBUG>:-fsanitize=address>
#						$<$<BOOL:${ENABLE_COV}>:-fprofile-arcs -ftest-coverage>
#						$<$<BOOL:${ENABLE_PROF}>:-pg>)
#
# TARGET_LINK_DIRECTORIES(intf INTERFACE 
#)
#
# TARGET_LINK_LIBRARIES(intf INTERFACE
#						$<$<CONFIG:DEBUG>:-staitc-libasan>
#						$<$<BOOL:${ENABLE_COV}>:-lgcov>)

TARGET_INCLUDE_DIRECTORIES(intf INTERFACE 
                                ${BASE_DIR}/src)

TARGET_COMPILE_OPTIONS(intf INTERFACE 
						-Wall
						$<$<CONFIG:RELEASE>:-O3>
						$<$<CONFIG:DEBUG>:-O0 -g>
)

TARGET_COMPILE_DEFINITIONS(intf INTERFACE 
)

TARGET_LINK_OPTIONS(intf INTERFACE 
)

TARGET_LINK_DIRECTORIES(intf INTERFACE 
)

TARGET_LINK_LIBRARIES(intf INTERFACE)