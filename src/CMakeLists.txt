add_subdirectory("core")
add_subdirectory("support")

add_executable(minijava minijava.cpp)
target_include_directories(minijava PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(minijava LINK_PRIVATE
		core ${Boost_FILESYSTEM_LIBRARIES} ${Boost_SYSTEM_LIBRARIES})

add_executable(mj2c mj2c.cpp)
target_include_directories(mj2c PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mj2c LINK_PRIVATE core)
