function(import_libfirm target)
	include(ExternalProject)
	set(install_dir "${CMAKE_BINARY_DIR}/downloads/libFirm/install")
	ExternalProject_Add(
		libFirmBuilder
		GIT_REPOSITORY "http://pp.ipd.kit.edu/git/libfirm.git"
		GIT_TAG "5a89d4b83cdf70286bfc338f98527ace8bbc4c60"
		PREFIX "${CMAKE_BINARY_DIR}/downloads/libFirm"
		# skip revision.h generation, since it won't work on Windows
		PATCH_COMMAND "${CMAKE_COMMAND}" -E remove_directory "${CMAKE_BINARY_DIR}/downloads/libFirm/src/libFirmBuilder/.git"
		UPDATE_COMMAND ""
		CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${install_dir}"
	)

	add_library(${target} INTERFACE)
	add_dependencies(${target} libFirmBuilder)
	target_link_libraries(${target} INTERFACE "${install_dir}/lib/libfirm.a")
	target_include_directories(${target} INTERFACE "${install_dir}/include")
endfunction(import_libfirm)
