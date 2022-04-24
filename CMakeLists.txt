cmake_minimum_required(VERSION 3.16)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(phase0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_C_FLAGS "-g")

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)

find_package(Boost REQUIRED)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

include_directories(${CMAKE_SOURCE_DIR}/src/log/include)
include_directories(${CMAKE_SOURCE_DIR}/src/config/include)
include_directories(${CMAKE_SOURCE_DIR}/src/utils/include)
include_directories(${CMAKE_SOURCE_DIR}/thirdpart)
include_directories(${CMAKE_SOURCE_DIR}/examples)

link_directories(${CMAKE_SOURCE_DIR}/lib)

add_subdirectory(src)
add_subdirectory(examples)
add_subdirectory(tests)