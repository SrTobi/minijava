cmake_minimum_required(VERSION 3.0)

project(MiniJava VERSION 0.1)
enable_testing()

include(CheckIncludeFileCXX)
include("extras/cmake/activate_cpp14.cmake")
include("extras/cmake/enable_diagnostics.cmake")
include("extras/cmake/import_libfirm.cmake")

option(MAINTAINER_MODE "Enable diagnostic options for maintainers" ON)

buildx_activate_cpp14()

if(MAINTAINER_MODE)
	buildx_enable_diagnostics()
endif(MAINTAINER_MODE)

find_package(PythonInterp 3.4)

if(WIN32)
	set(Boost_USE_STATIC_LIBS ON)
endif()

# This list includes transitive dependencies, since older versions of CMake
# include a version of FindBoost.cmake which does not know version 1.62. Those
# versions will print warnings, which can safely be ignored.
find_package(Boost 1.61 REQUIRED COMPONENTS
	filesystem
	program_options
	system
	unit_test_framework
)

import_libfirm(libFirm)

check_include_file_cxx("sys/resource.h" MINIJAVA_HAVE_RLIMIT)

include_directories(SYSTEM ${Boost_INCLUDE_DIRS}) # mark as system due to https://svn.boost.org/trac/boost/ticket/12373
link_directories(${Boost_LIBRARY_DIRS})

set(COMPONENTS
	asm/asm
	asm/data
	asm/instruction
	asm/opcode
	asm/register
	asm/text
	cli
	exceptions
	global
	io/file_data
	io/file_output
	io/misc
	irg/global_firm_state
	irg/globals
	irg/irg
	irg/lowering
	irg/mangle
	irg/method_builder
	irg/type_builder
	lexer/character
	lexer/keyword
	lexer/lexer
	lexer/serializer
	lexer/token
	lexer/token_iterator
	lexer/token_type
	lexer/token_type_set
	opt/conditional
	opt/control_flow
	opt/opt
	opt/folding
	opt/inline
	opt/unroll
	opt/load_store
	opt/unused_method
	opt/unused_params
	parser/ast
	parser/ast_factory
	parser/ast_misc
	parser/for_each_node
	parser/parser
	parser/pretty_printer
	position
	runtime/host_cc
	runtime/runtime
	semantic/attribute
	semantic/constant
	semantic/name_type_analysis
	semantic/semantic
	semantic/semantic_error
	semantic/symbol_table
	semantic/thou_shalt_return
	semantic/type_info
	symbol/symbol
	symbol/symbol_anchor
	symbol/symbol_entry
	symbol/symbol_pool
	system/subprocess
	system/system
	util/meta
	util/raii
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
