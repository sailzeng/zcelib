# zcelib

add_library(zcelib STATIC "")
file(GLOB_RECURSE SRC_LIST ${CMAKE_SOURCE_DIR}/commlib/zcelib/*.cpp)
#message(STATUS "SRC_LIST: ${SRC_LIST}")
target_sources(zcelib PRIVATE ${SRC_LIST} )
target_include_directories(zcelib PUBLIC ${CMAKE_SOURCE_DIR}/commlib/zcelib/)
target_precompile_headers(zcelib PUBLIC ${CMAKE_SOURCE_DIR}/commlib/zcelib/zce/predefine.h)