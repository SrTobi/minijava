

include("debug.cmake")

# check needed buildx variables
if(NOT DEFINED PROJECT_NAME)
	message(FATAL_ERROR "Buildx: No project name specified!")
endif(NOT DEFINED PROJECT_NAME)

if(NOT PROJECT_VERSION)
	message(WARNING "Buildx: Please specify a version for the project!")
endif(NOT PROJECT_VERSION)

if(NOT DEFINED PROJECT_SHORTCUT)
	set(PROJECT_SHORTCUT ${PROJECT_NAME})
endif(NOT DEFINED PROJECT_SHORTCUT)

if(NOT DEFINED PROJECT_PREFIX)
	set(PROJECT_PREFIX ${PROJECT_SHORTCUT})
endif(NOT DEFINED PROJECT_PREFIX)

include("print_properties.cmake")
include("pch.cmake")
# include("copy_media.cmake")
include("src_scanner.cmake")
include("library.cmake")
include("install.cmake")
include("dependencies.cmake")
include("utils.cmake")
include("test.cmake")
include("copy_media.cmake")

macro(buildx_set_default_output_dirs)
	
	set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

endmacro()


macro(buildx_activate_cpp11)
	include(CheckCXXCompilerFlag)
	CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
	CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
	if(COMPILER_SUPPORTS_CXX11)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	elseif(COMPILER_SUPPORTS_CXX0X)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
	else()
			message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
	endif()
endmacro(buildx_activate_cpp11)


