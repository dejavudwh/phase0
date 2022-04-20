set(SRC_LIST testLog.cc)
add_executable(testLog ${SRC_LIST})
target_link_libraries(testLog liblog.so pthread)

set(SRC_LIST testConfig.cc)
add_executable(testConfig ${SRC_LIST})
target_link_libraries(testConfig liblog.so libconfig.so libyaml-cpp.a pthread)