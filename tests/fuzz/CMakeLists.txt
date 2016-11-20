set(FUZZERS
	parsetest
	print-ast
)

foreach(fuzz ${FUZZERS})
	add_executable(${fuzz} ${fuzz}.cpp)
	target_include_directories(${fuzz} PRIVATE "${CMAKE_CURRENT_DIRECTORY}")
	target_link_libraries(${fuzz}
		LINK_PRIVATE core
		LINK_PRIVATE support
		LINK_PRIVATE ${Boost_PROGRAM_OPTIONS_LIBRARIES}
	)
	add_test(fuzztest-${fuzz} ${fuzz})
endforeach(fuzz)
