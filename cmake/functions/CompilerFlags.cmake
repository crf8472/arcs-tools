## arcstk: Compiler-specific warning and optimization flags
## vim:fdm=marker

## --- g++ Flags: Warnings {{{1

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
	-Wstrict-overflow=3
	-Wswitch-default
	-Wundef
	-Wuseless-cast
	-Wzero-as-null-pointer-constant
)

## 1}}}
## --- g++ Flags: Optimize {{{1

set ( PROJECT_CXX_GNU_FLAGS_OPTIMIZE
	-march=native
	-mtune=generic
)

## 1}}}

## --- clang++ Flags: Warnings {{{1

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

## 1}}}
## --- clang++ Flags: Optimize {{{1

set (PROJECT_CXX_CLANG_FLAGS_OPTIMIZE
	-march=native
	-mtune=generic
)

## 1}}}

## --- Option: WITH_NATIVE (default ON for Release only) {{{1

option (WITH_NATIVE "Use platform specific optimization on compiling" OFF )

## For 'Release', Set Default ON
if (CMAKE_BUILD_TYPE STREQUAL "Release")

	set (WITH_NATIVE ON CACHE BOOL
			"Release: Force platform specific optimization" FORCE )
endif ()
# }}}1

## --- Option: ENABLE_COVERAGE (default OFF) {{{1

option (ENABLE_COVERAGE "Enable code coverage" OFF )

if (ENABLE_COVERAGE )

	include (${CMAKE_CURRENT_SOURCE_DIR}/cmake/functions/ToolCodeCoverage.cmake)
	enable_code_coverage (COVERAGE_ENABLED )

	if (NOT COVERAGE_ENABLED )
		message (WARNING "No code coverage available" )
	endif()
endif ()

# }}}1

## --- Select Flags for Actual Compiler {{{1

if (CMAKE_COMPILER_IS_GNUCXX )                      ## for g++

	set (PROJECT_CXX_FLAGS_WARNINGS ${PROJECT_CXX_GNU_FLAGS_WARNINGS} )
	set (PROJECT_CXX_FLAGS_OPTIMIZE ${PROJECT_CXX_GNU_FLAGS_OPTIMIZE} )
	#set (PROJECT_CXX_FLAGS_OPTIMIZE ${PROJECT_CXX_GNU_FLAGS_OPTIMIZE} -fsanitize=address -fsanitize=undefined )

	message(STATUS "Compiler: g++ (selected appropriate flags)" )

elseif (CMAKE_CXX_COMPILER MATCHES ".*clang"        ## for clang++
		OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" )

	set (PROJECT_CXX_FLAGS_WARNINGS ${PROJECT_CXX_CLANG_FLAGS_WARNINGS} )
	set (PROJECT_CXX_FLAGS_OPTIMIZE ${PROJECT_CXX_CLANG_FLAGS_OPTIMIZE} )
	#set (PROJECT_CXX_FLAGS_OPTIMIZE ${PROJECT_CXX_CLANG_FLAGS_OPTIMIZE} -fsanitize=address -fsanitize=undefined )

	message (STATUS "Compiler: clang++ (selected appropriate flags)" )

else()

	message (WARNING "Unknown compiler: no specific flags will be applied" )

endif()

## 1}}}

## --- Apply Compiler Flags to Library Sources (not to Test Sources) {{{1

function (apply_compiler_flags TARGET_NAME )

	## Apply warning flags

	## We do not use target_compile_options since those flags should not be
	## applied to Catch2 and tests.
	## TODO Find a way to use target_compile_options

	## Apply warning flags to each source file
	## (not applied to Catch2 or test sources)
	get_target_property (PROJECT_SOURCES ${TARGET_NAME} SOURCES )

	foreach (_sourcefile ${PROJECT_SOURCES} )

		set_property (SOURCE "${_sourcefile}"
			APPEND
			PROPERTY COMPILE_OPTIONS ${PROJECT_CXX_FLAGS_WARNINGS}
		)

	endforeach()

	message(STATUS "Applied warning flags to ${TARGET_NAME}")

	## Apply optimization flags if requested

	if (WITH_NATIVE )

		message (STATUS "Build with platform specific optimization")

		target_compile_options (${TARGET_NAME}
			PRIVATE ${PROJECT_CXX_FLAGS_OPTIMIZE} )
	else()

		message (STATUS "Build without platform specific optimization" )

	endif (WITH_NATIVE )
endfunction()
# }}}1
