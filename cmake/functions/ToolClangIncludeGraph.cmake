## arcstk: CMake functions for drawing an include graph
## vim:fdm=marker

function (arcstk_enable_clang_include_graph )

	set (${OUT_VAR} FALSE PARENT_SCOPE )

	set (options      KEEP_DOT_FILES )
	set (oneValueArgs FORMAT OUTPUT_DIR )
	set (multiValueArgs )

	cmake_parse_arguments (GRAPH
		"${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

	## Find clang-include-graph

	find_program (CLANG_INCLUDE_GRAPH_EXECUTABLE clang-include-graph )

	if (NOT CLANG_INCLUDE_GRAPH_EXECUTABLE )
		message(WARNING "clang-include-graph executable not found.")
		return()
	endif()

	## Find dot

	find_program (DOT_EXECUTABLE dot )

	if (NOT DOT_EXECUTABLE )
		message(WARNING "dot executable not found. Install Graphviz.")
		return()
	endif()

	## Defaults

	if (NOT GRAPH_FORMAT )
		set (GRAPH_FORMAT "png" )
	endif()

	if (NOT GRAPH_OUTPUT_DIR )
		set (GRAPH_OUTPUT_DIR "${LIBARCSDEC_BINARY_DIR}/clang_include_graph" )
	endif()

	## Setup files and directories

	set (DOT_FILE   "${GRAPH_OUTPUT_DIR}/${PROJECT_NAME}.dot" )
	set (IMAGE_FILE
		"${GRAPH_OUTPUT_DIR}/${PROJECT_NAME}_includes.${GRAPH_FORMAT}" )

	file(MAKE_DIRECTORY ${GRAPH_OUTPUT_DIR} )

	## Collect commands

	set (GRAPH_COMMANDS
		COMMAND ${CLANG_INCLUDE_GRAPH_EXECUTABLE}
			-J 4
			--output ${DOT_FILE}
			--compilation-database-dir ${LIBARCSDEC_BINARY_DIR}
			--relative-to ${LIBARCSDEC_INCLUDE_BINARY_DIR}
			--relative-only
			--exclude-system-headers
			--graphviz
		COMMAND ${DOT_EXECUTABLE} -T${GRAPH_FORMAT} ${DOT_FILE} -o ${IMAGE_FILE}
	)

	## Optional remove dot file
	if (NOT GRAPH_KEEP_DOT_FILES )
		list (APPEND GRAPH_COMMANDS
			COMMAND ${CMAKE_COMMAND} -E rm -f ${DOT_FILE} )
	endif()

	add_custom_target (${PROJECT_NAME}_include-graph
		${GRAPH_COMMANDS}
		WORKING_DIRECTORY ${LIBARCSDEC_BINARY_DIR}
		BYPRODUCTS ${IMAGE_FILE}
		COMMENT "Generating target dependency graph (${IMAGE_FILE})"
	)

	set(${OUT_VAR} TRUE PARENT_SCOPE)
endfunction()

