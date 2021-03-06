add_library(core ${COMPONENTS})

target_include_directories(core
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
	PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
	PUBLIC ${PROJECT_BINARY_DIR}
)

target_link_libraries(core
	LINK_PUBLIC ${Boost_PROGRAM_OPTIONS_LIBRARIES}
	LINK_PUBLIC libFirm
)

add_custom_command(
	OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/lexer/keyword_pearson.tpp"
	COMMAND "${CMAKE_COMMAND}" -E make_directory lexer
	COMMAND "${PYTHON_EXECUTABLE}"
	        "${PROJECT_SOURCE_DIR}/extras/gen/keyword-hash.py"
	        -o "lexer/keyword_pearson.tpp"
	DEPENDS "${PROJECT_SOURCE_DIR}/extras/gen/keyword-hash.py"
	WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)

add_custom_command(
	OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/asm/opcode.hpp"
	COMMAND "${CMAKE_COMMAND}" -E make_directory asm
	COMMAND "${PYTHON_EXECUTABLE}"
	        "${PROJECT_SOURCE_DIR}/extras/gen/x64-isa.py"
	        -o "asm/opcode.hpp"
	DEPENDS "${PROJECT_SOURCE_DIR}/extras/gen/x64-isa.py"
	WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)

add_custom_command(
	OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/runtime/runtime.tpp"
	COMMAND "${CMAKE_COMMAND}" -E make_directory runtime
	COMMAND "${PYTHON_EXECUTABLE}"
	        "${PROJECT_SOURCE_DIR}/extras/gen/blobdump.py"
	        --text
	        -o "runtime/runtime.tpp"
	        "${PROJECT_SOURCE_DIR}/src/runtime/mj_runtime.c"
	DEPENDS "${PROJECT_SOURCE_DIR}/extras/gen/blobdump.py"
	DEPENDS "${PROJECT_SOURCE_DIR}/src/runtime/mj_runtime.c"
	WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)

add_custom_target(core-generate-sources
	DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/lexer/keyword_pearson.tpp"
	DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/asm/opcode.hpp"
	DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/runtime/runtime.tpp"
)

add_dependencies(core core-generate-sources)
