set(SRC_LIST testLog.cc)
add_executable(testLog ${SRC_LIST})
target_link_libraries(testLog libphase0.so yaml-cpp pthread)

set(SRC_LIST testConfig.cc)
add_executable(testConfig ${SRC_LIST})
target_link_libraries(testConfig libphase0.so yaml-cpp pthread)

set(SRC_LIST testFiber.cc)
add_executable(testFiber ${SRC_LIST})
target_link_libraries(testFiber libphase0.so yaml-cpp pthread)

set(SRC_LIST testFiber2.cc)
add_executable(testFiber2 ${SRC_LIST})
target_link_libraries(testFiber2 libphase0.so yaml-cpp pthread)

set(SRC_LIST testScheduler.cc)
add_executable(testScheduler ${SRC_LIST})
target_link_libraries(testScheduler libphase0.so yaml-cpp pthread)
