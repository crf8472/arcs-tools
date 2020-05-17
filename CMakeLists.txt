## Root CMake file for arcs-tools

cmake_minimum_required (VERSION 3.9.4 )

## Set variables corresponding to unspecified versions to the empty string
## needed for specifying the project version along with the name in project()
cmake_policy (SET CMP0048 NEW )

## Support new IN_LIST if() operator
## required by FindDoxygen.cmake
cmake_policy (SET CMP0057 NEW )


project (arcs-tools VERSION 0.1.0 LANGUAGES CXX )

set (PROJECT_BRIEF_DESC "Example Toolkit for libarcstk" )

set (PROJECT_URL "https://codeberg.org/tristero/${PROJECT_NAME}/" )

set (PROJECT_VERSION_SUFFIX "alpha.1" )

set (PROJECT_DEFAULT_BUILD_TYPE "Release" )

set (CMAKE_CXX_STANDARD 17 )
set (CMAKE_CXX_STANDARD_REQUIRED ON )
set (CMAKE_CXX_EXTENSIONS OFF ) ## use -std=c++11 instead of std=gnu++11


## --- Setup Version information

set (PROJECT_VERSION_STRING_WITHOUT_SUFFIX
	"${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}"
	)

set (PROJECT_VERSION_STRING "${PROJECT_VERSION_STRING_WITHOUT_SUFFIX}" )

if (PROJECT_VERSION_SUFFIX)
	set(PROJECT_VERSION_STRING
		"${PROJECT_VERSION_STRING}-${PROJECT_VERSION_SUFFIX}" )
endif(PROJECT_VERSION_SUFFIX)

message (STATUS "Building ${PROJECT_NAME} ${PROJECT_VERSION_STRING}" )


## --- Setup Default Build Type

if (NOT CMAKE_BUILD_TYPE )

	set (CMAKE_BUILD_TYPE "${PROJECT_DEFAULT_BUILD_TYPE}"
		CACHE STRING "Choose the type of build (e.g. Debug or Release)" FORCE )

	message (STATUS "No build type specified, set to default" )

endif (NOT CMAKE_BUILD_TYPE )

message (STATUS "Build type: ${CMAKE_BUILD_TYPE}" )


## --- Project Directory Layout


## "Root directory for out-of-source build"
set (PROJECT_BINARY_DIR       "${CMAKE_CURRENT_SOURCE_DIR}/build" )

## "Root directory for sources"
set (PROJECT_SOURCE_DIR       "${CMAKE_CURRENT_SOURCE_DIR}/src" )

## "Root directory for testcases and testdata"
set (PROJECT_TEST_DIR         "${CMAKE_CURRENT_SOURCE_DIR}/test" )

## "Root directory for documentation config and sources" )
set (PROJECT_DOC_DIR          "${CMAKE_CURRENT_SOURCE_DIR}/doc" )

## "Root directory for manpages config and sources" )
set (PROJECT_MAN_DIR          "${CMAKE_CURRENT_SOURCE_DIR}/man" )

## "Target directory for non-cmake logs"
set (PROJECT_LOG_DIR          "${PROJECT_BINARY_DIR}/log" )

## "Target directory for generated sources and configuration"
set (PROJECT_BUILD_SOURCE_DIR "${PROJECT_BINARY_DIR}/generated-sources" )

## "Target directory for generated documentation"
set (PROJECT_BUILD_DOC_DIR    "${PROJECT_BINARY_DIR}/generated-docs" )


## Install directories

set (INSTALL_BINARY_DIR "${CMAKE_INSTALL_PREFIX}/bin"
	CACHE PATH "Installation directory for binaries" )

set (INSTALL_MAN_DIR    "${CMAKE_INSTALL_PREFIX}/man"
	CACHE PATH "Installation directory for man pages" )


## Tool informations

## Names
set (TOOL_NAME_CALC   arcstk-calc )
set (TOOL_NAME_ID     arcstk-id )
set (TOOL_NAME_PARSE  arcstk-parse )
set (TOOL_NAME_VERIFY arcstk-verify )

