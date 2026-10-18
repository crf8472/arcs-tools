## CMake script for providing the manpage texts for processing by doc/ or man/
##
## vim:fdm=marker
##
## Provides:
##
## - Target: arcstk_manpage_text_files
## - Variables:

# MAN_TEXTS_DIR -> Quelle mit dox.in files
# MAN_SRC_DIR   -> Ziel für dox files
# set(MAN_TEXT_NAMES "man_calc_1" "man_id_1" "man_parse_1" "man_verify_1"
#	"include" )

## --- Generate the dox file sources from the .dox.in files

function (configure_manpage_texts TEXTS_SRC_DIR TEXTS_BIN_DIR )

	list (GET TOOL_NAMES 0 TOOL_NAME_CALC )
	list (GET TOOL_NAMES 1 TOOL_NAME_ID )
	list (GET TOOL_NAMES 2 TOOL_NAME_PARSE )
	list (GET TOOL_NAMES 3 TOOL_NAME_VERIFY )

	## List of cmake-controlled manpage source files
	file (GLOB MAN_SOURCE_TEMPLATES ${TEXTS_SRC_DIR}/*.dox.in )

	if (${CMAKE_VERSION} VERSION_LESS "3.14.0" ) ## No NAME_WLE available

		foreach (_input IN LISTS MAN_SOURCE_TEMPLATES )

			get_filename_component(_input_NAME ${_input} NAME_WE )
			set (_output "${TEXTS_BIN_DIR}/${_input_NAME}.dox")

			configure_file (${_input} "${_output}" @ONLY )
		endforeach()

	else ()

		foreach (_input IN LISTS MAN_SOURCE_TEMPLATES )

			get_filename_component(_input_OUTFILE ${_input} NAME_WLE )
			set (_output "${TEXTS_BIN_DIR}/${_input_OUTFILE}" )

			configure_file (${_input} "${_output}" @ONLY )
		endforeach()

	endif()


	## List of manpage source text files (those not controlled by cmake)
	file (GLOB MAN_SOURCE_TEXTS ${TEXTS_SRC_DIR}/*.dox )

	foreach (_doxfile IN LISTS MAN_SOURCE_TEXTS )

		get_filename_component(_doxfilename ${_doxfile} NAME )
		file (COPY ${_doxfile} DESTINATION "${TEXTS_BIN_DIR}" )
	endforeach()

endfunction()

