
#	buildx_export_header(	<target>
#							[DESTINATION_FILE <out-file>]
#							[DESTINATION_DIR <out-path>]
#							[SUB_DIR <sub-dir>]
#							[NAME <name>]
#
function(buildx_export_header _target)

	set(options)
	set(oneValueArgs DESTINATION_FILE DESTINATION_DIR NAME SUB_DIR)
	set(multiValueArgs)
	cmake_parse_arguments(_arg "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
	
	# default options
	set(out_base_path ${CMAKE_CURRENT_BINARY_DIR}/generated_includes)
	
	if(_arg_SUB_DIR)
		set(out_path ${out_base_path}/${_arg_SUB_DIR})
	endif()
	
	set(name ${_target})
	if(_arg_NAME)
		set(name ${_arg_NAME})
	endif()
	set(out_file ${out_path}/${name}_api.hpp)
	
	
	
	generate_export_header(	${_target}
							EXPORT_FILE_NAME ${out_file}
							BASE_NAME ${name}
							)
							
	if(_arg_DESTINATION_FILE)
		set(${_arg_DESTINATION_FILE} ${${_arg_DESTINATION_FILE}} ${out_file} PARENT_SCOPE)
	endif()
	
	if(_arg_DESTINATION_DIR)
		set(${_arg_DESTINATION_DIR} ${${_arg_DESTINATION_DIR}} ${out_base_path} PARENT_SCOPE)
	endif()
endfunction()


macro(buildx_add_library _target)

	string(TOUPPER "${_target}_BUILD_SHARED" shared_sym)
	option(${shared_sym} "Build ${_target} as shared library (.dll or .so)" ON)

	if(${${shared_sym}})
		add_library(${_target} SHARED ${ARGN})
	else()
		add_library(${_target} STATIC ${ARGN})
	endif()

endmacro()

macro(buildx_set_default_prefixes)
	set(CMAKE_DEBUG_POSTFIX "_d")
endmacro()