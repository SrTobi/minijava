function(import_libfirm target)
	include(ExternalProject)
	set(install_dir "${CMAKE_BINARY_DIR}/downloads/libFirm/install")
	ExternalProject_Add(
		libFirmBuilder
		GIT_REPOSITORY "http://pp.ipd.kit.edu/git/libfirm.git"
		#GIT_MODULE CMake
		PREFIX "${CMAKE_BINARY_DIR}/downloads/libFirm"
		UPDATE_COMMAND ""
		CMAKE_ARGS
			-DCMAKE_INSTALL_PREFIX=${install_dir}
		LOG_DOWNLOAD ON
		BUILD_IN_SOURCE ON
	)
	#ExternalProject_Get_Property(libFirmBuilder install_dir)

	add_library(${target} INTERFACE)
	target_link_libraries(${target} INTERFACE "${install_dir}/lib/libfirm.a")
	target_link_libraries(${target} INTERFACE "m")
	target_include_directories(${target} INTERFACE "${install_dir}/include")

	include_directories(SYSTEM "${install_dir}/include")

endfunction(import_libfirm)
