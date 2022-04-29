set(SRC_LIST testLog.cc)
add_executable(testLog ${SRC_LIST})
target_link_libraries(testLog libphase0.so yaml-cpp pthread libdl.so)

set(SRC_LIST testConfig.cc)
add_executable(testConfig ${SRC_LIST})
target_link_libraries(testConfig libphase0.so yaml-cpp pthread libdl.so)

set(SRC_LIST testFiber.cc)
add_executable(testFiber ${SRC_LIST})
target_link_libraries(testFiber libphase0.so yaml-cpp pthread libdl.so)

set(SRC_LIST testFiber2.cc)
add_executable(testFiber2 ${SRC_LIST})
target_link_libraries(testFiber2 libphase0.so yaml-cpp pthread libdl.so)

set(SRC_LIST testScheduler.cc)
add_executable(testScheduler ${SRC_LIST})
target_link_libraries(testScheduler libphase0.so yaml-cpp pthread libdl.so)

set(SRC_LIST testIOManager.cc)
add_executable(testIOManager ${SRC_LIST})
target_link_libraries(testIOManager libphase0.so yaml-cpp pthread libdl.so)

set(SRC_LIST testTimer.cc)
add_executable(testTimer ${SRC_LIST})
target_link_libraries(testTimer libphase0.so yaml-cpp pthread libdl.so)

set(SRC_LIST testHook.cc)
add_executable(testHook ${SRC_LIST})
target_link_libraries(testHook libphase0.so yaml-cpp pthread libdl.so)

set(SRC_LIST testAddress.cc)
add_executable(testAddress ${SRC_LIST})
target_link_libraries(testAddress libphase0.so yaml-cpp pthread libdl.so)

set(SRC_LIST testSocketTcpClient.cc)
add_executable(testSocketTcpClient ${SRC_LIST})
target_link_libraries(testSocketTcpClient libphase0.so yaml-cpp pthread libdl.so)

set(SRC_LIST testSocketTcpServer.cc)
add_executable(testSocketTcpServer ${SRC_LIST})
target_link_libraries(testSocketTcpServer libphase0.so yaml-cpp pthread libdl.so)