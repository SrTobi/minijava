add_library(testaux temporary_file)

target_include_directories(testaux PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(testaux
	LINK_PUBLIC ${Boost_FILESYSTEM_LIBRARIES}
	LINK_PUBLIC ${Boost_SYSTEM_LIBRARIES}
)
