# make VERBOSE = 1 显示所有编译命令

cmake_minimum_required(VERSION 3.16)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithAssert" CACHE STRING "" FORCE)
if(CMAKE_GENERATOR MATCHES "Ninja Multi-Config")
  set(CMAKE_DEFAULT_BUILD_TYPE Debug)
endif()
set(CMAKE_BUILD_TYPE "Debug"  CACHE STRING "" FORCE)
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithAssert")

#Linux 默认使用的编译文件是c++，也就是g++
#set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

# set the project name
project(zceproject CXX)

#config 是大小写敏感的

IF (NOT CMAKE_BUILD_TYPE)
    MESSAGE(STATUS "CMAKE_BUILD_TYPE is None")
ELSEIF (CMAKE_BUILD_TYPE MATCHES "Debug" )
    MESSAGE(STATUS "CMAKE_BUILD_TYPE is Debug")
ELSEIF (CMAKE_BUILD_TYPE MATCHES "Release")
    MESSAGE(STATUS "CMAKE_BUILD_TYPE is Release")
ELSE ()
    MESSAGE(FATAL_ERROR "unknown CMAKE_BUILD_TYPE=" ${CMAKE_BUILD_TYPE}",Please check it.")
ENDIF ()


set(CMAKE_DEBUG_POSTFIX "_d") 
set(CMAKE_RELEASE_POSTFIX "_r") 

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../bin/ )
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../bin/ )
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../lib/ )
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/../lib/debug/ )
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_REALSE ${CMAKE_SOURCE_DIR}/../lib/realse/ )


set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG}-DDEBUG )
set(CMAKE_CXX_FLAGS_REALSE ${CMAKE_CXX_FLAGS_REALSE}-DNDEBUG )

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

#默认都打开debuginfo,optimize，GCC这点和WINDOWS完全不一样

set(debuginfo "1" CACHE STRING "this is a comment of debuginfo")
set(optimize "1" CACHE STRING "this is a comment of optimize")
set(pipes "1" CACHE STRING "this is a comment of pipes")
set(threads "1" CACHE STRING "this is a comment of threads")
set(effcpp "0" CACHE BOOL "this is a comment of effcpp")


#编译选项,告警相关
add_compile_options(-W -Wall -Wpointer-arith -Winline -Wunreachable-code -Wshadow -Wcast-align)


# if 里面的变量不需要加${},CMake 不严谨
#-Werror 是告警为错误,我消灭了一半服务器的告警,所以我有权要求你也这样做,
if(rogue EQUAL 1)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror" )
endif() 

if (effcpp EQUAL 1)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weffc++" )
endif()

#-pipe用于加快编译速度，如果pipes被设置1打开-pipe,
#我自己的测试-pipe加快不了多少速度
if (pipes EQUAL 1)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe" )
endif()

#-ggdb这个调试开关打开后调试信息应该好于-g,但是不能使用其他调试器调试了
if (debuginfo EQUAL 1)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb")
endif()

if (optimize EQUAL 1)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2" )
endif()



add_library(zcelib)
file(GLOB_RECURSE SRC_LIST "./commlib/zcelib/*.cpp")
target_sources(zcelib PRIVATE ${SRC_LIST})
target_include_directories(zcelib PUBLIC "./commlib/zcelib/")
target_precompile_headers(zcelib PUBLIC ./commlib/zcelib/zce/predefine.h)

include_directories(./commlib/zcelib/)

add_library(soarlib)
file(GLOB_RECURSE SRC_LIST "./commlib/soarlib/*.cpp")
target_sources(soarlib PRIVATE ${SRC_LIST})
target_include_directories(soarlib PUBLIC "./commlib/soarlib/")
add_dependencies(soarlib zcelib)

include_directories(./commlib/soarlib/)

# add the executable
add_executable(zealot)
file(GLOB_RECURSE SRC_LIST "./test/zealot/*.cpp")
target_sources(zealot PRIVATE ${SRC_LIST})
target_include_directories(zealot PUBLIC "./test/zealot/")
add_dependencies(zealot zcelib)
target_precompile_headers(zealot PRIVATE ./test/zealot/zealot_predefine.h)