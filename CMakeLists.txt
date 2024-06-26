## Root CMake file for arcs-tools

cmake_minimum_required (VERSION 3.15 )

## Set variables corresponding to unspecified versions to the empty string.
## Required for specifying the project version along with the name in project()
cmake_policy (SET CMP0048 NEW )


project (arcstk VERSION 0.1.1 LANGUAGES CXX )


## --- Setup Version Information

set (PROJECT_BRIEF_DESC "Example Toolkit for libarcstk" )

set (PROJECT_URL "https://github.com/crf8472/${PROJECT_NAME}/" )

set (PROJECT_VERSION_SUFFIX "alpha.1" )

set (PROJECT_DEFAULT_BUILD_TYPE "Release" )

set (PROJECT_VERSION_STRING_WITHOUT_SUFFIX
	"${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}"
	)

set (PROJECT_VERSION_STRING "${PROJECT_VERSION_STRING_WITHOUT_SUFFIX}" )

if (PROJECT_VERSION_SUFFIX )
	set (PROJECT_VERSION_STRING
		"${PROJECT_VERSION_STRING_WITHOUT_SUFFIX}-${PROJECT_VERSION_SUFFIX}" )
endif (PROJECT_VERSION_SUFFIX )

message (STATUS "Building ${PROJECT_NAME} ${PROJECT_VERSION_STRING}" )



## --- Setup Default Build Type

if (NOT CMAKE_BUILD_TYPE )

	set (CMAKE_BUILD_TYPE "${PROJECT_DEFAULT_BUILD_TYPE}"
		CACHE STRING "Choose the type of build (e.g. Debug or Release)" FORCE )

	message (STATUS "No build type specified, set to ${CMAKE_BUILD_TYPE}" )
endif (NOT CMAKE_BUILD_TYPE )

message (STATUS "Build type: ${CMAKE_BUILD_TYPE}" )



## --- Global settings

## Trigger configuration phase whenever git index changes
set_property (DIRECTORY APPEND
	PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/.git/index )

set (CMAKE_CXX_STANDARD 17 )
set (CMAKE_CXX_STANDARD_REQUIRED ON )
set (CMAKE_CXX_EXTENSIONS OFF ) ## use -std=c++17 instead of -std=gnu++17

## Tell find_package to first try locating libarcstk via system-installed
## .config file instead of project's FindLibarcstk.cmake module
set (CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE ) ## since CMake 3.15

## Search path for cmake modules
set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules" )
message (STATUS "Changed CMAKE_MODULE_PATH to: ${CMAKE_MODULE_PATH}" )



## --- Project Directory Layout


## "Root directory for sources"
set (PROJECT_SOURCE_DIR       "${CMAKE_CURRENT_SOURCE_DIR}/src" )

## "Root directory for public/exported headers"
set (PROJECT_LOCAL_INCLUDE_DIR "${PROJECT_SOURCE_DIR}" )

## "Root directory for out-of-source build"
set (PROJECT_BINARY_DIR       "${CMAKE_CURRENT_BINARY_DIR}" )

## "Target directory for non-cmake logs"
set (PROJECT_LOG_DIR          "${PROJECT_BINARY_DIR}/log" )

## "Target directory for generated sources and configuration"
set (PROJECT_BUILD_SOURCE_DIR "${PROJECT_BINARY_DIR}/generated-sources" )

set (PRIVATE_HEADERS
	${PROJECT_SOURCE_DIR}/ansi.hpp
	${PROJECT_SOURCE_DIR}/app-calc.hpp
	${PROJECT_SOURCE_DIR}/app-id.hpp
	${PROJECT_SOURCE_DIR}/app-parse.hpp
	${PROJECT_SOURCE_DIR}/app-verify.hpp
	${PROJECT_SOURCE_DIR}/application.hpp
	${PROJECT_SOURCE_DIR}/appregistry.hpp
	${PROJECT_SOURCE_DIR}/clitokens.hpp
	${PROJECT_SOURCE_DIR}/config.hpp
	${PROJECT_SOURCE_DIR}/layouts.hpp
	${PROJECT_SOURCE_DIR}/table.hpp
	${PROJECT_SOURCE_DIR}/tools-arid.hpp
	${PROJECT_SOURCE_DIR}/tools-calc.hpp
	${PROJECT_SOURCE_DIR}/tools-dbar.hpp
	${PROJECT_SOURCE_DIR}/tools-fs.hpp
	${PROJECT_SOURCE_DIR}/tools-info.hpp
	${PROJECT_SOURCE_DIR}/tools-table.hpp
	${PROJECT_SOURCE_DIR}/result.hpp
	${PROJECT_SOURCE_DIR}/version.hpp
)


## --- Create Log directory

add_custom_command (
	OUTPUT  ${PROJECT_LOG_DIR}
	PRE_BUILD
	COMMAND ${CMAKE_COMMAND}
	ARGS    -E make_directory ${PROJECT_LOG_DIR} )

add_custom_target (arcstools_create_log_dir ALL
	COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_LOG_DIR} )



## --- Update version.cpp with info from git

find_package (Git QUIET REQUIRED )

