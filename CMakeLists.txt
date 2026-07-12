## Root CMake file for arcs-tools
## vim:fdm=marker

cmake_minimum_required (VERSION 3.15 )

project (arcstk VERSION 0.1.3 LANGUAGES CXX )

set (ARCSTK_VERSION_SUFFIX "alpha.2" )
set (ARCSTK_VERSION_STRING "${PROJECT_VERSION}-${ARCSTK_VERSION_SUFFIX}" )

set (PROJECT_DESCRIPTION "Example Toolkit for libarcstk" )

set (PROJECT_HOMEPAGE_URL "https://github.com/crf8472/${PROJECT_NAME}/" )

include (cmake/functions/Banner.cmake )
banner (BEGIN "Configure ${PROJECT_NAME} ${ARCSTK_VERSION_STRING}" )

## --- CMake Setup {{{1

set (CMAKE_EXPORT_COMPILE_COMMANDS ON ) ## required for clang-tidy and iwyu

## --- Setup Default Build Type {{{2

if (NOT CMAKE_BUILD_TYPE )

	set (CMAKE_BUILD_TYPE "Release"
		CACHE STRING "Choose the type of build (e.g. Debug or Release)" FORCE )

	message (STATUS "No build type specified, set to ${CMAKE_BUILD_TYPE}" )
endif (NOT CMAKE_BUILD_TYPE )

message (STATUS "Build type: ${CMAKE_BUILD_TYPE}" )
## 2}}}
## --- Modules {{{2

## Tell find_package to first try locating libarcstk via system-installed
## .config file instead of project's FindLibarcstk.cmake module
set (CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE ) ## since CMake 3.15

## Search path for cmake modules
message (STATUS "CMAKE_MODULE_PATH is currently ${CMAKE_MODULE_PATH}" )
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")
message (STATUS "CMAKE_MODULE_PATH is now ${CMAKE_MODULE_PATH}" )
## 2}}}
## 1}}}
## --- Mandatory includes {{{1
set (_arcstk_cmake_include_dir "${CMAKE_CURRENT_SOURCE_DIR}/cmake/functions" )

## Mandatory configuration
include ("${_arcstk_cmake_include_dir}/ProjectFilesAndDirectories.cmake" )
include ("${_arcstk_cmake_include_dir}/BuildTreeSetup.cmake" )
include ("${_arcstk_cmake_include_dir}/CompilerFlags.cmake" )
include ("${_arcstk_cmake_include_dir}/GitInfo.cmake" )
include ("${_arcstk_cmake_include_dir}/Dependencies.cmake" )
## 1}}}
## --- Setup Build Tree {{{1
arcstk_setup_build_tree()

## 1}}}
## --- Includes for optional development support {{{1

## Optional support for development
include  ("${_arcstk_cmake_include_dir}/ToolClangTidy.cmake" )
#include ("${_arcstk_cmake_include_dir}/ToolGraphViz.cmake" )
#include ("${_arcstk_cmake_include_dir}/ToolClangIncludeGraph.cmake" )

unset (_arcstk_cmake_include_dir )
## 1}}}
## --- Trigger Configuration Step Whenever Git Index Changes {{{1

