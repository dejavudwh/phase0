set(SRC_LIST testLog.cc)
add_executable(testLog ${SRC_LIST})
target_link_libraries(testLog libphase0.so yaml-cpp pthread)

set(SRC_LIST testConfig.cc)
add_executable(testConfig ${SRC_LIST})
target_link_libraries(testConfig libphase0.so yaml-cpp pthread)

set(SRC_LIST testFiber.cc)
add_executable(testFiber ${SRC_LIST})
target_link_libraries(testFiber libphase0.so yaml-cpp pthread)