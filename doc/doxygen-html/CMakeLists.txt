## CMake Script for Generating HTML with Doxygen
##
## vim:fdm=marker
##
## Prerequisites:
## - Variables: DOC_DOXYFILE_BASE

cmake_minimum_required (VERSION 3.12 )

message (STATUS "Doxygen stock HTML generation is required" )

## --- Overwrite Default Values From Parent {{{1

set (DOC_DOXYFILE_INPUT "${DOC_DOXYFILE_BASE}" )

## Comment or uncomment this line to use a alternative layouts to the
## doxygen stock HTML layout. The subscope may re-set DOC_DOXYFILE_INPUT
## and add dependencies to target 'arcstk_doxygen_generate_html'.
#add_subdirectory ("doxygen-awesome-css" )

## --- Setup Targets {{{1
add_doxygen_target (arcstk_doxygen_generate_html
	"${DOC_DOXYFILE_INPUT}"
	"${DOXYGEN_OUTPUT_DIRECTORY}/html/index.html" )

if (TARGET arcstk_doxygen_awesome_css )

	add_dependencies (arcstk_doxygen_generate_html
		arcstk_doxygen_awesome_css )
endif()

add_dependencies (arcstk_doc_generate_html arcstk_doxygen_generate_html )

