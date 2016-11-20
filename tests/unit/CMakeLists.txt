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
