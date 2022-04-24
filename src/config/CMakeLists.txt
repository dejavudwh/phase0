aux_source_directory(. SRC_LIST)
add_library(phase0_config SHARED ${SRC_LIST})
target_link_libraries(phase0_config libphase0_log.so ${BOOST_LIBRARIES} libyaml-cpp.a)