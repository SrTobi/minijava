# add a custom test target that outputs errors of failed test cases
add_custom_target(
		check
		COMMAND ${CMAKE_COMMAND} -E echo CWD=${CMAKE_BINARY_DIR}
		COMMAND ${CMAKE_COMMAND} -E echo CMD=${CMAKE_CTEST_COMMAND} -C $<CONFIG>
		COMMAND ${CMAKE_COMMAND} -E echo ----------------------------------
		COMMAND ${CMAKE_COMMAND} -E env CTEST_OUTPUT_ON_FAILURE=1
		${CMAKE_CTEST_COMMAND} -C $<CONFIG>
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_subdirectory("unit")
add_subdirectory("fuzz")
add_subdirectory("comp")
