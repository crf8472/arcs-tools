## CMake script for addings postprocessing tasks to doxygen's XML
##
## vim:fdm=marker

cmake_minimum_required (VERSION 3.12 )

message (STATUS "Doxygen XSLT postprocessing is required" )


## --- Setup Python target arcstools_doxygen_xml_env {{{1

## Target for preparing the doxygen XML run (python etc.)
add_custom_target (arcstools_doxygen_xml_env )
add_dependencies (arcstools_doxygen_provide_xml arcstools_doxygen_xml_env )

if (PYTHON_ENV_AVAILABLE )

	## A virtualenv was already setup for a HTML static site generator
	## (like m.css).

	add_requirements_target (arcstools_doxygen_xml_python_requirements
		"${CMAKE_CURRENT_SOURCE_DIR}/requirements.txt"
	)

	add_dependencies  (arcstools_doxygen_xml_env
		arcstools_doxygen_xml_python_requirements )

else()

	## Try to use the systemwide python.

	## TODO Test this, it may be just unsuitable!

	## Will fail if python requirements were not met. We cannot install
	## requirements systemwide.

	find_package (Python3 COMPONENTS Interpreter Development REQUIRED )

	set (PYTHON_CMD )

endif()
## 1}}}
## --- Function: Add a XSLT post processing command {{{1
function (add_doxygen_xslt_task TASK XML_INPUT_FILES )

	## "Chain" a target: task-target depends on doxygen_generate_xml and is
	## itself a dependency of doxygen_provide_xml. The actual posprocessing
	## will be a POST_BUILD command to this target.

	set (TASK_TARGET arcstools_doxygen_xsl_task_${TASK} )

	add_custom_target (${TASK_TARGET} )

	add_dependencies (${TASK_TARGET}
		arcstools_doxygen_generate_xml
		arcstools_doxygen_xml_env
	)

	add_dependencies (arcstools_doxygen_provide_xml ${TASK_TARGET} )

	## NOTE Get XML input filenames at build time
	##
	## It would be nice to use a task-specific call at build time to get the
	## XML input filenames. We would use a build-time command like:
	##
	## add_custom_command (
	##	TARGET  ${TASK_TARGET} PRE_BUILD
	##	COMMENT "List input files of task ${TASK}"
	##	COMMAND ${CMAKE_COMMAND} -P
	##		"${PROJECT_ROOT_DIR}/doc/doxygen-xml/tasks/${TASK}/file_list.cmake"
	##	VERBATIM
	## )
	##
	## Also possible as POST_BUILD of libarcstk_doxygen_generate_xml.
	##
	## 2 impediments:
	##
	## - We would have to know and pass the XML input path at this site. This
	##   would be redundant and overly complex since doxygen-xml/CMakeLists.txt
	##   is the correct site to do that.
	## - We would have to transport the file list back to this site. If we do
	##   this by -P with a script, there is no PARENT_SCOPE. We would have to
	##   capture the output (somehow).

	add_custom_command (
		TARGET  ${TASK_TARGET} POST_BUILD
		COMMAND "${PYTHON_CMD}"
		ARGS    "${PROJECT_ROOT_DIR}/doc/doxygen-xml/tasks/xsl_transform.py"
				"${XML_INPUT_FILES}"
				"${PROJECT_ROOT_DIR}/doc/doxygen-xml/tasks/${TASK}/${TASK}.xsl"
		WORKING_DIRECTORY "${DOC_GEN_DIR}"
		COMMENT "Run transformations of task ${TASK}"
		VERBATIM
	)

	unset (TASK_TARGET )

endfunction()


## --- Load Requested Postprocessing Tasks from Subdirectories {{{1
foreach (_task IN LISTS XML_POSTPROC_TASKS )

	add_subdirectory (${_task} )
endforeach()
unset (_task )