set (TOOL_NAMES ) ## Iterable tool names, used for manpage generation
list (APPEND TOOL_NAMES
	${TOOL_NAME_CALC}
	${TOOL_NAME_ID}
	${TOOL_NAME_PARSE}
	${TOOL_NAME_VERIFY} )


## Modules

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules" )
message (STATUS "Changed CMAKE_MODULE_PATH to: ${CMAKE_MODULE_PATH}" )



## --- Source files configured by CMake

configure_file (
  "${PROJECT_SOURCE_DIR}/version.hpp.in"
  "${PROJECT_BUILD_SOURCE_DIR}/version.hpp"
)



## --- Create Log directory

file (MAKE_DIRECTORY ${PROJECT_LOG_DIR} )



## --- Compiler specific settings

## Settings for g++ and clang

if (CMAKE_COMPILER_IS_GNUCXX
		OR CMAKE_CXX_COMPILER MATCHES ".*clang"
		OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" )

	## Warnings

	add_compile_options (-Wall -Wextra -Weffc++ -Wold-style-cast -Wpedantic )

	## Platform specific optimizations (default ON for Release, otherwise OFF)

	if (CMAKE_BUILD_TYPE STREQUAL "Release" )

		set (WITH_NATIVE ON CACHE BOOL
			"Build with platform specific optimization" FORCE )

	endif (CMAKE_BUILD_TYPE STREQUAL "Release" )

	if (WITH_NATIVE )

		message (STATUS "Build with platform specific optimization" )

		add_compile_options (-march=native -mtune=generic )
	else()

		message (STATUS "Build without platform specific optimization" )

	endif (WITH_NATIVE )

else()

	message (WARNING "No compile flags specified for the actual C++ compiler" )
	message (WARNING "Try a generic compile without project specific flags" )

endif()


## --- Mandatory dependencies

## Commented out: coming with 3.14.5 ?
#set (CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE )

find_package (libarcstk 0.1.0 REQUIRED )

find_package (libarcsdec 0.1.0 REQUIRED )


## --- Create objects

## -- Targets for All Objects

set (REQUIRED_OBJECTS
	application
	app-calc
	app-id
	app-parse
	app-verify
	printers
	parsehandlers
	cliparse
	config
	format
	options
	tools_calc
	tools_fs )

foreach (_object IN LISTS REQUIRED_OBJECTS )

	add_library (${_object} OBJECT ${PROJECT_SOURCE_DIR}/${_object}.cpp )
endforeach()


## -- Add binary tree to find cmake-generated files like version.hpp

target_include_directories (application PRIVATE ${PROJECT_BUILD_SOURCE_DIR} )


## -- Objects dependent on libarcstk (all app-objects depend on libarcstk)

## Compatibility: set libarcstk_INCLUDE_DIRS from imported target if possible
if (TARGET libarcstk::libarcstk )

	get_target_property (libarcstk_INCLUDE_DIRS libarcstk::libarcstk
		INTERFACE_INCLUDE_DIRECTORIES )
endif ()

if (libarcstk_INCLUDE_DIRS )

	set (OBJECTS_DEPENDING_ON_LIBARCSTK
		application
		printers
		parsehandlers
		tools_calc
		config )

	foreach (_object IN LISTS OBJECTS_DEPENDING_ON_LIBARCSTK )

		target_include_directories (${_object}
			PRIVATE ${libarcstk_INCLUDE_DIRS} ) ## used from imported target
	endforeach()
endif()


## -- Objects dependent on libarcsdec (not a dependency of non-apps)

## Compatibility: set libarcsdec_INCLUDE_DIRS from imported target if possible
if (TARGET libarcsdec::libarcsdec )

	get_target_property (libarcsdec_INCLUDE_DIRS libarcsdec::libarcsdec
		INTERFACE_INCLUDE_DIRECTORIES )
endif ()

if (libarcsdec_INCLUDE_DIRS )

	set (OBJECTS_DEPENDING_ON_LIBARCSDEC
		app-calc
		app-id
		app-verify
		printers
		tools_calc )

	foreach (_object IN LISTS OBJECTS_DEPENDING_ON_LIBARCSDEC )

		target_include_directories (${_object}
			PRIVATE ${libarcsdec_INCLUDE_DIRS} ) ## used from imported target
	endforeach()
endif()


## -- Add executables to this list

set (PROJECT_EXECUTABLES )


## calc-tool with specific dependencies

add_executable (${TOOL_NAME_CALC}
	${PROJECT_SOURCE_DIR}/main-calc.cpp
	$<TARGET_OBJECTS:tools_calc>
	$<TARGET_OBJECTS:app-calc> )

target_link_libraries (${TOOL_NAME_CALC} PRIVATE libarcsdec::libarcsdec )
list (APPEND PROJECT_EXECUTABLES ${TOOL_NAME_CALC} )


## id-tool with specific dependencies

add_executable (${TOOL_NAME_ID}
	${PROJECT_SOURCE_DIR}/main-id.cpp
	$<TARGET_OBJECTS:app-id> )

target_link_libraries (${TOOL_NAME_ID} PRIVATE libarcsdec::libarcsdec )
list (APPEND PROJECT_EXECUTABLES ${TOOL_NAME_ID} )


## parse-tool with specific dependencies

add_executable (${TOOL_NAME_PARSE}
	${PROJECT_SOURCE_DIR}/main-parse.cpp
	$<TARGET_OBJECTS:parsehandlers>
	$<TARGET_OBJECTS:app-parse> )

target_link_libraries (${TOOL_NAME_PARSE} PRIVATE libarcsdec::libarcsdec )
list (APPEND PROJECT_EXECUTABLES ${TOOL_NAME_PARSE} )


## verify-tool with specific dependencies

add_executable (${TOOL_NAME_VERIFY}
	${PROJECT_SOURCE_DIR}/main-verify.cpp
	$<TARGET_OBJECTS:tools_calc>
	$<TARGET_OBJECTS:app-calc>
	$<TARGET_OBJECTS:app-verify> )

target_link_libraries (${TOOL_NAME_VERIFY} PRIVATE libarcsdec::libarcsdec )
list (APPEND PROJECT_EXECUTABLES ${TOOL_NAME_VERIFY} )


## -- Configure executables with the settings common to all

foreach (_executable IN LISTS PROJECT_EXECUTABLES )

	# Add common object dependencies

	target_sources (${_executable}
		PRIVATE $<TARGET_OBJECTS:printers>
		PRIVATE $<TARGET_OBJECTS:application>
		PRIVATE $<TARGET_OBJECTS:format>
		PRIVATE $<TARGET_OBJECTS:cliparse>
		PRIVATE $<TARGET_OBJECTS:config>
		PRIVATE $<TARGET_OBJECTS:options>
		PRIVATE $<TARGET_OBJECTS:tools_fs>
	)

	# Add the binary tree to the search path for include files
	# for each target, so that we will find version.hpp

	target_include_directories (${_executable} PRIVATE ${PROJECT_BINARY_DIR} )

	target_link_libraries (${_executable} PRIVATE libarcstk::libarcstk )

endforeach()



## --- Optional: Build tests, using Catch2 testing framework (default: OFF)

option (WITH_TESTS "Build unit tests" OFF )

if (WITH_TESTS)

	message (STATUS "Build with tests" )

	enable_testing()

	## Add Catch2 test framework
	add_subdirectory (thirdparty/catch2 EXCLUDE_FROM_ALL )

	## Add tests
	add_subdirectory (${PROJECT_TEST_DIR} )

else ()

	message (STATUS "Build without tests" )
endif (WITH_TESTS)



## --- If on *nix, prepare man pages

if (UNIX )

	add_subdirectory (man )

endif(UNIX )



## --- Generate documentation

add_subdirectory (doc )



## --- Install executables

if (NOT SKIP_INSTALL_ALL )

	install (TARGETS ${PROJECT_EXECUTABLES}
		RUNTIME DESTINATION "${INSTALL_BINARY_DIR}" )

endif()

