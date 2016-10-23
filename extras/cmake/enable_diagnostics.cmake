macro(buildx_enable_diagnostics)
	if(${CMAKE_C_COMPILER_ID} MATCHES "GNU|Clang")
		# -Wsign-conversion might be useful as well, but is too much of a PITA
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wconversion -Werror -pedantic")
	endif()
	if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU|Clang")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wconversion -Werror -pedantic")
	endif()
endmacro(buildx_enable_diagnostics)
