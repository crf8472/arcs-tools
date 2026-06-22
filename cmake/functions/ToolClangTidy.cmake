## arcstk: CMake functions for static code analysis
## vim:fdm=marker

function (arcstk_enable_clang_tidy OUT_VAR )

	set (${OUT_VAR} FALSE PARENT_SCOPE )

	find_program (CLANG_TIDY_BINARY
		NAMES clang-tidy
		DOC "Path to clang-tidy excecutable"
	)

	if (NOT CLANG_TIDY_BINARY )
		message (WARNING "clang-tidy not found. Install clang-tools.")
		return()
	endif()

	message (STATUS "clang-tidy found: ${CLANG_TIDY_BINARY}" )

	set (CLANG_TIDY_SCRIPT
		"${CMAKE_CURRENT_SOURCE_DIR}/cmake/scripts/run_clang_tidy.cmake" )

	if (NOT EXISTS "${CLANG_TIDY_SCRIPT}" )
		message(WARNING "clang-tidy script not found: ${CLANG_TIDY_SCRIPT}")
		return()
	endif()

	set (COMPILEDB_DIR "${ARCSTK_BINARY_DIR}/" )
	set (INCLUDE_DIR   "${ARCSTK_INCLUDE_BINARY_DIR}" )
	set (REPORT_FILE   "${ARCSTK_BINARY_DIR}/clang_tidy_report.txt" )
	set (LOG_FILE      "${ARCSTK_BINARY_DIR}/clang_tidy_report.log" )

	add_custom_target(${PROJECT_NAME}_clang-tidy
		COMMAND ${CMAKE_COMMAND}
			-DCLANG_TIDY_BINARY="${CLANG_TIDY_BINARY}"
			-DSOURCES_DIR="${ARCSTK_SOURCE_DIR}"
			-DCOMPILEDB_DIR="${COMPILEDB_DIR}"
			-DINCLUDE_DIR="${INCLUDE_DIR}"
			-DREPORT_FILE="${REPORT_FILE}"
			-DLOG_FILE="${LOG_FILE}"
			-P "${CLANG_TIDY_SCRIPT}"
		WORKING_DIRECTORY "${ARCSTK_ROOT_DIR}"
		BYPRODUCTS "${REPORT_FILE}" "${LOG_FILE}"
		COMMENT "Running clang-tidy..."
	)

	set(${OUT_VAR} TRUE PARENT_SCOPE)
endfunction()

