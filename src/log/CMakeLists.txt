aux_source_directory(. SRC_LIST)
add_library(phase0_log SHARED ${SRC_LIST})
target_link_libraries(phase0_log pthread)