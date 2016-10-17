


macro(buildx_enable_debug)

	set(BUILDX_DEBUG_CATS ${ARGV})
	message("# buildx_debug enabled for {${BUILDX_DEBUG_CATS}}")

endmacro(buildx_enable_debug)



function(buildx_debug _msg)

	if(DEFINED BUILDX_DEBUG_CATS)
	
		if(DEFINED ARGV1)
			list(FIND BUILDX_DEBUG_CATS "all" CAT_FOUND)
			
			# check if cat should be printed
			if(CAT_FOUND EQUAL "-1")
				list(FIND BUILDX_DEBUG_CATS ${ARGV1} CAT_FOUND)
			endif()
		endif()
	
		if(NOT CAT_FOUND EQUAL "-1")
		
			message("# ${_msg}")
		
		endif()
	
	endif()
	
endfunction()
