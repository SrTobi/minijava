# Welcome to the top-level `CMakeLists.txt` file!  Here is a quick
# overview of the physical layout of this project.
#
# The compiler is built from the single source file `src/minijava.cpp`
# which is statically linked against a convenience library `libcore`
# that provides the actual features.  The convenience library is made
# up of *components*.  A component is a triple of a header file,
# source file and unit test.  (This terminology is borrowed from John
# Lakos.)
#
#  Each component has a name.  Whenever you add a new component, say,
# `feature`, exercise the following steps.
#
#  1. Add `feature` to the definition of the variable `COMPONENTS`
#     further down below in this file.  Please put one name per line
#     and keep the list sorted.
#
#     We maintain this list manually instead of scanning the
#     file-system because this allows CMake to determine correctly
#     when the generated build-tools are outdated.  See the discussion
#     about `GLOB` here
#
#         https://cmake.org/cmake/help/v3.0/command/file.html
#
#     for some arguments.  In addition, it helps ensuring that each
#     component has indeed the three required files.
#
#	  You may also place your feature into a subdirectory.
#	  Put a `<dir>/` before the components name in the
#	  COMPONENTS variable.
#
#  2. Create the file `src/<dir>/feature.hpp` with the interface of your
#     component.  You don't have to put everything into a single
#     header file, as long as `src/feature.hpp` `#include`s all other
#     files.
#
#  3. Create the file `src/<dir>/feature.cpp` with the implementation of
#     your component.  It should `#include "feature.hpp"` as its first
#     dependency to make sure it compiles stand-alone.  Do this even
#     if the source file is otherwise empty.
#
#  4. Create the file `tests/<dir>/feature.cpp` with the unit-tests
#     for your component.  It will be linked against `libcore`.  In
#     this file, you should also `#include "<dir>/feature.hpp"`.
#
# Assuming you're using CMake with the Make backend, you can build the
# compiler via `make` and run all unit tests via `make test`.
#
# A grab bag of utility features that were considered useful for
# writing unit tests can be found in `tests/testaux/`.  If you add new
# compiled source files to it, don't forget to mention them in
# `tests/testaux/CMakeLists.txt`.

cmake_minimum_required(VERSION 3.0)

project(MiniJava VERSION 0.1)
enable_testing()

include("extras/cmake/activate_cpp14.cmake")
include("extras/cmake/enable_diagnostics.cmake")

buildx_activate_cpp14()
buildx_enable_diagnostics()

find_package(Boost REQUIRED COMPONENTS
	filesystem
	program_options
	system
	unit_test_framework
)

include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

set(COMPONENTS
	cli
	global
)

add_subdirectory("src")
add_subdirectory("tests")

configure_file(
	"${PROJECT_SOURCE_DIR}/config.h.in"
	"${PROJECT_BINARY_DIR}/config.h"
)
