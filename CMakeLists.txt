cmake_minimum_required(VERSION 3.0)

project(MiniJava VERSION 0.1)
enable_testing()

include(CheckIncludeFileCXX)
include("extras/cmake/activate_cpp14.cmake")
include("extras/cmake/enable_diagnostics.cmake")

buildx_activate_cpp14()
buildx_enable_diagnostics()

if(WIN32)
	set(Boost_USE_STATIC_LIBS ON)
endif()

find_package(Boost REQUIRED COMPONENTS
	filesystem
	program_options
	system
	unit_test_framework
)

check_include_file_cxx("sys/resource.h" MINIJAVA_HAVE_RLIMIT)

include_directories(SYSTEM ${Boost_INCLUDE_DIRS}) # mark as system due to https://svn.boost.org/trac/boost/ticket/12373
link_directories(${Boost_LIBRARY_DIRS})

set(COMPONENTS
	cli
	exceptions
	global
	lexer/character
	lexer/keyword
	lexer/lexer
	lexer/serializer
	lexer/token
	lexer/token_iterator
	lexer/token_type
	lexer/token_type_set
	parser/ast
	parser/ast_factory
	parser/for_each_node
	parser/parser
	parser/pretty_printer
	semantic/builtins
	semantic/constant
	semantic/ref_type_analysis
	semantic/semantic_error
	semantic/symbol_def
	semantic/thou_shalt_return
	semantic/type_system
	semantic/unique_entry_point
	symbol/symbol
	symbol/symbol_anchor
	symbol/symbol_entry
	symbol/symbol_pool
	system/system
)

add_subdirectory("src")
add_subdirectory("tests")
add_subdirectory("benchmarks")

configure_file(
	"${PROJECT_SOURCE_DIR}/config.h.in"
	"${PROJECT_BINARY_DIR}/config.h"
)

find_package(Doxygen QUIET)

if(DOXYGEN_FOUND)
	configure_file(
		${CMAKE_CURRENT_SOURCE_DIR}/doxygen.in
		${CMAKE_CURRENT_BINARY_DIR}/doxyfile @ONLY
	)
	add_custom_target(
		docs
		${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doxyfile
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Generating API documentation with Doxygen" VERBATIM
	)
else(DOXYGEN_FOUND)
	message(STATUS "Doxygen was not found. You may want to consider installing it to generate html docs for minijava.")
endif(DOXYGEN_FOUND)
