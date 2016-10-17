cmake_minimum_required(VERSION 3.0)

project(MiniJava VERSION 0.1)
set(Project_SHORTCUT "cp")
set(Project_PREFIX "cp")
set(Project_MEDIA_DIR "media")

add_subdirectory("extras/buildx")
#buildx_enable_debug(all)
buildx_set_default_prefixes()
buildx_set_default_output_dirs()
buildx_activate_cpp14()

# find packages
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/extras/cmake/Modules")

if(WIN32)
	SET(Boost_USE_STATIC_LIBS ON)
endif()

find_package(Boost COMPONENTS program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

add_subdirectory("src")
