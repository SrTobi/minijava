set(RUNTIME_SOURCES
	allocate
	println
)

add_library(runtime
	${RUNTIME_SOURCES}
)

set(MINIJAVA_C_RUNTIME "")

foreach(source_file ${RUNTIME_SOURCES})
	file(READ "${source_file}.c" source_file_contents)
	string(CONCAT
			MINIJAVA_C_RUNTIME
			${MINIJAVA_C_RUNTIME} "\r\n" ${source_file_contents}
	)
endforeach()

configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/runtime.h.in"
	"${PROJECT_BINARY_DIR}/runtime.h"
)
