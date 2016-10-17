
# make given variables available in the parent scope
macro(buildx_global)

	foreach(var ${ARGV})
		set(${var} ${${var}} PARENT_SCOPE)
	endforeach()

endmacro()