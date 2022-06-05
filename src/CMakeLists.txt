# make VERBOSE = 1 显示所有编译命令

#cmake_minimum_required 需要的最小的版本
cmake_minimum_required(VERSION 3.16)

# set the project name
project(zceproject)

set ( CMAKE_VERBOSE_MAKEFILE on  )

message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
message(STATUS "CMAKE_GENERATOR: ${CMAKE_GENERATOR}")
message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

# C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8 " )

#工程外部的头文件
set(PATH_THIRDPARTY_INC "../third_party/include/")
include_directories(${PATH_THIRDPARTY_INC}lualib-5.1.5/)
include_directories(${PATH_THIRDPARTY_INC}protobuf-2.6.1/src/)
include_directories(${PATH_THIRDPARTY_INC}sqlite-3.8.8.1/) 
include_directories(${PATH_THIRDPARTY_INC}mysql-linux-5.6.22/)
include_directories(${PATH_THIRDPARTY_INC}rapidxml-1.13/)

set(CMAKE_DEBUG_POSTFIX "_d") 
set(CMAKE_RELEASE_POSTFIX "_r")

#输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../bin/ )
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../bin/ )
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../lib/ )
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/../lib/debug/ )
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_REALSE ${CMAKE_SOURCE_DIR}/../lib/realse/ )

add_subdirectory(commlib/zcelib)
add_subdirectory(commlib/soarlib)
add_subdirectory(test/zealot)

