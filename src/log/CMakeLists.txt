aux_source_directory(. SRC_LIST)
add_library(log SHARED ${SRC_LIST})
target_link_libraries(log pthread)