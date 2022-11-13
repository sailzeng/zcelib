# server zergsvrd

# add the executable
add_executable(zergsvrd "")

file(GLOB_RECURSE SOURCE_FILES ${CMAKE_SOURCE_DIR}/commsvr/zergsvrd/*.cpp)
file(GLOB_RECURSE HEADER_FILES ${CMAKE_SOURCE_DIR}/commsvr/zergsvrd/*.h)
set(All_FILES ${HEADER_FILES} ${SOURCE_FILES} )
target_sources(zergsvrd PRIVATE ${All_FILES})
target_include_directories(zergsvrd PUBLIC ${CMAKE_SOURCE_DIR}/commlib/zcelib/)
target_include_directories(zergsvrd PUBLIC ${CMAKE_SOURCE_DIR}/commlib/soarlib/)
target_include_directories(zergsvrd PUBLIC ${CMAKE_SOURCE_DIR}/commsvr/zergsvrd/)
add_dependencies(zergsvrd zcelib soarlib)
target_link_libraries(zergsvrd zcelib soarlib)
#target_precompile_headers(zealot PRIVATE zealot_predefine.h)
