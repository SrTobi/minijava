
set(_BUILDX_TESTX_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/testx/include" PARENT_SCOPE)
set(_BUILDX_TESTX_MODULE_CONFIG_FILE "${CMAKE_CURRENT_LIST_DIR}/configs/test-module-config.cpp" PARENT_SCOPE)

function(buildx_add_boost_test_framework _target_name)
	set(Boost_USE_STATIC_LIBS true)
	find_package(Boost COMPONENTS unit_test_framework REQUIRED)
	target_link_libraries(${_target_name} ${Boost_LIBRARIES})
	target_include_directories(${_target_name} PRIVATE ${Boost_INCLUDE_DIRS})

endfunction(buildx_add_boost_test_framework)


# buildx_add_external_test(	test_target
#							test_path
#							TEST_TARGETS targets
#							DEFINITIONS defs
#							)
macro(buildx_add_external_test _target_name _test_path)

	set(options)
	set(oneValueArgs)
	set(multiValueArgs TEST_TARGETS DEFINITIONS)
	cmake_parse_arguments(_arg "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

	# configure module file
	set(TESTX_MODULE_NAME ${_target_name})
	set(config_file_target ${CMAKE_CURRENT_BINARY_DIR}/${_target_name}-module.cpp)
	configure_file(${_BUILDX_TESTX_MODULE_CONFIG_FILE} ${config_file_target} @ONLY)
	
	set(_BUILDX_TMP_TEST_SOURCE "")
	buildx_scan(_BUILDX_TMP_TEST_SOURCE ${_test_path} "hpp;cpp")
	buildx_auto_group(${_BUILDX_TMP_TEST_SOURCE} BASE_PATH ${_test_path} PREFIX tests)	
	buildx_auto_group(${config_file_target} BASE_PATH ${CMAKE_CURRENT_BINARY_DIR} PREFIX test_module)	
	add_executable(${_target_name} ${_BUILDX_TMP_TEST_SOURCE} ${config_file_target})
	set_target_properties(${_target_name} PROPERTIES COMPILE_DEFINITIONS "TESTX_TEST")
	set_property(TARGET ${_target_name} APPEND_STRING PROPERTY COMPILE_DEFINITIONS "${_arg_DEFINITIONS}")
	target_link_libraries(${_target_name} ${_arg_TEST_TARGETS})
	target_include_directories(${_target_name} PRIVATE ${_BUILDX_TESTX_INCLUDE_DIR})
	buildx_add_boost_test_framework(${target_name})

endmacro(buildx_add_external_test)


# buildx_add_internal_test(	test_target
#							test_path
#							TEST_TARGET target
#							DEFINITIONS defs
#							)
macro(buildx_add_internal_test _target_name _test_path)

	set(options)
	set(oneValueArgs TEST_TARGET)
	set(multiValueArgs DEFINITIONS)
	cmake_parse_arguments(_arg "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

	if(NOT _arg_TEST_TARGET)
		message(FATAL_ERROR "TEST_TARGET not specified!")
	endif(NOT _arg_TEST_TARGET)
	
	
	# configure module file
	set(TESTX_MODULE_NAME ${_target_name})
	set(config_file_target ${CMAKE_CURRENT_BINARY_DIR}/${_target_name}-module.cpp)
	configure_file(${_BUILDX_TESTX_MODULE_CONFIG_FILE} ${config_file_target} @ONLY)
	
	# get target source
	get_target_property(_org_source ${_arg_TEST_TARGET} SOURCES)
	
	# search test source and build executable
	set(_BUILDX_TMP_TEST_SOURCE "")
	buildx_scan(_BUILDX_TMP_TEST_SOURCE ${_test_path} "hpp;cpp")
	buildx_auto_group(${_BUILDX_TMP_TEST_SOURCE} BASE_PATH ${_test_path} PREFIX tests)	
	buildx_auto_group(${config_file_target} BASE_PATH ${CMAKE_CURRENT_BINARY_DIR} PREFIX test_module)	
	add_executable(${_target_name} ${_BUILDX_TMP_TEST_SOURCE} ${config_file_target} ${_org_source})
	
	# get definitions
	get_target_property(_org_definitions ${_arg_TEST_TARGET} COMPILE_DEFINITIONS)
	if(_org_definitions)
		set_property(TARGET ${_target_name} PROPERTY  ${_org_definitions})
	endif()
	set_property(TARGET ${_target_name} APPEND_STRING PROPERTY COMPILE_DEFINITIONS "TESTX_TEST")
	set_property(TARGET ${_target_name} APPEND_STRING PROPERTY COMPILE_DEFINITIONS "${_arg_TEST_TARGET}_EXPORTS")
	set_property(TARGET ${_target_name} APPEND_STRING PROPERTY COMPILE_DEFINITIONS "${_arg_DEFINITIONS}")
	
	# get include dirs
	get_target_property(_org_include_dirs ${_arg_TEST_TARGET} INCLUDE_DIRECTORIES)
	
	target_include_directories(${_target_name} PRIVATE ${_BUILDX_TESTX_INCLUDE_DIR} ${_org_include_dirs})
	buildx_add_boost_test_framework(${_target_name})

endmacro(buildx_add_internal_test)
