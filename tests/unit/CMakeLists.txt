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

foreach(comp ${COMPONENTS})
	# Replace `/` with a `.` so cmake accepts the target name
	string(REPLACE "/" "." comp-id ${comp})
	add_executable(${comp-id} ${comp}.cpp)
	target_include_directories(${comp-id} PRIVATE "${CMAKE_CURRENT_DIRECTORY}")
	if(NOT Boost_USE_STATIC_LIBS)
		target_compile_definitions(${comp-id} PRIVATE BOOST_TEST_DYN_LINK=1)
	endif()
	target_link_libraries(${comp-id}
		LINK_PRIVATE core
		LINK_PRIVATE support
		LINK_PRIVATE ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES}
	)
	add_test(unittest-${comp-id} ${comp-id})
	add_dependencies(check ${comp-id})
endforeach(comp)
