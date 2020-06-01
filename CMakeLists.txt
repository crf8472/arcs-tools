## Root CMake file for arcs-tools

cmake_minimum_required (VERSION 3.9.4 )

## Set variables corresponding to unspecified versions to the empty string
## needed for specifying the project version along with the name in project()
cmake_policy (SET CMP0048 NEW )

## Support new IN_LIST if() operator
## required by FindDoxygen.cmake
cmake_policy (SET CMP0057 NEW )


project (arcstk VERSION 0.1.0 LANGUAGES CXX )

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


## --- CMake Modules

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



## --- Optional: Add include-what-you-use support (default: OFF)

option (IWYU "Don't compile, check headers with include-what-you-use" OFF )
if (IWYU)

	message (STATUS "Program include-what-you-use requested" )
	find_program (IWYU_BINARY NAMES include-what-you-use iwyu )

	if (IWYU_BINARY )

		## Set for all subsequent targets
		set (CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${IWYU_BINARY} )

		## Commented out: Set for specific targets only
		#set_property(TARGET ${TARGET_NAME}
		#	PROPERTY CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${IWYU_PATH})

		message (STATUS "Program include-what-you-use found: ${IWYU_BINARY}" )

	else (IWYU_BINARY )

		message (FATAL_ERROR "Program include-what-you-use not found" )

	endif (IWYU_BINARY )
endif (IWYU)


## --- Mandatory dependencies

## Commented out: coming with 3.14.5 ?
#set (CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE )

find_package (libarcstk 0.1.0 REQUIRED )

find_package (libarcsdec 0.1.0 REQUIRED )



## --- Build

## All objects without name, used as dependency in tests
add_library ( ${PROJECT_NAME}-objects OBJECT
	${PROJECT_SOURCE_DIR}/app-calc.cpp
	${PROJECT_SOURCE_DIR}/app-id.cpp
	${PROJECT_SOURCE_DIR}/app-parse.cpp
	${PROJECT_SOURCE_DIR}/app-verify.cpp
	${PROJECT_SOURCE_DIR}/application.cpp
	${PROJECT_SOURCE_DIR}/apps.cpp
	${PROJECT_SOURCE_DIR}/clitokens.cpp
	${PROJECT_SOURCE_DIR}/config.cpp
	${PROJECT_SOURCE_DIR}/layouts.cpp
	${PROJECT_SOURCE_DIR}/options.cpp
	${PROJECT_SOURCE_DIR}/printers.cpp
	${PROJECT_SOURCE_DIR}/tools_calc.cpp
	${PROJECT_SOURCE_DIR}/tools_fs.cpp
	${PROJECT_SOURCE_DIR}/tools_parse.cpp
)
target_include_directories (${PROJECT_NAME}-objects
	PUBLIC ${PROJECT_BUILD_SOURCE_DIR} )


## The MAIN target

add_executable ( ${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/main.cpp )
target_link_libraries (${PROJECT_NAME} PRIVATE ${PROJECT_NAME}-objects )

## MAIN: add libarcstk and libarcsdec

if (TARGET libarcstk::libarcstk )

	get_target_property (libarcstk_INCLUDE_DIRS libarcstk::libarcstk
		INTERFACE_INCLUDE_DIRECTORIES )
endif ()

if (TARGET libarcsdec::libarcsdec )

	get_target_property (libarcsdec_INCLUDE_DIRS libarcsdec::libarcsdec
		INTERFACE_INCLUDE_DIRECTORIES )
endif ()

target_link_libraries (${PROJECT_NAME} PRIVATE libarcstk::libarcstk )
target_link_libraries (${PROJECT_NAME} PRIVATE libarcsdec::libarcsdec )



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



## --- Tool informations (for manpages and installing symlinks)

## Names
set (TOOL_NAME_ID     ${PROJECT_NAME}-id )
set (TOOL_NAME_CALC   ${PROJECT_NAME}-calc )
set (TOOL_NAME_PARSE  ${PROJECT_NAME}-parse )
set (TOOL_NAME_VERIFY ${PROJECT_NAME}-verify )

set (TOOL_NAMES ) ## Iterable tool names, used for manpage generation
list (APPEND TOOL_NAMES
	${TOOL_NAME_CALC}
	${TOOL_NAME_ID}
	${TOOL_NAME_PARSE}
	${TOOL_NAME_VERIFY} )



## --- If on *nix, prepare man pages

if (UNIX )

	add_subdirectory (man )

endif(UNIX )



## --- Generate documentation

add_subdirectory (doc )



## --- Install executables

if (NOT SKIP_INSTALL_ALL )

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

