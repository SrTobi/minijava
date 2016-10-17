# includes
include(CMakePackageConfigHelpers)

# global settings
set(_BUILDX_DEFAULT_PGK_CONFIG "${CMAKE_CURRENT_LIST_DIR}/configs/default-pkg-config.cmake" PARENT_SCOPE)

#	buildx_version_dir(_out _package_name _version)
function(buildx_version_dir _out)
	
	set(package_name ${PROJECT_NAME})
	set(package_version ${PROJECT_VERSION})
	
	if(${ARGV1})
		set(package_name ${ARGV1})
	endif()
	
	if(${ARGV2})
		set(package_version ${ARGV2})
	endif()
	
	set(${_out} "${package_name}-${package_version}" PARENT_SCOPE)
endfunction()


#
#	buildx_install_package(	EXPORT <target>
#							
#							[PACKAGE_NAME <package-name>]
#							
#							[PACKAGE_CONFIG_FILE <config-file>]
#							[PACKAGE_VERSION_FILE <version-file>]
#							[PACKAGE_VERSION <version>]
#							[BUILD_EXPORT_DIR <dir>]
#							[COMPONENT <component>]
#							[NAMESPACE <namespace>]
#							[NO_VERSION_DIR])
function(buildx_install_package)

	set(options NO_VERSION_DIR)
	set(oneValueArgs EXPORT PACKAGE_NAME PACKAGE_CONFIG_FILE PACKAGE_VERSION_FILE PACKAGE_VERSION COMPONENT NAMESPACE BUILD_EXPORT_DIR)
	set(multiValueArgs)
	cmake_parse_arguments(_arg "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

	#set export_name
	if(NOT _arg_EXPORT)
		message(FATAL_ERROR "EXPORT must be specified!")
	else()
		set(export_name ${_arg_EXPORT})
	endif(NOT _arg_EXPORT)
	
	# set pack_name
	if(NOT _arg_PACKAGE_NAME)
		set(pack_name ${PROJECT_NAME})
	else()
		set(pack_name ${_arg_PACKAGE_NAME})
	endif(NOT _arg_PACKAGE_NAME)
	
	#set export_dir
	if(NOT _arg_BUILD_EXPORT_DIR)
		set(export_dir ${CMAKE_CURRENT_BINARY_DIR}/${pack_name})
	else()
		set(export_dir ${CMAKE_CURRENT_BINARY_DIR}/${_arg_BUILD_EXPORT_DIR})
	endif(NOT _arg_BUILD_EXPORT_DIR)
	
	# set install location of config files
	if(NOT NO_VERSION_DIR)
		buildx_version_dir(version_dirname ${pack_name} ${_arg_PACKAGE_VERSION})
		set(version_dirname ${version_dirname}/)
	endif()
	
	if(WIN32 AND NOT CYGWIN)
		set(config_install_dir cmake/)
	else()
		if(NOT version_dirname)
			set(config_install_dir share/${pack_name}cmake/)
		else()
			set(config_install_dir share/${version_dirname}cmake/)
		endif()
	endif()	
	
	# build the name and path of the config file
	string(TOLOWER "${pack_name}-config.cmake" config_file_name)
	set(config_file_path ${export_dir}/${config_file_name})
	
	# build the name and path of the version file
	string(TOLOWER "${pack_name}-config-version.cmake" version_file_name)
	set(version_file_path ${export_dir}/${version_file_name})
	
	# build the name and path of the target file
	string(TOLOWER "${pack_name}-targets.cmake" targets_file_name)
	set(targets_file_path ${export_dir}/${targets_file_name})
	
	# print some informations
	buildx_debug("Install package ${pack_name} (Export: ${export_name})" install)
	buildx_debug("(Export to '${export_dir}')" install)
	buildx_debug("  Config:  '<prefix>/${config_install_dir}${config_file_name}" install)
	buildx_debug("  Version: '<prefix>/${config_install_dir}${version_file_name}" install)
	buildx_debug("  Targets: '<prefix>/${config_install_dir}${targets_file_name}" install)
	
	# copy or create config file
	if(_arg_PACKAGE_CONFIG_FILE)
		configure_file(${_arg_PACKAGE_CONFIG_FILE} ${config_file_path} @ONLY)
	else()
		configure_file(${_BUILDX_DEFAULT_PGK_CONFIG} ${config_file_path} @ONLY)
	endif(_arg_PACKAGE_CONFIG_FILE)
	
	# copy or create version file
	if(_arg_PACKAGE_VERSION_FILE)
		configure_file(${_arg_PACKAGE_VERSION_FILE} ${config_file_path} @ONLY)
	else()
		write_basic_package_version_file(${version_file_path} VERSION ${_arg_PACKAGE_VERSION} COMPATIBILITY AnyNewerVersion )
	endif(_arg_PACKAGE_VERSION_FILE)
	
	# export targets
	export(EXPORT ${export_name} FILE ${targets_file_path} NAMESPACE ${_arg_NAMESPACE})
	export(PACKAGE ${pack_name})

	install(EXPORT ${export_name} FILE ${targets_file_name} DESTINATION ${config_install_dir} NAMESPACE ${_arg_NAMESPACE} COMPONENT ${_arg_COMPONENT})
	install(FILES ${config_file_path} ${version_file_path} DESTINATION ${config_install_dir} COMPONENT ${_arg_COMPONENT})
	
endfunction(buildx_install_package)




#	buildx_target_includes(	TARGETS <target>
#							BASE_DIRECTORIES <dir>
#							DESTINATION <dir>
#							[FILES <headers>]
#							[CONFIGURATIONS <configurations>]
#							[COMPONENT <component>])
#
function(buildx_target_includes)

	set(options)
	set(oneValueArgs DESTINATION COMPONENT)
	set(multiValueArgs TARGETS CONFIGURATIONS FILES BASE_DIRECTORIES)
	cmake_parse_arguments(_arg "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

	if(NOT _arg_TARGETS)
		buildx_debug("No Targets specified!" install)
		return()
	endif(NOT _arg_TARGETS)
	
	if(NOT _arg_BASE_DIRECTORIES)
		buildx_debug("No base directories specified!" install)
		return()
	endif(NOT _arg_BASE_DIRECTORIES)
	
	if(NOT DEFINED _arg_DESTINATION)
		message(FATAL_ERROR "No include destination specified!")
	endif(NOT DEFINED _arg_DESTINATION)

	foreach(bdir ${_arg_BASE_DIRECTORIES})
		set(base_dirs ${base_dirs} ${bdir}/)
	endforeach()
	
	if(NOT DEFINED _arg_FILES)
		# install directory		
		install(DIRECTORY ${base_dirs}
				DESTINATION ${_arg_DESTINATION}
				CONFIGURATIONS ${_arg_CONFIGURATIONS}
				COMPONENT ${_arg_COMPONENT}
				PATTERN "scripts/*")
	else()
		foreach(header ${_arg_FILES})
			get_filename_component(hdir "${header}" DIRECTORY)
			foreach(bdir ${_arg_BASE_DIRECTORIES})
				string(FIND ${hdir} ${bdir} fres)
				if(${fres} EQUAL 0)
					#found
					string(LENGTH "${bdir}" dlength)
					string(SUBSTRING ${hdir} ${dlength} -1 incl_dir)
					buildx_debug("Install include directory: '${incl_dir}'" install)
					install(FILES ${header}
							DESTINATION ${_arg_DESTINATION}/${incl_dir}
							CONFIGURATIONS ${_arg_CONFIGURATIONS}
							COMPONENT ${_arg_COMPONENT})
					
					break()
				endif(${fres} EQUAL 0)
			endforeach()
			
		endforeach()
	endif()
	
	# add paths to targets
	foreach(trg ${_arg_TARGETS})
		buildx_debug("Include directories for Target ${trg}" install)
		foreach(bdir ${_arg_BASE_DIRECTORIES})
			buildx_debug("  Build:   '${bdir}'" install)
			target_include_directories(${trg} PUBLIC $<BUILD_INTERFACE:${bdir}>)
		endforeach()
		
		buildx_debug("  Install: '<prefix>/${_arg_DESTINATION}'" install)
		target_include_directories(${trg} PUBLIC $<INSTALL_INTERFACE:${_arg_DESTINATION}>)
	endforeach()
	
	#set_target_properties(<lib> PROPERTIES
	#  PUBLIC_HEADER "${PUBLIC_INCLUDES}")
	 

endfunction(buildx_target_includes)