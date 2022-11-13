# server ogre4asvrd

# add the executable
add_executable(ogre4asvrd "")

file(GLOB_RECURSE SOURCE_FILES ${CMAKE_SOURCE_DIR}/commsvr/ogre4asvrd/*.cpp)
file(GLOB_RECURSE HEADER_FILES ${CMAKE_SOURCE_DIR}/commsvr/ogre4asvrd/*.h)
set(All_FILES ${HEADER_FILES} ${SOURCE_FILES} )
target_sources(ogre4asvrd PRIVATE ${All_FILES})
target_include_directories(ogre4asvrd PUBLIC ${CMAKE_SOURCE_DIR}/commlib/zcelib/)
target_include_directories(ogre4asvrd PUBLIC ${CMAKE_SOURCE_DIR}/commlib/soarlib/)
target_include_directories(ogre4asvrd PUBLIC ${CMAKE_SOURCE_DIR}/commsvr/ogre4asvrd/)
add_dependencies(ogre4asvrd zcelib soarlib)
target_link_libraries(ogre4asvrd zcelib soarlib)
#target_precompile_headers(zealot PRIVATE zealot_predefine.h)
