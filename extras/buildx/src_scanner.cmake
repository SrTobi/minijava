
function(buildx_scan _out _subdir _endings)
	
	buildx_debug("-------- begin buildx_scan --------" scan)
	buildx_debug("_out=${_out}" scan)
	buildx_debug("_subdir=${_subdir}" scan)
	buildx_debug("_endings=${_endings}" scan)
	buildx_debug("" scan)
	
	foreach(ending ${_endings})
		file(GLOB_RECURSE found_files "${_subdir}/*.${ending}")
		
		set(out ${out} ${found_files})
		buildx_debug("[${ending}]: ${found_files}")
	endforeach()
	set(${_out} ${${_out}} ${out} PARENT_SCOPE)
	
	buildx_debug("" scan)
	buildx_debug("${_out}+= ${out}" scan)
	buildx_debug("-------- end buildx_scan --------" scan)
endfunction()



function(buildx_scan_here _out _endings)
	buildx_scan(OUT ${CMAKE_CURRENT_SOURCE_DIR} "${_endings}")
	set(${_out} ${OUT} PARENT_SCOPE)
endfunction()


#	buildx_auto_group(files...
#						[BASE_PATH <base-path>]
#						[PREFIX <prefix>])
function(buildx_auto_group)
	
	set(options)
	set(oneValueArgs BASE_PATH PREFIX)
	set(multiValueArgs)
	cmake_parse_arguments(_arg "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
	
	
	set(_files ${_arg_UNPARSED_ARGUMENTS})
	set(_base_path "${CMAKE_CURRENT_SOURCE_DIR}/")
	if(_arg_BASE_PATH)
		set(_base_path "${_arg_BASE_PATH}/")
	endif()
	
	foreach(f ${_files})
		if(f)
			set(REPLACE_STRING ${_base_path})
			set(DONE "false")
			while(NOT ${DONE})
				string(FIND ${f} ${REPLACE_STRING} ERG)
				if(NOT ${ERG} EQUAL -1)
					set(DONE "true")
				else()
					get_filename_component(REPLACE_STRING ${REPLACE_STRING} PATH)
				endif()
			endwhile()
			
			string(REPLACE "${REPLACE_STRING}" "" striped ${f})
			get_filename_component(pathed ${striped} PATH)
			
			string(REPLACE "/" "\\" group_name "${_arg_PREFIX}/${pathed}")
			
			buildx_debug("${f} = ${group_name}" group)
			source_group("${group_name}" FILES "${f}")
		endif()
	endforeach()
endfunction()