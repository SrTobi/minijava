if(DEFINED ENV{LIBFIRM_GIT_URL})
	set(LIBFIRM_GIT_URL "$ENV{LIBFIRM_GIT_URL}")
else()
	set(LIBFIRM_GIT_URL "http://pp.ipd.kit.edu/git/libfirm.git")
endif()

function(import_libfirm target)
	include(ExternalProject)
	set(install_dir "${CMAKE_BINARY_DIR}/downloads/libFirm/install")
	ExternalProject_Add(
		libFirmBuilder
		GIT_REPOSITORY "${LIBFIRM_GIT_URL}"
		GIT_TAG "5a89d4b83cdf70286bfc338f98527ace8bbc4c60"
		PREFIX "${CMAKE_BINARY_DIR}/downloads/libFirm"
		# Skip revision.h generation, since it won't work on Windows
		PATCH_COMMAND "${CMAKE_COMMAND}" -E remove_directory "${CMAKE_BINARY_DIR}/downloads/libFirm/src/libFirmBuilder/.git"
		UPDATE_COMMAND ""
		CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${install_dir}"
	)
	add_library(${target} INTERFACE)
	add_dependencies(${target} libFirmBuilder)
	target_link_libraries(${target} INTERFACE "${install_dir}/lib/libfirm.a")
	if(UNIX)
		target_link_libraries(${target} INTERFACE m)
	elseif(WIN32)
		target_link_libraries(${target} INTERFACE gnurx winmm)
	endif()
	target_include_directories(${target} INTERFACE "${install_dir}/include")
endfunction(import_libfirm)
