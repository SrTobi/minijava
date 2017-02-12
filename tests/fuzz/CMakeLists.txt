set(FUZZERS
	parsetest
	print-ast
	generic
)

foreach(fuzz ${FUZZERS})
	add_executable(${fuzz} ${fuzz}.cpp)
	target_include_directories(${fuzz} PRIVATE "${CMAKE_CURRENT_DIRECTORY}")
	target_link_libraries(${fuzz}
		LINK_PRIVATE core
		LINK_PRIVATE support
		LINK_PRIVATE ${Boost_PROGRAM_OPTIONS_LIBRARIES}
	)
	add_dependencies(check ${fuzz})
endforeach(fuzz)

add_test(fuzztest-parsetest   parsetest)
add_test(fuzztest-print-ast   print-ast)
add_test(fuzztest-check       generic -r 50 -a check)
add_test(fuzztest-firm-create generic -r 50 -a firm-create)
add_test(fuzztest-firm-dump   generic -r 50 -a firm-dump)
if(NOT WIN32)
	add_test(fuzztest-firm-asm    generic -r 20 -a firm-asm)
	add_test(fuzztest-firm-link   generic -r 20 -a firm-link)
endif()
