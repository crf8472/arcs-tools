## CMake file for building the documentation

cmake_minimum_required (VERSION 3.12 )

## Support IN_LIST if() operator required by FindDoxygen.cmake
cmake_policy (SET CMP0057 NEW )

find_package (Doxygen )
#find_package (Doxygen 1.9.1...<1.9.7 ) # 1.9.7 crashes m.css
#find_package (Doxygen 1.8.15...1.8.20 ) # OK too

message (STATUS
	"Note: to build documentation do 'cmake --build . --target doc'" )


## "Root directory for documentation config and sources" )
set (PROJECT_DOC_DIR      ${CMAKE_CURRENT_SOURCE_DIR} )


## --- Organize subdirectories and build directories

## "Target directory for generated documentation"
set (DOC_GEN_DIR          ${PROJECT_BINARY_DIR}/generated-docs )

## Target directory for 'doc' target (to be provided to subdirectory scripts)
set (DOC_BINARY_DIR       ${CMAKE_CURRENT_BINARY_DIR} )

## "Target directory for configured documentation texts"
set (DOC_TEXTS_GEN_DIR    ${DOC_BINARY_DIR}/texts )

## Path to import documentation texts
set (DOC_TEXTS_SOURCE_DIR ${PROJECT_DOC_DIR}/texts )


## --- Organize documentation sources

## Construct comment for doc target
set (DOC_COMMENT          "Build documentation for APIs" )

## Deployed mainpage file
set (DOC_MAINPAGE         ${DOC_TEXTS_GEN_DIR}/MAINPAGE.md )

## Define input for documentation
set (DOC_SOURCES          ${INTERFACE_HEADERS} ${PRIVATE_HEADERS}
						  ${DOC_MAINPAGE} )

## Define the target name and location of the base Doxyfile to be included
set (DOC_DOXYFILE_BASE    ${DOC_BINARY_DIR}/Doxyfile-base )

## Define the target name and location of the base Doxyfile to be included
set (DOC_DOXYFILE_BASE_TEMPLATE ${PROJECT_DOC_DIR}/Doxyfile.in )



## --- Generate documentation output directory

add_custom_command (
	OUTPUT  ${DOC_GEN_DIR}
	COMMAND ${CMAKE_COMMAND}
	ARGS    -E make_directory ${DOC_GEN_DIR} )


add_custom_target (arcstools_create_output_dir DEPENDS ${DOC_GEN_DIR} )


## --- Generate base doxyfile

## Set CMake controlled values in the Doxyfile base template

set (DOXYGEN_OUTPUT_DIRECTORY       "${DOC_GEN_DIR}/doxygen" )
set (DOXYGEN_FILE_PATTERNS          "*.hpp *.md" )
set (DOXYGEN_USE_MDFILE_AS_MAINPAGE "${DOC_MAINPAGE}" )
set (DOXYGEN_STRIP_FROM_INC_PATH    "${PROJECT_LOCAL_INCLUDE_DIR}/" )
set (DOXYGEN_STRIP_FROM_PATH        "${PROJECT_LOCAL_INCLUDE_DIR}/" )

string (REPLACE ";" " " DOXYGEN_INPUT
	"${PROJECT_LOCAL_INCLUDE_DIR}/;${DOC_TEXTS_GEN_DIR}/" )



## --- Rule to build base Doxyfile

configure_file ( ${DOC_DOXYFILE_BASE_TEMPLATE} ${DOC_DOXYFILE_BASE} @ONLY )



## --- Generate static mainpage

configure_file (${DOC_TEXTS_SOURCE_DIR}/MAINPAGE.md.in ${DOC_MAINPAGE} @ONLY )



## --- Activate requested documentation targets

## Add default doc target.
## Each documentation config in thirdparty/ may add its own concrete dependency
## target to 'doc' and do the concrete work.

add_custom_target (arcstools_doc )


## --- If some thirdparty documentation tool was requested, use it

list (APPEND GENERATORS MCSS RTD )

foreach (GENERATOR IN ITEMS ${GENERATORS} )

	if (USE_${GENERATOR} )

		string (TOLOWER ${GENERATOR} GENERATOR_DIR )

		list (APPEND LOAD_GENERATORS "${GENERATOR_DIR}" )
	endif()

endforeach()
unset (GENERATOR )
unset (GENERATOR_DIR )


## --- Detect Doxygen version with duplication bug and setup deduplication

## DOXYGEN_NEEDS_XML_DEDUP indicates that XML output requires post processing.
## It also indicates that not only plain doxygen HTML is requested.

if (LOAD_GENERATORS AND DOXYGEN_VERSION VERSION_GREATER_EQUAL 1.8.16
                    AND DOXYGEN_VERSION VERSION_LESS_EQUAL    1.13.2 )

	## TODO Not sure which doxygen version fixes the "duplication bug"
	## that was introduced in 1.8.16, but 1.13.2 seems to not contain it
	## anymore.

	set (DOXYGEN_NEEDS_XML_DEDUP TRUE ) ## Use this in subdirectories!
endif()


## Note: DOXYGEN_NEEDS_XML_DEDUP must be set _before_ actually loading the
## CMAKE scripts for generators with add_subdirectory()!

if (LOAD_GENERATORS )

	## Configure virtual Python Sandbox
	add_subdirectory ("thirdparty/python-virtualenv" EXCLUDE_FROM_ALL )
	## Now, PYTHON_ENV_AVAILABLE is "YES"
endif()



