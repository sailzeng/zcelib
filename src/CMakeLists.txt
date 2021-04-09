cmake_minimum_required(VERSION 3.16)

# set the project name
project(zceproject CXX)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../lib)

# C++标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(PATH_THIRDPARTY_INC ../third_party/include)
include_directories(${PATH_THIRDPARTY_INC}/lualib-5.1.5 )
include_directories(${PATH_THIRDPARTY_INC}/protobuf-2.6.1/src )
include_directories(${PATH_THIRDPARTY_INC}/sqlite-3.8.8.1/src )
include_directories(${PATH_THIRDPARTY_INC}/mysql-linux-5.6.22 )
include_directories(${PATH_THIRDPARTY_INC}/rapidxml-1.13 )


add_library(zcelib)
file(GLOB_RECURSE SRC_LIST "./commlib/zcelib/*.cpp")
message(""${SRC_LIST})
target_sources(zcelib PRIVATE ${SRC_LIST})
target_include_directories(zcelib PUBLIC "./commlib/zcelib/")


add_library(soarlib)
file(GLOB_RECURSE SRC_LIST "./commlib/soarlib/*.cpp")
target_sources(soarlib PRIVATE ${SRC_LIST})
target_include_directories(soarlib PUBLIC "./commlib/soarlib/")

# add the executable
add_executable(zealot)
file(GLOB_RECURSE SRC_LIST "./test/zealot/*.cpp")
target_sources(zealot PRIVATE ${SRC_LIST})
target_include_directories(soarlib PUBLIC "./test/zealot/")