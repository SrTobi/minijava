add_subdirectory("testaux")

foreach(comp ${COMPONENTS})
	# replace `/` with a `.` so cmake accepts the target name
	string(REPLACE "/" "." comp-id ${comp})
	set(comp-target test-${comp-id})
	add_test(${comp-id} ${comp-target})
	add_executable(${comp-target} ${comp}.cpp)
	target_include_directories(${comp-target} PRIVATE testaux)
	target_compile_definitions(${comp-target}
		PRIVATE $<$<NOT:$<BOOL:${Boost_USE_STATIC_LIBS}>>:BOOST_TEST_DYN_LINK=1>
		PRIVATE MINIJAVA_TEST
	)
	target_link_libraries(${comp-target}
		LINK_PRIVATE core
		LINK_PRIVATE testaux
		LINK_PRIVATE ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES}
	)
endforeach(comp)

# add a custom test target that outputs errors of failed test cases
add_custom_target(
	check
	COMMAND ${CMAKE_COMMAND} -E echo CWD=${CMAKE_BINARY_DIR}
	COMMAND ${CMAKE_COMMAND} -E echo CMD=${CMAKE_CTEST_COMMAND} -C $<CONFIG>
	COMMAND ${CMAKE_COMMAND} -E echo ----------------------------------
	COMMAND ${CMAKE_COMMAND} -E env CTEST_OUTPUT_ON_FAILURE=1
            ${CMAKE_CTEST_COMMAND} -C $<CONFIG> || true
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
