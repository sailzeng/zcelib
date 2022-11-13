# server wormholesvrd

# add the executable
add_executable(wormholesvrd "")

file(GLOB_RECURSE SOURCE_FILES ${CMAKE_SOURCE_DIR}/commsvr/wormholesvrd/*.cpp)
file(GLOB_RECURSE HEADER_FILES ${CMAKE_SOURCE_DIR}/commsvr/wormholesvrd/*.h)
set(All_FILES ${HEADER_FILES} ${SOURCE_FILES} )
target_sources(wormholesvrd PRIVATE ${All_FILES})
target_include_directories(wormholesvrd PUBLIC ${CMAKE_SOURCE_DIR}/commlib/zcelib/)
target_include_directories(wormholesvrd PUBLIC ${CMAKE_SOURCE_DIR}/commlib/soarlib/)
target_include_directories(wormholesvrd PUBLIC ${CMAKE_SOURCE_DIR}/commsvr/wormholesvrd/)
add_dependencies(wormholesvrd zcelib soarlib)
target_link_libraries(wormholesvrd zcelib soarlib)
#target_precompile_headers(zealot PRIVATE zealot_predefine.h)
