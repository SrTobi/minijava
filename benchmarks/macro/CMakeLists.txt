add_executable(huge_random_file huge_random_file.cpp)
target_link_libraries(huge_random_file
		LINK_PRIVATE ${Boost_PROGRAM_OPTIONS_LIBRARIES}
)

add_executable(many_different_ids many_different_ids.cpp)
target_link_libraries(many_different_ids
		LINK_PRIVATE ${Boost_PROGRAM_OPTIONS_LIBRARIES}
)
