## CMake file for building the documentation with m.css
##
## vim:fdm=marker
##
## Use HTML5/CSS3 from m.css for generating HTML from Doxygen XML output
##
## Prerequisites from PARENT:
## - Targets:   'doc'
## - Variables: DOC_COMMENT, DOC_DOXYFILE_BASE, DOC_GEN_DIR, DOC_SOURCES
##
## Provides:
## - Targets: 'arcstk_mcss_generate_html'

cmake_minimum_required (VERSION 3.10 )

message (STATUS "Will build documentation with m.css" )

if (WIN32 )

	message (FATAL_ERROR
		"m.css support is currently not available for Windows" )
endif()

find_package (Git REQUIRED )


## --- Overwrite Default Values From Parent {{{1

set (DOXYGEN_OUTPUT_DIRECTORY "${DOC_GEN_DIR}/mcss" )
set (DOC_COMMENT              "${DOC_COMMENT}, using m.css" )

## --- Some Variables {{{1

set (INDEX_HTML        "${DOXYGEN_OUTPUT_DIRECTORY}/html/index.html" )
set (INDEX_XML         "${DOXYGEN_OUTPUT_DIRECTORY}/xml/index.xml" )
set (DOXYFILE_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in" )
set (DOXYFILE          "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile" )
set (CONFPY            "${CMAKE_CURRENT_BINARY_DIR}/conf.py" )

set (MCSS_REPO_URL       "https://github.com/mosra/m.css.git" )
set (MCSS_SCRIPT_RELPATH "documentation/doxygen.py" )
## Commented out:
## Before m.css commit 3f568563a8b70fc49a479250d51a2fdc71a66943:
## set (MCSS_SCRIPT_RELPATH "doxygen/dox2html5.py" )
## 1}}}


add_custom_target (arcstk_mcss_generate_html DEPENDS "${INDEX_HTML}" )
add_dependencies  (arcstk_doc_generate_html arcstk_mcss_generate_html )


## --- Define m.css to be Checked out/Updated on Configure {{{1

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.11.0" )

	## --- FetchContent {{{2

	## This will NOT update the cloned m.css on build!

	include (FetchContent )

	FetchContent_Declare (m_css
		GIT_REPOSITORY    "${MCSS_REPO_URL}"
		#GIT_TAG          ... use always HEAD of default branch
		UPDATE_COMMAND    "${GIT_EXECUTABLE}" pull
	)

	if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.14" )

		FetchContent_MakeAvailable (m_css )

	else()

		if (NOT m_css_POPULATED )

			FetchContent_Populate (m_css )
		endif()
	endif()

	set (MCSS_SOURCE_DIR "${m_css_SOURCE_DIR}" )
	set (MCSS_CMD        "${m_css_SOURCE_DIR}/${MCSS_SCRIPT_RELPATH}" )
	# 2}}}
else()

	## No FetchContent module available

	## --- ExternalProject (will be updated on build) {{{2

	include (ExternalProject )

	ExternalProject_Add (arcstk_mcss_download
		GIT_REPOSITORY    ${MCSS_REPO_URL}
		#GIT_TAG          ... use always HEAD of default branch
		GIT_SHALLOW       true
		GIT_PROGRESS      true
		TIMEOUT           10
		# DOWNLOAD_COMMAND use default
		PATCH_COMMAND     ""
		UPDATE_COMMAND    "${GIT_EXECUTABLE}" pull
		CONFIGURE_COMMAND ""
		BUILD_COMMAND     ""
		INSTALL_COMMAND   ""
		TEST_COMMAND      ""
		LOG_DOWNLOAD      ON
	)

	ExternalProject_Get_Property (mcss_download SOURCE_DIR )

	set (MCSS_SOURCE_DIR "${SOURCE_DIR}" )
	set (MCSS_CMD        "${SOURCE_DIR}/${MCSS_SCRIPT_RELPATH}" )

	unset(SOURCE_DIR )

	add_dependencies (arcstk_mcss_generate_html arcstk_mcss_download )
	# 2}}}
endif()



## --- Create Doxyfile for m.css {{{1
configure_file ("${DOXYFILE_TEMPLATE}"  "${DOXYFILE}"  @ONLY )


## --- Create conf.py for m.css {{{1
set (CONFPY_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in" )
configure_file ("${CONFPY_SOURCE}" "${CONFPY}"  @ONLY )


## --- Configure Custom Colors (commented out) {{{1
#add_subdirectory (custom-colors )
## TODO Activate this for custom colors in m.css



## --- Set up Python Environment {{{1

add_requirements_target (arcstk_mcss_requirements
	"${CMAKE_CURRENT_SOURCE_DIR}/requirements.txt"
)

add_dependencies (arcstk_mcss_generate_html
	arcstk_mcss_requirements
	arcstk_doxygen_provide_xml
)



## --- Target for Running m.css {{{1

set (MCSS_LOG_FILE "${ARCSTK_LOG_BINARY_DIR}/MCSS.log" )

if (DOXYGEN_XML_POSTPROCESSING_TASKS )

	## Doxygen needs to run separately from m.css since we want to post-process
	## its XML output.

	add_doxygen_xml_target (arcstk_mcss_generate_xml
		"${DOXYFILE}" "${INDEX_XML}" )

	add_custom_command (
		OUTPUT  "${INDEX_HTML}"
		DEPENDS "${INDEX_XML}" ${DOC_SOURCES}
		COMMAND "${PYTHON_CMD}"
		ARGS    "${MCSS_CMD}" --no-doxygen "${CONFPY}" 2> "${MCSS_LOG_FILE}"
		WORKING_DIRECTORY "${DOC_GEN_DIR}"
		COMMENT "${DOC_COMMENT}"
		VERBATIM
	)
else()

	## Doxygen is called by m.css.

	## Since we do not use add_doxygen_target, we have to take care for the
	## dependencies manually
	add_dependencies (arcstk_mcss_generate_html
		## add deps from PARENT
		arcstk_create_output_dir
		arcstk_create_log_dir
		arcstk_static_pages_files
		## add deps from here
		arcstk_mcss_requirements
	)

	add_custom_command (
		OUTPUT  "${INDEX_HTML}"
		DEPENDS "${DOC_DOXYFILE_BASE}"
				"${DOXYFILE}"
				${DOC_SOURCES}
		COMMAND "${PYTHON_CMD}"
		ARGS    "${MCSS_CMD}" "${CONFPY}" 2> "${MCSS_LOG_FILE}"
		WORKING_DIRECTORY "${DOC_GEN_DIR}"
		COMMENT "${DOC_COMMENT}"
		VERBATIM
	)
endif()

