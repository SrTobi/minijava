add_library(core ${COMPONENTS})

target_include_directories(core
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
	PUBLIC ${PROJECT_BINARY_DIR}
)

target_link_libraries(core
	LINK_PUBLIC ${Boost_PROGRAM_OPTIONS_LIBRARIES}
)

add_executable(minijava minijava.cpp)
target_include_directories(minijava PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(minijava LINK_PRIVATE core)
