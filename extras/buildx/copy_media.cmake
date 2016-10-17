################### setup media copy support ###################


option(Option_COPY_MEDIA	"Copies the media files for samples and tests into the target directories" ON)



function(buildx_copy_media _target _resource)
	
	if(Option_COPY_MEDIA)
		set(res "${PROJECT_SOURCE_DIR}/${Project_MEDIA_DIR}/${_resource}")
		
		if(NOT EXISTS ${res})
			message(SEND_ERROR "[${_resource}] does not exist in media!")
		endif()
		
		if(IS_DIRECTORY ${res})
			set(copy_command  copy_directory)
		else()
			set(copy_command copy_if_different)
		endif()

		add_custom_command(	TARGET ${_target} POST_BUILD
							COMMAND ${CMAKE_COMMAND} -E ${copy_command}
							${res}
							$<TARGET_FILE_DIR:${_target}>)
	endif(Option_COPY_MEDIA)
endfunction()