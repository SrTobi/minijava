add_library(support
	testaux/ast_id_checker
	testaux/ast_test_factory
	testaux/benchmark
	testaux/temporary_file
)

target_include_directories(support PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(support
	LINK_PUBLIC core
	LINK_PUBLIC ${Boost_FILESYSTEM_LIBRARIES}
	LINK_PUBLIC ${Boost_SYSTEM_LIBRARIES}
)
