# soarlib

add_library(soarlib STATIC "")

file(GLOB_RECURSE SOURCE_FILES ${CMAKE_SOURCE_DIR}/commlib/soarlib/*.cpp)
file(GLOB_RECURSE HEADER_FILES ${CMAKE_SOURCE_DIR}/commlib/soarlib/*.h)

set(All_FILES ${HEADER_FILES} ${SOURCE_FILES} )
target_sources(soarlib PRIVATE ${All_FILES})
target_include_directories(soarlib PUBLIC ${CMAKE_SOURCE_DIR}/commlib/zcelib/)
target_include_directories(soarlib PUBLIC ${CMAKE_SOURCE_DIR}/commlib/soarlib/)

add_dependencies(soarlib zcelib)
