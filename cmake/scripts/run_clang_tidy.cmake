## arcstk: CMake script for executing clang-tidy
##
## Note that clang-tidy does not have an option to specify file output. One has
## capture the output and pipe it to a file. This should be done platform
## independent by execute_process. Since we must use execute_process we use
## this standalone script tied to a custom target.

file (WRITE "${REPORT_FILE}" "")

file (GLOB_RECURSE ALL_SOURCES "${SOURCES_DIR}/*.cpp" )
list (FILTER ALL_SOURCES EXCLUDE REGEX ".*/test/.*" ) # do not tidy up tests

execute_process(
	COMMAND ${CLANG_TIDY_BINARY} ${ALL_SOURCES}
		--config-file=${CLANG_TIDY_CONFIG}
		-p ${BUILD_DIR}
		--
		-I${INCLUDE_DIR}
		-I${SOURCES_DIR}
		-isystem${LIBARCSTK_INCLUDE_DIR} # find libarcstk headers
		-isystem${LIBARCSDEC_INCLUDE_DIR} # find libarcsdec headers
		-std=c++17
	OUTPUT_FILE "${REPORT_FILE}"
	ERROR_FILE  "${LOG_FILE}"
)

message (STATUS "clang-tidy report written to: ${REPORT_FILE}" )
message (STATUS "clang-tidy log written to:    ${LOG_FILE}" )

file (SIZE ${REPORT_FILE} REPORT_SIZE )

## Determine exit code
if (IGNORE_ISSUES )
	message (STATUS "clang-tidy is told to ignore issues" )
	set (EXIT_CODE 0 )
else ()
	message (STATUS "clang-tidy is told to fail on issues" )
	set (EXIT_CODE ${REPORT_SIZE} )
endif ()

## Message according to result
if (REPORT_SIZE GREATER 0)
	message (WARNING
		"clang-tidy found issues, report file size: ${REPORT_SIZE}" )
else ()
	message (STATUS "clang-tidy did not find any issues" )
endif()

## Fail on error
if (EXIT_CODE GREATER 0 )
	## Print issue list
	file (READ "${REPORT_FILE}" CLANG_TIDY_ISSUES )
	message (STATUS "clang-tidy report: ${CLANG_TIDY_ISSUES}" )
endif ()