## Run 'git describe'
execute_process (
	COMMAND ${GIT_EXECUTABLE} describe --always HEAD
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	RESULT_VARIABLE GIT_EXIT_CODE
	OUTPUT_VARIABLE GIT_VERSION_STRING
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

## Did running git work?
if (GIT_EXIT_CODE EQUAL 0 )
	message (STATUS "git describe --always HEAD: ${GIT_VERSION_STRING}" )
else()
	message (WARNING "Failed to run git describe --always HEAD" )
endif()

## Run 'git rev-parse'
execute_process (
	COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	RESULT_VARIABLE GIT_EXIT_CODE
	OUTPUT_VARIABLE GIT_COMMIT_ID
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

## Did running git work?
if (GIT_EXIT_CODE EQUAL 0 )
	message (STATUS "git rev-parse HEAD: ${GIT_COMMIT_ID}" )
else()
	message (WARNING "Failed to run git rev-parse HEAD" )
endif()

## This presupposes having CMAKE_CONFIGURE_DEPENDS set to .git/index
configure_file (${PROJECT_SOURCE_DIR}/version.cpp.in
	${PROJECT_BUILD_SOURCE_DIR}/version.cpp
	@ONLY )


## --- Add and configure project target

## All objects, used as dependency in tests as well as in the application
add_library (objects OBJECT
	${PROJECT_SOURCE_DIR}/ansi.cpp
	${PROJECT_SOURCE_DIR}/app-calc.cpp
	${PROJECT_SOURCE_DIR}/app-id.cpp
	${PROJECT_SOURCE_DIR}/app-parse.cpp
	${PROJECT_SOURCE_DIR}/app-verify.cpp
	${PROJECT_SOURCE_DIR}/application.cpp
	${PROJECT_SOURCE_DIR}/appregistry.cpp
	${PROJECT_SOURCE_DIR}/clitokens.cpp
	${PROJECT_SOURCE_DIR}/config.cpp
	${PROJECT_SOURCE_DIR}/layouts.cpp
	${PROJECT_SOURCE_DIR}/table.cpp
	${PROJECT_SOURCE_DIR}/tools-arid.cpp
	${PROJECT_SOURCE_DIR}/tools-calc.cpp
	${PROJECT_SOURCE_DIR}/tools-dbar.cpp
	${PROJECT_SOURCE_DIR}/tools-fs.cpp
	${PROJECT_SOURCE_DIR}/tools-info.cpp
	${PROJECT_SOURCE_DIR}/tools-table.cpp
	${PROJECT_SOURCE_DIR}/result.cpp
	${PROJECT_BUILD_SOURCE_DIR}/version.cpp )

target_include_directories (objects PRIVATE ${PROJECT_SOURCE_DIR} )

## Actual application binary
add_executable (${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/main.cpp )
target_link_libraries (${PROJECT_NAME} PRIVATE objects )


## --- Dependencies with or without submodules

option (WITH_SUBMODULES "Compile libarcstk and libarcsdec as submodules" OFF )

if (WITH_SUBMODULES )

	message (STATUS "Link to local submodules libarcstk and libarcsdec" )

	## This signals libarcsdec's CMakeLists.txt where to find it's libarcstk
	## depencency.
	set (SUBMODULES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs" )

	add_subdirectory (${SUBMODULES_DIR}/libarcstk  )
	add_subdirectory (${SUBMODULES_DIR}/libarcsdec )

	## Commented out: try to force RPATH to build tree .so's but no success
	#set (CMAKE_SKIP_BUILD_RPATH            FALSE )
	#set (CMAKE_BUILD_WITH_INSTALL_RPATH    FALSE )
	#set (CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE  )
else()

	message (STATUS "Link to system-installed libarcstk and libarcsdec" )

	find_package (libarcstk  0.1.1 REQUIRED )
	find_package (libarcsdec 0.1.0 REQUIRED )
endif()


## --- Required: libarcstk

if (TARGET libarcstk::libarcstk )

	if (WITH_SUBMODULES )
		# Force to set RPATH instead of RUNPATH, point to .so's in build tree.
		# Note that this is for development and will not support installation.
		target_link_libraries (objects
			PUBLIC libarcstk::libarcstk -Wl,--disable-new-dtags )
	else()
		target_link_libraries (objects PUBLIC libarcstk::libarcstk )
	endif()
else()
	message (FATAL_ERROR "libarcstk targets are not present" )
endif()



## --- Required: libarcsdec

if (TARGET libarcsdec::libarcsdec )

	if (WITH_SUBMODULES )
		# Force to set RPATH instead of RUNPATH, point to .so's in build tree.
		# Note that this is for development and will not support installation.
		target_link_libraries (objects
			PUBLIC libarcsdec::libarcsdec -Wl,--disable-new-dtags )
	else()
		target_link_libraries (objects PUBLIC libarcsdec::libarcsdec )
	endif()
else()
	message (FATAL_ERROR "libarcsdec targets are not present" )
endif()



## --- Install executables

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



## --- BUILD_TYPE specific settings

option (WITH_NATIVE "Use platform specific optimization on compiling" OFF )

if (CMAKE_BUILD_TYPE STREQUAL "Release" )

	set (WITH_NATIVE ON CACHE BOOL
			"Release: Force platform specific optimization" FORCE )
endif ()



## --- Compiler Specific Settings

## g++: Warnings
set (PROJECT_CXX_GNU_FLAGS_WARNINGS
	-Wall -Wextra -Wpedantic -Weffc++
	-Wold-style-cast
	-Wsuggest-override
	-Wcast-align
	-Wcast-qual
	-Wctor-dtor-privacy
	-Wdisabled-optimization
	-Wformat=2
	-Wlogical-op
	-Wmissing-declarations
	-Wmissing-include-dirs
	-Wnoexcept
	-Woverloaded-virtual
	-Wredundant-decls
	-Wshadow
	-Wsign-conversion
	-Wsign-promo
	-Wstrict-null-sentinel
	-Wstrict-overflow=5
	-Wswitch-default
	-Wundef
	-Wuseless-cast
	-Wzero-as-null-pointer-constant
	)

## g++: Optimize flags
set (PROJECT_CXX_GNU_FLAGS_OPTIMIZE  -march=native -mtune=generic )


## clang++: Warnings
## Clang warnings are quite aggressive by default. Some of the g++ warnings are
## either compatibility flags that do nothing or are activated by other meta
## flags. Some are just on by default. The following list is intended to contain
## only those flags that are off by default and no compatibility flags.
set (PROJECT_CXX_CLANG_FLAGS_WARNINGS
	-Wall -Wextra -Wpedantic -Weffc++
	-Wold-style-cast
	-Wcast-align
	-Wcast-qual
	-Wformat=2
	-Wshadow
	-Wsign-conversion
	-Wundef
	-Wunused
	-Wzero-as-null-pointer-constant
	# -- clang++ specific --
	-Winconsistent-missing-destructor-override
	)

## clang++: Optimize flags
set (PROJECT_CXX_CLANG_FLAGS_OPTIMIZE -march=native -mtune=generic )


## Compiler: apply specific flags

if (CMAKE_COMPILER_IS_GNUCXX ) ## for g++

	message (STATUS "Select compile flags for g++" )

	set (PROJECT_CXX_FLAGS_WARNINGS ${PROJECT_CXX_GNU_FLAGS_WARNINGS} )
	set (PROJECT_CXX_FLAGS_OPTIMIZE ${PROJECT_CXX_GNU_FLAGS_OPTIMIZE} )

elseif (CMAKE_CXX_COMPILER MATCHES ".*clang"       ## for clang
		OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" )

	message (STATUS "Select compile flags for clang++" )

	set (PROJECT_CXX_FLAGS_WARNINGS ${PROJECT_CXX_CLANG_FLAGS_WARNINGS} )
	set (PROJECT_CXX_FLAGS_OPTIMIZE ${PROJECT_CXX_CLANG_FLAGS_OPTIMIZE} )

else()

	message (WARNING "C++ Compiler seems to be neither g++ nor clang++" )
	message (WARNING "No compile flags specified for the actual C++ compiler" )
	message (WARNING "Try a generic compile without project specific flags" )

endif()


## Compiler: Activate Warnings

target_compile_options (objects         PRIVATE ${PROJECT_CXX_FLAGS_WARNINGS} )
target_compile_options (${PROJECT_NAME} PRIVATE ${PROJECT_CXX_FLAGS_WARNINGS} )

message (STATUS "Warning Flags: ${PROJECT_CXX_FLAGS_WARNINGS}" )


## Compiler: Activate Optimizations (default ON for Release, otherwise OFF)

if (WITH_NATIVE )

	message (STATUS "Build with platform specific optimization" )

	target_compile_options (objects PRIVATE ${PROJECT_CXX_FLAGS_OPTIMIZE} )

	target_compile_options (${PROJECT_NAME}
		PRIVATE ${PROJECT_CXX_FLAGS_OPTIMIZE} )
else()

	message (STATUS "Build without platform specific optimization" )

endif (WITH_NATIVE )



## --- Tool informations (for manpages and installing symlinks)

set (TOOL_NAMES ) ## Iterable tool names, used for manpage generation
list (APPEND TOOL_NAMES
	${PROJECT_NAME}-calc
	${PROJECT_NAME}-id
	${PROJECT_NAME}-parse
	${PROJECT_NAME}-verify )



## --- Optional: Build tests, using Catch2 testing framework (default: OFF)

option (WITH_TESTS "Build unit tests" OFF )

if (WITH_TESTS)

	message (STATUS "Build with tests" )

	enable_testing()

	## Add Catch2 test framework
	add_subdirectory (thirdparty/catch2 EXCLUDE_FROM_ALL )

	## Add tests
	add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/test )

endif (WITH_TESTS)


## --- Optional: Build documentation (default: OFF)

option (WITH_DOCS          "Build documentation for public API"           OFF )
option (MCSS               "Use m.css when building documentation"        OFF )

## Process configuration for building the documentation
if (WITH_DOCS )

	add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/doc )
endif (WITH_DOCS )


## If on *nix, prepare man pages

if (UNIX )

	add_subdirectory (man )

endif (UNIX )

