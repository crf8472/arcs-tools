## arcstk: Setup build tree
## vim:fdm=marker

function (setup_build_tree)

## -- Log Directory {{{1

add_custom_command (
	OUTPUT  "${PROJECT_LOG_DIR}"
	COMMAND "${CMAKE_COMMAND}"
	ARGS    -E make_directory "${PROJECT_LOG_DIR}"
	VERBATIM
)

add_custom_target (arcstools_create_log_dir ALL
	DEPENDS "${PROJECT_LOG_DIR}" )
##}}}1

endfunction()

