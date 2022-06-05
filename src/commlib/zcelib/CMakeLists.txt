# zcelib

add_library(zcelib STATIC "")

file(GLOB_RECURSE SOURCE_FILES ${CMAKE_SOURCE_DIR}/commlib/zcelib/*.cpp)
file(GLOB_RECURSE HEADER_FILES ${CMAKE_SOURCE_DIR}/commlib/zcelib/*.h)

set(All_FILES ${HEADER_FILES} ${SOURCE_FILES} )
target_sources(zcelib PRIVATE ${All_FILES} )
target_include_directories(zcelib PUBLIC ${CMAKE_SOURCE_DIR}/commlib/zcelib/)