set_property (DIRECTORY APPEND
	PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/.git/index" )

arcstk_git_get_version_string (ARCSTK_GIT_VERSION_STRING )

configure_file (${ARCSTK_SOURCE_DIR}/version.cpp.in
	${ARCSTK_GENSRC_BINARY_DIR}/version.cpp
	@ONLY )

## 1}}}
## --- Define Project Target {{{1

add_executable (${PROJECT_NAME} ${ARCSTK_SOURCE_DIR}/main.cpp )

## Use this for all and everything, intentionally not target_properties
#set (CMAKE_CXX_STANDARD          17  )
#set (CMAKE_CXX_STANDARD_REQUIRED ON  )
#set (CMAKE_CXX_EXTENSIONS        OFF ) ## use -std=c++17 instead of -std=gnu++17

set_target_properties (${PROJECT_NAME} PROPERTIES
	CXX_STANDARD          17
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS        OFF ## use -std=c++17 instead of std=gnu++17
	PREFIX                ""
	VERSION               "${PROJECT_VERSION}"
	SOVERSION             "${PROJECT_VERSION}" )

## -- target: objects {{{2

add_library (arcstk_objects OBJECT
	"${ARCSTK_SOURCE_DIR}/ansi.cpp"
	"${ARCSTK_SOURCE_DIR}/app-calc.cpp"
	"${ARCSTK_SOURCE_DIR}/app-id.cpp"
	"${ARCSTK_SOURCE_DIR}/app-parse.cpp"
	"${ARCSTK_SOURCE_DIR}/app-verify.cpp"
	"${ARCSTK_SOURCE_DIR}/application.cpp"
	"${ARCSTK_SOURCE_DIR}/appregistry.cpp"
	"${ARCSTK_SOURCE_DIR}/clitokens.cpp"
	"${ARCSTK_SOURCE_DIR}/config.cpp"
	"${ARCSTK_SOURCE_DIR}/layouts.cpp"
	"${ARCSTK_SOURCE_DIR}/table.cpp"
	"${ARCSTK_SOURCE_DIR}/tools-arid.cpp"
	"${ARCSTK_SOURCE_DIR}/tools-calc.cpp"
	"${ARCSTK_SOURCE_DIR}/tools-dbar.cpp"
	"${ARCSTK_SOURCE_DIR}/tools-fs.cpp"
	"${ARCSTK_SOURCE_DIR}/tools-info.cpp"
	"${ARCSTK_SOURCE_DIR}/tools-input.cpp"
	"${ARCSTK_SOURCE_DIR}/tools-table.cpp"
	"${ARCSTK_SOURCE_DIR}/result.cpp"
	"${ARCSTK_GENSRC_BINARY_DIR}/version.cpp"
)

target_include_directories (arcstk_objects
	PRIVATE $<BUILD_INTERFACE:${ARCSTK_SOURCE_DIR}> )

set_target_properties (arcstk_objects PROPERTIES
	CXX_STANDARD          17
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS        OFF ## use -std=c++17 instead of std=gnu++17
)

## 2}}}

target_link_libraries (${PROJECT_NAME} PRIVATE arcstk_objects )

## 1}}}
## --- Compiler Specific Settings {{{1

arcstk_apply_compiler_flags (arcstk_objects )
arcstk_apply_compiler_flags (${PROJECT_NAME} )

## 1}}}

get_target_property (ARCSTK_CXX_FLAGS ${PROJECT_NAME} COMPILE_OPTIONS )
message (STATUS "Compile flags: ${ARCSTK_CXX_FLAGS}" )

## --- Setup required dependencies {{{1

option (WITH_SUBMODULES "Compile libarcstk and libarcsdec as submodules" OFF )

arcstk_setup_required_dependencies ()
## 1}}}
## --- Option: WITH_TESTS {{{1

option (WITH_TESTS "Build unit tests" OFF )

if (WITH_TESTS )

	message (STATUS "Build with tests" )

	enable_testing()

	## Add Catch2 test framework if it is not already present
	## (i.e. if PARENT did not include it)
	if (NOT TARGET Catch2::Catch2 )
		add_subdirectory (thirdparty/catch2 EXCLUDE_FROM_ALL )
	else()
		message (WARNING "Catch2 skipped since target already exists" )
	endif()

	## Add tests
	add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/test" )

endif (WITH_TESTS )
## 1}}}
## --- Installation {{{1

set (TOOL_NAMES ) ## tool names, used for installation + manpage generation
list (APPEND TOOL_NAMES
	${PROJECT_NAME}-calc
	${PROJECT_NAME}-id
	${PROJECT_NAME}-parse
	${PROJECT_NAME}-verify )


if (NOT SKIP_INSTALL_ALL )

	set (INSTALL_BINARY_DIR "${CMAKE_INSTALL_PREFIX}/bin"
		CACHE PATH "Installation directory for binaries" )

	install (TARGETS ${PROJECT_NAME}
		RUNTIME DESTINATION "${INSTALL_BINARY_DIR}" )

	if (UNIX )

		# Install symlinks

		foreach (LINK IN LISTS TOOL_NAMES)
			message (STATUS "Create symlink ${LINK} in ${INSTALL_BINARY_DIR}" )
			install (CODE "execute_process( \
				COMMAND ${CMAKE_COMMAND} -E create_symlink \
					${INSTALL_BINARY_DIR}/${PROJECT_NAME}  \
					${INSTALL_BINARY_DIR}/${LINK})" )
		endforeach()

	endif(UNIX )

endif()
## 1}}}
## --- Optional: Build documentation {{{1

option (WITH_DOCS          "Build documentation for public API"           OFF )
option (USE_DOC_TOOL       "Build documentation using specified tool"     OFF )

## -- Process configuration for building the documentation
if (WITH_DOCS )

	add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/doc )
else ()

	if (USE_DOC_TOOL )
		message (FATAL_ERROR "-DWITH_DOCS=ON is required to set USE_DOC_TOOL." )
	endif()
endif (WITH_DOCS )
## 1}}}
## --- Setup man pages (UNIX only) {{{1
option (WITH_MANPAGES      "Build Unix manpages"                          ON )

if (WITH_MANPAGES )

	if (UNIX )
		message (STATUS "Build with manpages" )
		add_subdirectory (man )
	else ()
		message (WARNING "Option WITH_MANPAGES is only available on unixoids" )
	endif (UNIX )

else()
	message (STATUS "Build without manpages" )
endif ()
## 1}}}
## --- Setup development tools {{{1
arcstk_enable_clang_tidy               (ARCSTK_CLANG_TIDY_ENABLED )
#enable_target_dep_graph_by_dot (ARCSTK_TARGET_DEPS_GRAPH_ENABLED )
#enable_clang_include_graph     (ARCSTK_CLANG_INCLUDE_GRAPH_ENABLED )
## 1}}}

banner (END
	"Configuration completed: ${PROJECT_NAME} ${ARCSTK_VERSION_STRING}" )

