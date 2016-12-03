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

add_custom_target(core-generate-sources
	DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/lexer/keyword_pearson.tpp"
)

add_dependencies(core core-generate-sources)
