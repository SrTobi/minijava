# Setup include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Scan for sources
buildx_scan_here(MINIJAVA_SOURCE "cpp;hpp")
buildx_debug("source: ${MINIJAVA_SOURCE}" proj)
buildx_auto_group("${MINIJAVA_SOURCE}")

add_executable(minijava ${MINIJAVA_SOURCE})
target_link_libraries(minijava ${Boost_LIBRARIES})

# Add an internal test
buildx_add_internal_test(
	test-minijava ${PROJECT_SOURCE_DIR}/tests
	TEST_TARGET minijava
	DEFINITIONS MINIJAVA_TEST)
target_link_libraries(test-minijava ${Boost_LIBRARIES})
