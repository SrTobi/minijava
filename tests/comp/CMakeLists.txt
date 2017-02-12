set(minijava_exec "${PROJECT_BINARY_DIR}/src/minijava")
set(comptest_exec "${CMAKE_CURRENT_SOURCE_DIR}/driver.py")

add_dependencies(check minijava)

add_test(
	NAME comptest-lextest
	COMMAND "${PYTHON_EXECUTABLE}" "${comptest_exec}"
	        "--input=${CMAKE_CURRENT_SOURCE_DIR}/analyze/*.mj"
			"--define=LEXER"
	        --
			"${minijava_exec}" --lextest
)

add_test(
	NAME comptest-parsetest
	COMMAND "${PYTHON_EXECUTABLE}" "${comptest_exec}"
	        "--input=${CMAKE_CURRENT_SOURCE_DIR}/analyze/*.mj"
			"--define=LEXER,PARSER"
	        --
			"${minijava_exec}" --parsetest
)

add_test(
	NAME comptest-check-valid
	COMMAND "${PYTHON_EXECUTABLE}" "${comptest_exec}"
	        "--input=${CMAKE_CURRENT_SOURCE_DIR}/analyze/*.mj"
			"--define=LEXER,PARSER,CHECK"
	        --
			"${minijava_exec}" --check
)

add_test(
	NAME comptest-check-invalid
	COMMAND "${PYTHON_EXECUTABLE}" "${comptest_exec}"
	        "--input=${CMAKE_CURRENT_SOURCE_DIR}/analyze/*.mj"
			"--define=LEXER,PARSER,CHECK,BUG"
	        --
			"${minijava_exec}" --check
)

add_test(
		NAME comptest-dump-ir
		COMMAND "${PYTHON_EXECUTABLE}" "${comptest_exec}"
		"--input=${CMAKE_CURRENT_SOURCE_DIR}/execute/*.mj"
		"--define=LEXER,PARSER,CHECK,FIRM"
		--
		"${minijava_exec}" --dump-ir
)

if(NOT WIN32)
	add_test(
			NAME comptest-compile-firm
			COMMAND "${PYTHON_EXECUTABLE}" "${comptest_exec}"
			"--input=${CMAKE_CURRENT_SOURCE_DIR}/execute/*.mj"
			"--define=LEXER,PARSER,CHECK,FIRM"
			--execute
			--
			"${minijava_exec}" --compile-firm
	)

	add_test(
			NAME comptest-compile-firm-opt
			COMMAND "${PYTHON_EXECUTABLE}" "${comptest_exec}"
			"--input=${CMAKE_CURRENT_SOURCE_DIR}/execute/*.mj"
			"--define=LEXER,PARSER,CHECK,FIRM"
			--execute
			--
			"${minijava_exec}" -O3 --compile-firm
	)
endif()

add_test(
		NAME comptest-asm-noexec
		COMMAND "${PYTHON_EXECUTABLE}" "${comptest_exec}"
		"--input=${CMAKE_CURRENT_SOURCE_DIR}/analyze/*.mj"
		"--input=${CMAKE_CURRENT_SOURCE_DIR}/execute/*.mj"
		"--define=LEXER,PARSER,CHECK,FIRM,ASM"
		--
		"${minijava_exec}"
)

add_test(
		NAME comptest-asm
		COMMAND "${PYTHON_EXECUTABLE}" "${comptest_exec}"
		"--input=${CMAKE_CURRENT_SOURCE_DIR}/execute/*.mj"
		"--define=LEXER,PARSER,CHECK,FIRM,ASM"
		--execute
		--
		"${minijava_exec}"
)

add_test(
		NAME comptest-asm-opt
		COMMAND "${PYTHON_EXECUTABLE}" "${comptest_exec}"
		"--input=${CMAKE_CURRENT_SOURCE_DIR}/execute/*.mj"
		"--define=LEXER,PARSER,CHECK,FIRM,ASM"
		--execute
		--
		"${minijava_exec}" -O3
)
