foreach(comp ${COMPONENTS})
	add_test(${comp} test-${comp})
	add_executable(test-${comp} ${comp}.cpp)
	target_include_directories(test-${comp} PRIVATE testaux)
	target_compile_definitions(test-${comp}
		PRIVATE BOOST_TEST_DYN_LINK=1
		PRIVATE MINIJAVA_TEST)
	target_link_libraries(test-${comp}
		LINK_PRIVATE convenience
		LINK_PRIVATE ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES})
endforeach(comp)
