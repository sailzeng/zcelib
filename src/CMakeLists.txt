# make VERBOSE = 1 显示所有编译命令

#cmake_minimum_required 需要的最小的版本
cmake_minimum_required(VERSION 3.16)

# set the project name
project(zceproject)

message(STATUS "Others platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "CMAKE_GENERATOR: ${CMAKE_GENERATOR}")
message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")

set (ZCE_VERSION_MAJOR 2)
set (ZCE_VERSION_MINOR 0)

# configure_file用于拷贝
configure_file (
  "${CMAKE_SOURCE_DIR}/commlib/zcelib/zce/version.h.in"
  "${CMAKE_SOURCE_DIR}/commlib/zcelib/zce/version.h"
  )

#config 是大小写敏感的

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "CMAKE_BUILD_TYPE is None")
elseif (CMAKE_BUILD_TYPE MATCHES "Debug" )
    message(STATUS "CMAKE_BUILD_TYPE is Debug")
elseif (CMAKE_BUILD_TYPE MATCHES "Release")
    message(STATUS "CMAKE_BUILD_TYPE is Release")
else ()
    message(FATAL_ERROR "unknown CMAKE_BUILD_TYPE=" ${CMAKE_BUILD_TYPE}",Please check it.")
endif ()


set(CMAKE_DEBUG_POSTFIX "_d") 
set(CMAKE_RELEASE_POSTFIX "_r") 

#输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../bin/ )
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../bin/ )
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../lib/ )
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/../lib/debug/ )
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_REALSE ${CMAKE_SOURCE_DIR}/../lib/realse/ )


# C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


#工程外部的头文件
set(PATH_THIRDPARTY_INC "../third_party/include/")
include_directories(${PATH_THIRDPARTY_INC}lualib-5.1.5/ )
include_directories(${PATH_THIRDPARTY_INC}protobuf-2.6.1/src/ )
include_directories(${PATH_THIRDPARTY_INC}sqlite-3.8.8.1/ )
include_directories(${PATH_THIRDPARTY_INC}mysql-linux-5.6.22/ )
include_directories(${PATH_THIRDPARTY_INC}rapidxml-1.13/ )


add_subdirectory(commlib/zcelib)
add_subdirectory(commlib/soarlib)
add_subdirectory(test/zealot)

