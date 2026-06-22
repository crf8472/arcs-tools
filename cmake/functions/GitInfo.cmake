## libarcsdec: Acquire git version info
## vim:fdm=marker

find_package (Git QUIET REQUIRED )

## Execute git
function (_git_execute_command OUT_VAR )

	# ARGN contains all arguments after OUT_VAR
	execute_process (
		COMMAND "${GIT_EXECUTABLE}" ${ARGN}
		WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
		RESULT_VARIABLE GIT_EXIT_CODE
		OUTPUT_VARIABLE GIT_OUTPUT
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)

	if (GIT_EXIT_CODE EQUAL 0)
		set (${OUT_VAR} "${GIT_OUTPUT}" PARENT_SCOPE )
		return()
	endif()

	set (${OUTPUT_VAR} "UNKNOWN" PARENT_SCOPE )
endfunction()


# Get git version string
function(git_get_version_string VERSION_VAR )

	_git_execute_command(GIT_VERSION describe --always HEAD )

	if (GIT_VERSION STREQUAL "UNKNOWN" )

		set (GIT_VERSION "v0.0.0-nogit")
		message (WARNING "Git describe failed, using fallback: ${GIT_VERSION}" )

	else()
		message (STATUS "git describe --always HEAD: ${GIT_VERSION}" )
	endif()

	set (${VERSION_VAR} "${GIT_VERSION}" PARENT_SCOPE )
endfunction()

# Get git commit id
function(git_get_commit_id COMMIT_VAR )

	_git_execute_command(GIT_COMMIT_ID rev-parse HEAD )

	if (GIT_COMMIT_ID STREQUAL "UNKNOWN")

		set (GIT_COMMIT_ID "00000000")
		message (WARNING
			"Git rev-parse failed, using fallback: ${GIT_COMMIT_ID}" )

	else()
		message (STATUS "git rev-parse HEAD: ${GIT_COMMIT_ID}" )
	endif()

	set (${COMMIT_VAR} "${GIT_COMMIT_ID}" PARENT_SCOPE )

endfunction()

