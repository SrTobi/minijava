

function(buildx_copy_dependency _target _to)

	buildx_debug("Examining target '${_target}'" dep)
	if(NOT TARGET ${_target})
		message(FATAL_ERROR "'${_target}' is not a target yet! Forgott to import?")
	endif()

	get_property(type TARGET ${_target} PROPERTY TYPE)
	get_property(imported TARGET ${_target} PROPERTY IMPORTED)
	if(NOT ${type} STREQUAL "STATIC_LIBRARY" AND ${imported})
		buildx_debug("Copy dynamic libraries for target '${_target}' to '${_to}'" dep)
		add_custom_command(	TARGET ${_to} POST_BUILD
							COMMAND ${CMAKE_COMMAND} -E
							copy_if_different $<TARGET_PROPERTY:${_target},IMPORTED_LOCATION_$<UPPER_CASE:$<CONFIG>>> $<TARGET_FILE_DIR:${_to}>)
		buildx_print_target_properties(${_target})
	endif()

	# recursive
	get_property(linkl TARGET ${_target} PROPERTY INTERFACE_LINK_LIBRARIES)
	foreach(li ${linkl})
		if(TARGET ${li})
			buildx_copy_dependency("${li}" "${_to}")
		endif()
	endforeach()
endfunction()

function(buildx_copy_target_dependencies _target)

	buildx_debug("Examining target '${_target}'" dep)
	get_property(linkl TARGET ${_target} PROPERTY LINK_LIBRARIES)
	foreach(li ${linkl})
		
		buildx_copy_dependency("${li}" "${_target}")
	endforeach()
endfunction()