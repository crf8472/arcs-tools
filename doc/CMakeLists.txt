## CMake script for building the documentation
##
## vim:fdm=marker
##
## Prerequisites:
## - target arcstools_create_log_dir
##
## Provides:
## - targets: doc, arcstools_doc, arcstools_create_output_dir,
##            arcstools_doc_generate_html, arcstools_doc_generate_pdf

cmake_minimum_required (VERSION 3.12 )
## --- Policies {{{1

## Support IN_LIST if() operator required by FindDoxygen.cmake
## https://cmake.org/cmake/help/latest/policy/CMP0057.html
cmake_policy (SET CMP0057 NEW )
## 1}}}

message (STATUS "Build documentation" )


find_package (Doxygen ) ## must be >= 1.9.8, also ok: 1.8.15...1.8.20


## --- Some Variables {{{1

## Target directory for generated documentation
set (DOC_GEN_DIR          "${PROJECT_BINARY_DIR}/generated-docs" )

## Construct comment for doc target
set (DOC_COMMENT          "Build documentation for APIs" )

## Define input for documentation
set (DOC_SOURCES          ${HEADER_FILES} ) ## FIXME empty

## Define the target name and location of the base Doxyfile to be included
set (DOC_DOXYFILE_BASE_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in" )

## Define the target name and location of the base Doxyfile to be included
set (DOC_DOXYFILE_BASE    "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile-base" )



## --- Add Default 'doc' Target {{{1

add_custom_target (arcstools_doc )
add_dependencies  (arcstools_doc arcstools_create_log_dir )
## Each documentation config in thirdparty/ may add its own concrete dependency
## target to 'doc' and do the concrete work.



## --- Generate Documentation Output Directory {{{1

add_custom_command (
	OUTPUT  "${DOC_GEN_DIR}"
	COMMAND "${CMAKE_COMMAND}"
	ARGS    -E make_directory "${DOC_GEN_DIR}"
	VERBATIM
)

add_custom_target (arcstools_create_output_dir DEPENDS "${DOC_GEN_DIR}" )



## --- Define Variables for Doxygen {{{1

## Set CMake controlled values in the Doxyfile base template.
## Doxygen may *only* use theses variables prefixed by "DOXYGEN_" and
## those from the root script prefixed by "PROJECT_".

set (DOXYGEN_OUTPUT_DIRECTORY       "${DOC_GEN_DIR}/doxygen" )
set (DOXYGEN_FILE_PATTERNS          "*.hpp *.md" )
set (DOXYGEN_STRIP_FROM_INC_PATH    "${PROJECT_LOCAL_INCLUDE_DIR}/" )
set (DOXYGEN_STRIP_FROM_PATH        "${PROJECT_LOCAL_INCLUDE_DIR}/" )
set (DOXYGEN_INPUT                  "${PROJECT_LOCAL_INCLUDE_DIR}/" )



## --- Provide Texts for Static Pages {{{1

add_subdirectory (texts ) ## Will update DOXYGEN_* variables or add more



## --- Setup Building Internal Docs if Requested (TODO broken) {{{1

if (WITH_INTERNAL_DOCS )

	message (STATUS "Will build documentation also for internal APIs" )

	## passed to doxygen

	set (DOXYGEN_INTERNAL_DOCS "YES" )

	string (REPLACE ";" " " DOXYGEN_INPUT
		"${DOXYGEN_INPUT};${PROJECT_SOURCE_DIR}/;${PROJECT_GEN_SOURCES_DIR}/" )

	string (REPLACE ";" " " DOXYGEN_STRIP_FROM_INC_PATH
		"${DOXYGEN_STRIP_FROM_INC_PATH}/;${PROJECT_SOURCE_DIR}/;${PROJECT_GEN_SOURCES_DIR}/" )

	string (REPLACE ";" " " DOXYGEN_STRIP_FROM_PATH
		"${DOXYGEN_STRIP_FROM_PATH}/;${PROJECT_SOURCE_DIR}/;${PROJECT_GEN_SOURCES_DIR}/" )

	# commented out, no list of private headers available
	#set (DOC_SOURCES ${DOC_SOURCES} ${PRIVATE_HEADERS} )

	## not passed to doxygen, cmake only

	set (DOC_COMMENT "${DOC_COMMENT} (also internal APIs)" )

else ()

	set (DOXYGEN_INTERNAL_DOCS "NO" )

endif (WITH_INTERNAL_DOCS)



## --- Configure Doxyfile-base {{{1

## Normalize DOXYGEN_INPUT list
string (REPLACE ";" " " DOXYGEN_INPUT "${DOXYGEN_INPUT}/" )

configure_file ( "${DOC_DOXYFILE_BASE_TEMPLATE}" "${DOC_DOXYFILE_BASE}" @ONLY )



## --- Populate List of Thirdparty Documentation Tools Requested {{{1

get_filename_component (_fullpath "${CMAKE_CURRENT_SOURCE_DIR}" REALPATH )

if (USE_DOC_TOOL )
	foreach (_generator IN LISTS USE_DOC_TOOL )

		string (TOLOWER ${_generator} _generator_dir )

		message (STATUS "Check path ${_fullpath}/thirdparty/${_generator_dir}")

		if (EXISTS "${_fullpath}/thirdparty/${_generator_dir}" )

			message (STATUS "Generate documentation by ${_generator_dir}" )

			list (APPEND LOAD_GENERATORS "${_generator_dir}" )
		else()

			message (WARNING "DOC_TOOL \"${_generator}\" not found, ignore" )
		endif()

		unset (_generator_dir )
	endforeach()
	unset (_fullpath  )
	unset (_generator )
endif (USE_DOC_TOOL )


## --- Declare doc targets for HTML and PDF {{{1

## FIXME Activate only those targets whose format is actually requested

add_custom_target (arcstools_doc_generate_html )
add_dependencies  (arcstools_doc arcstools_doc_generate_html )

add_custom_target (arcstools_doc_generate_pdf )
add_dependencies  (arcstools_doc arcstools_doc_generate_pdf )



## --- Provide Macro for Subdir CMake Scripts to Request Doxygen Runs {{{1
## Note that this target can produce XML as well as HTML, depends on Doxyfile.

function (add_doxygen_target DOXY_TARGET DOXYFILE INDEXFILE )

	add_custom_target (${DOXY_TARGET} DEPENDS "${INDEXFILE}" )

	add_dependencies  (${DOXY_TARGET}
			arcstools_create_log_dir
			arcstools_create_output_dir
			arcstools_static_pages_files
	)

	## Produce INDEXFILE (either XML or HTML) by running doxygen
	add_custom_command (
		OUTPUT  "${INDEXFILE}"
		DEPENDS "${DOC_DOXYFILE_BASE}" "${DOXYFILE}" ${DOC_SOURCES}
		COMMAND Doxygen::doxygen
		ARGS    "${DOXYFILE}"
		WORKING_DIRECTORY "${DOC_GEN_DIR}"
		COMMENT "Run doxygen for target ${DOXY_TARGET}"
		VERBATIM
	)
endfunction()



## --- Prepare Actual Generators or Stock HTML {{{1

if (LOAD_GENERATORS )

	## Configure virtual Python Sandbox
	add_subdirectory (python-virtualenv EXCLUDE_FROM_ALL )
	## Now, PYTHON_ENV_AVAILABLE is "YES"

	## Each generator requires XML (FIXME: no, lualatex doesn't)
	## TODO: Check: is doxygen-xml required by at least one generator?
	add_subdirectory (doxygen-xml )
	## Now, list DOXYGEN_XML_POSTPROCESSING_TASKS is provided (maybe empty)

	foreach (_generator IN ITEMS ${LOAD_GENERATORS} )

		add_subdirectory ("thirdparty/${_generator}" EXCLUDE_FROM_ALL )
	endforeach()
	unset (_generator )

else() # If no Generators are Specified, Just Use Doxygen's Stock HTML

	add_subdirectory (doxygen-html )
endif()
## 1}}}

## Add target 'doc' if it was not already added by submodules
if (NOT TARGET doc)
	add_custom_target (doc )
endif()

add_dependencies (doc arcstools_doc )

