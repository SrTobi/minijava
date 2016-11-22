set(minijava_exec "${PROJECT_BINARY_DIR}/src/minijava")
set(comptest_exec "${CMAKE_CURRENT_SOURCE_DIR}/driver.py")

add_dependencies(check minijava)

add_test(
	NAME comptest-lexical-valid
	COMMAND ${PYTHON_EXECUTABLE} ${comptest_exec}
	        "--input=${CMAKE_CURRENT_SOURCE_DIR}/lex/valid/*.mj"
	        --
			"${minijava_exec}" --lextest
)

add_test(
	NAME comptest-lexical-invalid
	COMMAND ${PYTHON_EXECUTABLE} ${comptest_exec}
	        "--input=${CMAKE_CURRENT_SOURCE_DIR}/lex/invalid/*.mj"
	        "--status=1"
	        "--message=error: "
	        --
			"${minijava_exec}" --lextest
)

add_test(
	NAME comptest-syntax-valid
	COMMAND ${PYTHON_EXECUTABLE} ${comptest_exec}
	        "--input=${CMAKE_CURRENT_SOURCE_DIR}/syn/valid/*.mj"
	        --
			"${minijava_exec}" --parsetest
)

add_test(
	NAME comptest-syntax-invalid
	COMMAND ${PYTHON_EXECUTABLE} ${comptest_exec}
	        "--input=${CMAKE_CURRENT_SOURCE_DIR}/syn/invalid/*.mj"
	        "--status=1"
	        "--message=error: "
	        --
			"${minijava_exec}" --parsetest
)

add_test(
	NAME comptest-semantic-valid
	COMMAND ${PYTHON_EXECUTABLE} ${comptest_exec}
	        "--input=${CMAKE_CURRENT_SOURCE_DIR}/sem/valid/*.mj"
	        --
			"${minijava_exec}" --check
)

add_test(
	NAME comptest-semantic-invalid
	COMMAND ${PYTHON_EXECUTABLE} ${comptest_exec}
	        "--input=${CMAKE_CURRENT_SOURCE_DIR}/sem/invalid/*.mj"
	        "--status=1"
	        "--message=error: "
			"--escape"
	        --
			"${minijava_exec}" --check
)

add_test(
	NAME comptest-semantic-invalid-noescape
	COMMAND ${PYTHON_EXECUTABLE} ${comptest_exec}
			"--input=${CMAKE_CURRENT_SOURCE_DIR}/sem/invalid_noescape/*.mj"
			"--status=1"
			"--message=error: "
			--
			"${minijava_exec}" --check
)
