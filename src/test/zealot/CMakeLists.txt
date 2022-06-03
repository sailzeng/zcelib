# test zealot

# add the executable
add_executable(zealot "")
file(GLOB_RECURSE ZEALOT_LIST ${CMAKE_SOURCE_DIR}/test/zealot/*.cpp)
target_sources(zealot PRIVATE ${ZEALOT_LIST})
target_include_directories(zealot PUBLIC ${CMAKE_SOURCE_DIR}/test/zealot/)
add_dependencies(zealot zcelib)
target_link_libraries(zealot zcelib)
target_precompile_headers(zealot PRIVATE ${CMAKE_SOURCE_DIR}/test/zealot/zealot_predefine.h)
