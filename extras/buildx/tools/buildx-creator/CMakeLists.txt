cmake_minimum_required(VERSION 2.8)
project(buildx-creator NONE)

macro(build_names name)
	string(TOUPPER ${name} upper_name)
	string(TOLOWER ${name} lower_name)
	set(prefix_name ${upper_name})
endmacro(build_names)

macro(add_project name)
	build_names(${name})
	
	option(${prefix_name}_use_precompiled_header "Use precompiled header" OFF)
	option(${prefix_name}_add_external_test "Add an external test" OFF)
	option(${prefix_name}_add_internal_test "Add an internal test" OFF)
	set(CREATOR_PROJECT_LIST ${CREATOR_PROJECT_LIST} ${name} CACHE INTERNAL "" FORCE)
	
endmacro(add_project)


	message("project '${CREATOR_add_project}'")

if(CREATOR_add_project)
	message("add project '${CREATOR_add_project}'")
	add_project(${CREATOR_add_project})
endif()

set(CREATOR_add_project "" CACHE STRING "add a new project to be created" FORCE)
option(CREATOR_buildx_directory "Relative path to buildx" "extras/buildx")
option(CREATOR_test_directory "Relative path to test directory" "tests")