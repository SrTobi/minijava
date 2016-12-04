add_executable(huge_random_file huge_random_file.cpp)
target_link_libraries(huge_random_file
		LINK_PRIVATE ${Boost_PROGRAM_OPTIONS_LIBRARIES}
)

add_executable(many_different_ids many_different_ids.cpp)
target_link_libraries(many_different_ids
		LINK_PRIVATE core
		LINK_PRIVATE support
		LINK_PRIVATE ${Boost_PROGRAM_OPTIONS_LIBRARIES}
)

add_executable(syntaxgen syntaxgen.cpp)
target_link_libraries(syntaxgen
		LINK_PRIVATE core
		LINK_PRIVATE support
		LINK_PRIVATE ${Boost_PROGRAM_OPTIONS_LIBRARIES}
)

add_executable(astgen astgen.cpp)
target_link_libraries(astgen
		LINK_PRIVATE core
		LINK_PRIVATE support
		LINK_PRIVATE ${Boost_PROGRAM_OPTIONS_LIBRARIES}
)
