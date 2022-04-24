set(SRC_LIST testLog.cc)
add_executable(testLog ${SRC_LIST})
target_link_libraries(testLog libphase0_log.so libphase0_utils.so pthread)

set(SRC_LIST testConfig.cc)
add_executable(testConfig ${SRC_LIST})
target_link_libraries(testConfig libphase0_log.so libphase0_config.so libphase0_utils.so libyaml-cpp.a pthread)