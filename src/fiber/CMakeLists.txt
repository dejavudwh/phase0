aux_source_directory(. SRC_LIST)
add_library(phase0_fiber SHARED ${SRC_LIST})
target_link_libraries(phase0_fiber phase0_log.so phase0_utils.so pthread)