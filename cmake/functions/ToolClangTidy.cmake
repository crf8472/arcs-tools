## arcstk: CMake functions for static code analysis
## vim:fdm=marker

cmake_minimum_required (VERSION 3.18 )


##
function (_arcstk_add_clang_tidy_target )

	set (options        IGNORE_ISSUES )
	set (one_value_args TARGET_NAME   )

	cmake_parse_arguments (TIDY "${options}" "${one_value_args}" "" ${ARGN} )

	## add cutom clang-tidy command

	set (_REPORT_NAME "${ARCSTK_BINARY_DIR}/${TIDY_TARGET_NAME}_report" )
	set (_REPORT_FILE "${_REPORT_NAME}.txt" )
	set (_LOG_FILE    "${_REPORT_NAME}.log" )

	add_custom_command(
		OUTPUT "${_REPORT_FILE}"
		COMMAND ${CMAKE_COMMAND}
			-DCLANG_TIDY_BINARY="${CLANG_TIDY_BINARY}"
			-DCLANG_TIDY_CONFIG="${ARCSTK_ROOT_DIR}/.clang-tidy"
			-DSOURCES_DIR="${ARCSTK_SOURCE_DIR}"
			-DBUILD_DIR="${ARCSTK_BINARY_DIR}"
			-DINCLUDE_DIR="${ARCSTK_INCLUDE_SOURCE_DIR}"
			-DLIBARCSTK_INCLUDE_DIR="${ARCSTK_BINARY_DIR}/libs/libarcstk/include"
			-DLIBARCSDEC_INCLUDE_DIR="${ARCSTK_BINARY_DIR}/libs/libarcsdec/include"
			-DREPORT_FILE="${_REPORT_FILE}"
			-DLOG_FILE="${_LOG_FILE}"
			-DIGNORE_ISSUES="${TIDY_IGNORE_ISSUES}"
			-P "${CLANG_TIDY_SCRIPT}"
		WORKING_DIRECTORY "${ARCSTK_ROOT_DIR}"
		BYPRODUCTS "${_LOG_FILE}"
		COMMENT "Running clang-tidy..."
	)

	add_custom_target (${TIDY_TARGET_NAME} DEPENDS "${_REPORT_FILE}" )

endfunction ()


## Enable static analysis functionality
function (arcstk_enable_clang_tidy OUT_VAR ) # {{{1

	set (${OUT_VAR} FALSE PARENT_SCOPE )

	find_program (CLANG_TIDY_BINARY
		NAMES clang-tidy
		DOC "Path to clang-tidy excecutable"
	)

	if (NOT CLANG_TIDY_BINARY )
		message (WARNING "Target ${PROJECT_NAME}_clang-tidy: not provided"
			" since clang-tidy was not found."
			" This message is for developers only,"
			" ignore it for regular builds."
		)
		return()
	endif()

	message (STATUS "clang-tidy found: ${CLANG_TIDY_BINARY}" )

	set (CLANG_TIDY_SCRIPT
		"${CMAKE_CURRENT_SOURCE_DIR}/cmake/scripts/run_clang_tidy.cmake" )

	if (NOT EXISTS "${CLANG_TIDY_SCRIPT}" )
		message (WARNING "Target ${PROJECT_NAME}_clang-tidy: not provided"
			" since clang-tidy script not found: ${CLANG_TIDY_SCRIPT}."
			" This message is for developers only, ignore it for regular builds."
		)
		return()
	endif()

	_arcstk_add_clang_tidy_target (
		TARGET_NAME   ${PROJECT_NAME}_clang-tidy
		IGNORE_ISSUES
	)

	_arcstk_add_clang_tidy_target (
		TARGET_NAME ${PROJECT_NAME}_clang-tidy-workflow
	)

	set(${OUT_VAR} TRUE PARENT_SCOPE)
endfunction() # 1}}}

