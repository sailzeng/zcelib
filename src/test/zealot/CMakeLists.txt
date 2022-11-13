# test zealot

# add the executable
add_executable(zealot "")

file(GLOB_RECURSE SOURCE_FILES ${CMAKE_SOURCE_DIR}/test/zealot/*.cpp)
file(GLOB_RECURSE HEADER_FILES ${CMAKE_SOURCE_DIR}/test/zealot/*.h)
set(All_FILES ${HEADER_FILES} ${SOURCE_FILES} )
target_sources(zealot PRIVATE ${All_FILES})
target_include_directories(zealot PUBLIC ${CMAKE_SOURCE_DIR}/commlib/zcelib/)
target_include_directories(zealot PUBLIC ${CMAKE_SOURCE_DIR}/test/zealot/)
add_dependencies(zealot zcelib)
target_link_libraries(zealot zcelib)
#target_precompile_headers(zealot PRIVATE zealot_predefine.h)