if (DOXYGEN_NEEDS_XML_DEDUP )

	message (STATUS "Doxygen version requires index deduplication" )

	## Setup deduplication script. We don't know whether it is actually
	## used, but it's in place.

	configure_file (
		"${PROJECT_DOC_DIR}/thirdparty/doxygen/dedup_index.py"
		"${DOC_BINARY_DIR}/dedup_index.py"
		@ONLY
	)

	## In either case, we need python for deduplication. Either a virtualenv is
	## setup for a generator (like m.css) or we try to use the systemwide
	## python.

	if (PYTHON_ENV_AVAILABLE )

		add_requirements_target (arcstools_doxygen_requirements
			"${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/doxygen/requirements.txt"
		)

	else()

		find_package (Python3 COMPONENTS Interpreter Development REQUIRED )

		set (PYTHON_CMD )

	endif (PYTHON_ENV_AVAILABLE )
else()
	if (NOT LOAD_GENERATORS )
		message (STATUS "No index deduplication since no generators requested" )
	else()
		message (STATUS "Doxygen version does not require index deduplication" )
	endif()
endif()



## --- Provide macro for subdir CMake configs to request doxygen runs

macro (add_doxygen_target DOXY_TARGET DOXYFILE INDEXFILE )

	add_custom_command (
		OUTPUT  ${INDEXFILE}
		COMMAND Doxygen::doxygen ${DOXYFILE}
		DEPENDS ${DOC_DOXYFILE_BASE} ${DOXYFILE} ${DOC_SOURCES}
		WORKING_DIRECTORY ${DOC_GEN_DIR}
		COMMENT "Run doxygen for target ${DOXY_TARGET}"
		VERBATIM
	)

	## Handle possible deduplication requirement
	if (DOXYGEN_NEEDS_XML_DEDUP )

		## Setup doxygen run target
		add_custom_target (${DOXY_TARGET}_run DEPENDS ${INDEXFILE} )
		add_dependencies  (${DOXY_TARGET}_run
			arcstools_create_log_dir arcstools_create_output_dir )

		set (STAMP_FILE ${INDEXFILE}.deduplicated )

		add_custom_command (
			OUTPUT  ${STAMP_FILE}
			COMMAND ${PYTHON_CMD}
			ARGS    ${DOC_BINARY_DIR}/dedup_index.py ${INDEXFILE}
			COMMAND ${CMAKE_COMMAND}
			ARGS    -E touch ${STAMP_FILE}
			DEPENDS ${INDEXFILE}
			WORKING_DIRECTORY "${DOC_GEN_DIR}"
			COMMENT "Remove duplicate compounds in ${INDEXFILE}"
			VERBATIM
		)

		## Setup deduplication target
		add_custom_target (${DOXY_TARGET}_dedup DEPENDS ${STAMP_FILE} )
		add_dependencies  (${DOXY_TARGET}_dedup ${DOXY_TARGET}_run )
		if (PYTHON_ENV_AVAILABLE )

			add_dependencies  (${DOXY_TARGET}_dedup
				arcstools_doxygen_requirements )
		endif()

		## Setup doxygen target
		add_custom_target (${DOXY_TARGET} )
		add_dependencies  (${DOXY_TARGET} ${DOXY_TARGET}_dedup )

		unset (STAMPFILE )
	else()

		add_custom_target (${DOXY_TARGET} DEPENDS ${INDEXFILE} )
		add_dependencies  (${DOXY_TARGET}
			arcstools_create_log_dir arcstools_create_output_dir )
	endif()
endmacro()


foreach (GENERATOR IN ITEMS ${LOAD_GENERATORS} )

	## Subdir CMake can attach target "python-virtualenv" as requirement
	add_subdirectory ("thirdparty/${GENERATOR}" EXCLUDE_FROM_ALL )
endforeach()



## --- If no generator is requested, just produce stock HTML

if (NOT LOAD_GENERATORS )

	message (STATUS "Will build documentation with pure doxygen (HTML)" )


	## -- Target for output mainpage

	set (DOC_OUTPUT_MAINPAGE ${DOXYGEN_OUTPUT_DIRECTORY}/html/index.html )

	add_custom_target (arcstools_doxygen_create_html
		DEPENDS ${DOC_OUTPUT_MAINPAGE} )


	## -- Load stylesheets if any

	## Fallback in case subdirectory does not overwrite it
	set (DOC_DOXYFILE_INPUT ${DOC_DOXYFILE_BASE} )

	## Comment or uncomment this line to use a alternative layouts to the
	## doxygen stock HTML layout. The subscope may re-set DOC_DOXYFILE_INPUT
	## and add dependencies to target 'arcstools_doxygen_create_html'.
	add_subdirectory ("thirdparty/doxygen-awesome-css" )

	message (STATUS "Use doxygen file: ${DOC_DOXYFILE_INPUT}" )


	## -- Add commands + organize targets

	add_custom_command (
		OUTPUT  ${DOC_OUTPUT_MAINPAGE}
		COMMAND Doxygen::doxygen ${DOC_DOXYFILE_INPUT}
		DEPENDS ${DOC_DOXYFILE_INPUT} ${DOC_SOURCES}
		WORKING_DIRECTORY ${DOC_GEN_DIR}
		COMMENT "Run doxygen to create HTML"
		VERBATIM
	)

	add_dependencies  (arcstools_doxygen_create_html
							arcstools_create_log_dir
							arcstools_create_output_dir )

	add_dependencies  (arcstools_doc arcstools_doxygen_create_html )

endif()


## --- If some parent defined a 'doc' target, otherwise define it autonomously

if (NOT TARGET doc)
	add_custom_target (doc )
endif()

add_dependencies (doc arcstools_doc )

