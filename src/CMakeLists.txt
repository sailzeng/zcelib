cmake_minimum_required(VERSION 3.16)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

# set the project name
project(zceproject CXX)

#config 是大小写敏感的
 

IF (CMAKE_BUILD_TYPE MATCHES "Debug" OR CMAKE_BUILD_TYPE MATCHES "None")
    MESSAGE(STATUS "CMAKE_BUILD_TYPE is Debug")
ELSEIF (CMAKE_BUILD_TYPE MATCHES "Release")
    MESSAGE(STATUS "CMAKE_BUILD_TYPE is Release")
ELSEIF (CMAKE_BUILD_TYPE MATCHES "RelWitchDebInfo")
    MESSAGE(STATUS "CMAKE_BUILD_TYPE is RelWitchDebInfo")
ELSEIF (CMAKE_BUILD_TYPE MATCHES "MinSizeRel")
    MESSAGE(STATUS "CMAKE_BUILD_TYPE is MinSizeRel")
ELSE ()
    MESSAGE(STATUS "unknown CMAKE_BUILD_TYPE = " ${CMAKE_BUILD_TYPE})
ENDIF ()


set(CMAKE_DEBUG_POSTFIX "_d") 
set(CMAKE_RELEASE_POSTFIX "_r") 

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../bin/ )
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../bin/ )
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../lib/ )
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/../lib/debug/ )
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_REALSE ${CMAKE_SOURCE_DIR}/../lib/realse/ )

# C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#Linux 默认使用的编译文件是c++，也就是g++
#set(CMAKE_C_COMPILER "gcc")
#set(CMAKE_CXX_COMPILER "g++")


#工程外部的头文件
set(PATH_THIRDPARTY_INC "../third_party/include/")
include_directories(${PATH_THIRDPARTY_INC}lualib-5.1.5/ )
include_directories(${PATH_THIRDPARTY_INC}protobuf-2.6.1/src/ )
include_directories(${PATH_THIRDPARTY_INC}sqlite-3.8.8.1/ )
include_directories(${PATH_THIRDPARTY_INC}mysql-linux-5.6.22/ )
include_directories(${PATH_THIRDPARTY_INC}rapidxml-1.13/ )

#编译选项
add_compile_options(-W -Wall -Wpointer-arith -Winline -Wunreachable-code -Wshadow -Wcast-align)
# if 里面的变量不需要加${},CMake 不严谨
#-Werror 是告警为错误,我消灭了一半服务器的告警,所以我有权要求你也这样做,
if(rogue EQUAL 1)
add_compile_options()
else()
add_compile_options(-Werror)
endif() 

# make VERBOSE = 1 显示所有编译命令

add_library(zcelib)
file(GLOB_RECURSE SRC_LIST "./commlib/zcelib/*.cpp")
target_sources(zcelib PRIVATE ${SRC_LIST})
target_include_directories(zcelib PUBLIC "./commlib/zcelib/")
target_precompile_headers(zcelib PRIVATE ./commlib/zcelib/zce/predefine.h)


# add the executable
add_executable(zealot)
file(GLOB_RECURSE SRC_LIST "./test/zealot/*.cpp")
target_sources(zealot PRIVATE ${SRC_LIST})
target_include_directories(zealot PUBLIC "./test/zealot/")
add_dependencies(zealot zcelib)
target_precompile_headers(zealot PRIVATE ./test/zealot/zealot_predefine.h)


add_library(soarlib)
file(GLOB_RECURSE SRC_LIST "./commlib/soarlib/*.cpp")
target_sources(soarlib PRIVATE ${SRC_LIST})
target_include_directories(soarlib PUBLIC "./commlib/soarlib/")
add_dependencies(soarlib zcelib)