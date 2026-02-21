## CMake Script for Generating XML with Doxygen
##
## vim:fdm=marker

cmake_minimum_required (VERSION 3.12 )

message (STATUS "Doxygen XML generation is required" )


## Run doxygen to Produce XML
add_custom_target (arcstools_doxygen_generate_xml )

## Providing Postprocessed XML for Generator
add_custom_target (arcstools_doxygen_provide_xml )
add_dependencies (arcstools_doxygen_provide_xml arcstools_doxygen_generate_xml )

## --- Check for required Postprocessing Tasks {{{1

if (DOXYGEN_VERSION VERSION_GREATER_EQUAL 1.8.16 )

	## Order is not guaranteed, tasks must be orthogonal

	list (APPEND XML_POSTPROC_TASKS "index-dedup" )
	list (APPEND XML_POSTPROC_TASKS "compound-fix_vns" )
endif()
## 1}}}
## --- List of Postprocessing Tasks To Be Visible to Generators {{{1
set (DOXYGEN_XML_POSTPROCESSING_TASKS ${XML_POSTPROC_TASKS} PARENT_SCOPE )


## --- If Any Postprocessing Tasks Are Required, Support Them {{{1
if (XML_POSTPROC_TASKS )

	## Use this in generator build scripts to decide whether XML is required
	## as an intermediate step or HTML can be produced directly.
	set (DOXYGEN_XML_REQUIRED TRUE PARENT_SCOPE )

	add_subdirectory (tasks )
endif()


## --- Function for generating XML in the CMakeLists.txt of the Generators {{{1
function (add_doxygen_xml_target DOXY_TARGET DOXYFILE INDEX_XML )

	add_doxygen_target (${DOXY_TARGET} "${DOXYFILE}" "${INDEX_XML}" )

	add_dependencies (arcstools_doxygen_generate_xml ${DOXY_TARGET} )

	cmake_path (GET INDEX_XML PARENT_PATH _xmlpath )

	## Note that at this point we can also use the generator-specific:
	#message (WARNING "doxygen output path: ${DOXYGEN_OUTPUT_DIRECTORY}" )

	if (DOXYGEN_XML_POSTPROCESSING_TASKS )

		foreach (_task IN LISTS DOXYGEN_XML_POSTPROCESSING_TASKS )

			cmake_language (CALL "register_${_task}" "${_xmlpath}" )
		endforeach()
		unset (_task )

	endif()
	unset (_xmlpath )

	## NOTE that this provides only the target! It does not define how to
	## produce INDEX_XML! - Generator CMakeLists.txt MUST provide the
	## custom_command to actually create INDEX_XML!

endfunction()

