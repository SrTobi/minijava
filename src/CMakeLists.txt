add_subdirectory("core")
add_subdirectory("support")

add_executable(minijava minijava.cpp)
target_include_directories(minijava PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(minijava LINK_PRIVATE core)
