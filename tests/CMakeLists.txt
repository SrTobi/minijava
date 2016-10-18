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
