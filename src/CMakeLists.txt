add_library(convenience ${COMPONENTS})

target_include_directories(convenience PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(convenience)

add_executable(minijava minijava.cpp)
target_include_directories(minijava PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(minijava LINK_PRIVATE convenience)
