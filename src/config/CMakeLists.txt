aux_source_directory(. SRC_LIST)
add_library(config SHARED ${SRC_LIST})
target_link_libraries(config liblog.so ${BOOST_LIBRARIES} libyaml-cpp.a)