set(MICRO_BENCHMARKS
	character
	keyword
	lexer
	parser
	semantic
	tts-combine
	tts-lookup
	tts-modify
)

foreach(bench ${MICRO_BENCHMARKS})
	add_executable(${bench} ${bench}.cpp)
	target_include_directories(${bench} PRIVATE "${CMAKE_CURRENT_DIRECTORY}")
	target_link_libraries(${bench}
		LINK_PRIVATE core
		LINK_PRIVATE support
	)
endforeach(bench)
