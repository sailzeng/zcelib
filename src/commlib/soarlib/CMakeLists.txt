# soarlib

add_library(soarlib STATIC "")
file(GLOB_RECURSE SOAR_LIST ${CMAKE_SOURCE_DIR}/commlib/soarlib/*.cpp)
#message(STATUS "SOAR_LIST: ${SOAR_LIST}")
target_sources(soarlib PRIVATE ${SOAR_LIST})
target_include_directories(soarlib PUBLIC ${CMAKE_SOURCE_DIR}/commlib/soarlib/)
target_precompile_headers(soarlib PUBLIC ${CMAKE_SOURCE_DIR}/commlib/soarlib/soar/predefine.h)