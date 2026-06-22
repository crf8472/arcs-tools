## arcstk: Setup build tree
## vim:fdm=marker

function (setup_build_tree)

## -- Log Directory {{{1

add_custom_command (
	OUTPUT  "${ARCSTK_LOG_BINARY_DIR}"
	COMMAND "${CMAKE_COMMAND}"
	ARGS    -E make_directory "${ARCSTK_LOG_BINARY_DIR}"
	VERBATIM
)

add_custom_target (arcstools_create_log_dir ALL
	DEPENDS "${ARCSTK_LOG_BINARY_DIR}" )
##}}}1

endfunction()

