

macro(buildx_doc _config)

	set(Option_DEFAULT_MAKE_DOXYGEN_TARGET ON)
	if(MSVC)
		set(Option_DEFAULT_MAKE_DOXYGEN_TARGET OFF)
	endif(MSVC)
	
	
	# create option
	option(Option_MAKE_DOXYGEN_TARGET	"Create a doxygen build target" ${Option_DEFAULT_MAKE_DOXYGEN_TARGET})

	
	
	if(Option_MAKE_DOXYGEN_TARGET)	
		# check config file
		if(NOT IS_DIRECTORY ${_config})
			message(FATAL_ERROR "${config} does not exist!")
		endif()
		
		if(NOT DOXYGEN_FOUND)
			#find doxygen
			find_package(Doxygen REQUIRED)
		endif(NOT DOXYGEN_FOUND)
		
		if(DOXYGEN_FOUND)
			#configure_file(${_config} ${CMAKE_CURRENT_BINARY_DIR}/${_config} @ONLY)
			add_custom_target(documentation ${DOXYGEN_EXECUTABLE} ${_config}
				WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
				COMMENT "Generating API documentation with Doxygen" VERBATIM
			)
		elseif(DOXYGEN_FOUND)
			message(SEND_ERROR "$Doxygen was not found!")			
		endif(DOXYGEN_FOUND)
	endif(Option_MAKE_DOXYGEN_TARGET)





endmacro(buildx_doc _config)