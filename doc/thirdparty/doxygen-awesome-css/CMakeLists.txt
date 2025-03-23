## CMake file for doxygen-awesome-css
##
## CSS files for Doxygen HTML output
##
## Prerequisites from parent:
## - Targets:   'doc'
## - Variables: DOC_GEN_DIR, DOC_COMMENT,
##              DOXYGEN_OUTPUT_DIRECTORY,
##
## Provides:
## -


cmake_minimum_required (VERSION 3.10 )

cmake_policy (SET CMP0135 NEW )

message (STATUS "Will use doxygen-awesome-css for themeing doxygen HTML" )

find_package (Git REQUIRED )


## --- Overwrite default values from parent

set (DOC_COMMENT       "${DOC_COMMENT}, using doxygen-awesome-css" )

## --- Some variables

set (OUTPUT_MAINPAGE   "${DOXYGEN_OUTPUT_DIRECTORY}/html/index.html" )
set (DOXYFILE_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in" )
set (DOXYFILE          "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile" )

set (CUSTOM_STYLE_FILE "custom-chocolate.css" )

set (TARGET_CSS_DIR    "${DOXYGEN_OUTPUT_DIRECTORY}/html/css" )


## Github Repo URL
set (AWESOME_GITHUB_URL "https://github.com/jothepro/doxygen-awesome-css" )


## --- Define doxygen-awesome-css to be checked out/updated while configuring

if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.11.0" )

	## --- Use FetchContent if possible

	## This will NOT update the cloned doxygen-awesome-css site on build!

	include (FetchContent )

	FetchContent_Declare (doxygen-awesome-css
		URL "${AWESOME_GITHUB_URL}/archive/refs/heads/main.zip"
	)

	if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.14" )

		FetchContent_MakeAvailable (doxygen-awesome-css )

	else()

		if (NOT doxygen-awesome-css_POPULATED )

			FetchContent_Populate (doxygen-awesome-css )

		endif (NOT doxygen-awesome-css_POPULATED )
	endif()

	# Save the location the files were cloned into
	# This allows us to get the path to doxygen-awesome.css
	FetchContent_GetProperties(doxygen-awesome-css SOURCE_DIR SOURCE_CSS_DIR )

endif()

## Create Doxyfile for doxygen-awesome-css
configure_file ("${DOXYFILE_TEMPLATE}"  "${DOXYFILE}"  @ONLY )


## --- list of style files

## Create list of theme files required to deploy
## TODO Select only those files required for the concrete style
file (GLOB STYLE_FILES RELATIVE "${SOURCE_CSS_DIR}"
	"${SOURCE_CSS_DIR}/*.css" "${SOURCE_CSS_DIR}/*.js" )


## --- target: deploy stock style files

foreach (FILE ${STYLE_FILES} )
	add_custom_command(
		OUTPUT  ${TARGET_CSS_DIR}/${FILE}
		COMMAND ${CMAKE_COMMAND} -E copy
				"${SOURCE_CSS_DIR}/${FILE}"
				"${TARGET_CSS_DIR}/${FILE}" )
endforeach (FILE )

## Make list of source files to a list of target files
list(TRANSFORM STYLE_FILES PREPEND "${TARGET_CSS_DIR}/" )

add_custom_target (arcstools_deploy_css_files ALL DEPENDS ${STYLE_FILES} )


## --- target: deploy custom style files

add_custom_target (arcstools_deploy_custom_style_file ALL
	DEPENDS ${TARGET_CSS_DIR}/${CUSTOM_STYLE_FILE} )

add_custom_command(
	OUTPUT  ${TARGET_CSS_DIR}/${CUSTOM_STYLE_FILE}
	COMMAND ${CMAKE_COMMAND} -E copy
			"${CMAKE_CURRENT_SOURCE_DIR}/${CUSTOM_STYLE_FILE}"
			"${TARGET_CSS_DIR}/${FILE}" )

add_dependencies   (arcstools_deploy_css_files
										arcstools_deploy_custom_style_file )

## --- target: create css directory

add_custom_command (
	OUTPUT  ${TARGET_CSS_DIR}
	COMMAND ${CMAKE_COMMAND}
	ARGS    -E make_directory ${TARGET_CSS_DIR} )

add_custom_target (arcstools_create_css_dir ALL DEPENDS ${TARGET_CSS_DIR} )

add_dependencies  (arcstools_deploy_css_files arcstools_create_css_dir )


## -- Pass results to parent

set (DOC_DOXYFILE_INPUT "${DOXYFILE}" PARENT_SCOPE )

add_dependencies  (arcstools_doxygen_create_html arcstools_deploy_css_files )

