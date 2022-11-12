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
message(STATUS "CMAKE_OSX_ARCHITECTURES: ${CMAKE_OSX_ARCHITECTURES}")

# C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
# using Clang
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
# using GCC
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
# using Intel C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
# using Visual Studio C++
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8 " )
endif()


# 工程外部的头文件
set(PATH_THIRDPARTY_INC "../third_party/include/")
include_directories(${PATH_THIRDPARTY_INC}lualib-5.4.3/)
include_directories(${PATH_THIRDPARTY_INC}protobuf-2.6.1/src/)
include_directories(${PATH_THIRDPARTY_INC}sqlite-3.8.8.1/) 
include_directories(${PATH_THIRDPARTY_INC}mysql-linux-5.6.22/)
include_directories(${PATH_THIRDPARTY_INC}rapidxml-1.13/)

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    add_definitions("-DDEBUG")
elseif (CMAKE_BUILD_TYPE MATCHES "Release")
    add_definitions("-DNDEBUG")
else()    
endif()


# 
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(PATH_THIRDPARTY_LIB "../third_party/lib/linux/")
    link_directories(${PATH_THIRDPARTY_LIB})
    link_libraries(libmysql libprotobuf liblua libsqlite)
elseif (CMAKE_SYSTEM_NAME MATCHES "Windows")
    if (CMAKE_BUILD_TYPE MATCHES "Debug")
        set(PATH_THIRDPARTY_LIB "../third_party/lib/win/debug")
    elseif (CMAKE_BUILD_TYPE MATCHES "Release")
        set(PATH_THIRDPARTY_LIB "../third_party/lib/win/release")
    endif()
    link_directories(${PATH_THIRDPARTY_LIB})
    link_libraries(libmysql libprotobuf lualib sqlite)
else()    
endif()

# 
if (CMAKE_BUILD_TYPE MATCHES "Debug")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../bin/debug/ )
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../bin/debug/ )
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../lib/debug/ )
elseif (CMAKE_BUILD_TYPE MATCHES "Release")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../bin/release/ )
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../bin/release/ )
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../lib/release/ )
else()    
endif()


#增加子目录
add_subdirectory(commlib/zcelib)
add_subdirectory(commlib/soarlib)
add_subdirectory(test/zealot)

