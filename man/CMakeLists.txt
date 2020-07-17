## CMakeLists.txt for generating and compressing the man pages

find_package (Doxygen 1.8.0 )

find_program (GZIP_EXEC NAMES gzip )


## --- Targets, Files and Directories

## Directory for manpage source texts
set (PROJECT_MAN_TEXTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/texts" )

## The generated doxyfile to use
set (DOXYFILE ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile )

## The manpage sources after cmake configure_file()
set (PROJECT_MAN_SRC_DIR "${CMAKE_CURRENT_BINARY_DIR}/input-dox" )

## The automated doxygen output (with all redundant pages)
set (PROJECT_MAN_DOXYGEN_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/output-man" )

## "Target directory for generated man pages
set (PROJECT_MAN_BUILD_DIR    "${PROJECT_BINARY_DIR}/generated-manpages" )


## Compound target for creating the manpages (conatins all steps)
add_custom_target (provide_manpages ALL)


## --- Generate the dox file sources from the .dox.in files

## List of cmake-controlled manpage source files
file (GLOB MAN_SOURCE_FORMS ${PROJECT_MAN_TEXTS_DIR}/*.dox.in )

if (${CMAKE_VERSION} VERSION_LESS "3.14.0" ) ## No NAME_WLE available

	foreach (_form IN LISTS MAN_SOURCE_FORMS )

		get_filename_component(_form_NAME ${_form} NAME_WE )
		configure_file (${_form} "${PROJECT_MAN_SRC_DIR}/${_form_NAME}.dox" )
	endforeach()

else ()

	foreach (_form IN LISTS MAN_SOURCE_FORMS )

		get_filename_component(_form_OUTFILE ${_form} NAME_WLE )
		configure_file (${_form} "${PROJECT_MAN_SRC_DIR}/${_form_OUTFILE}" )
	endforeach()

endif()


## List of manpage source text files (those not controlled by cmake)
file (GLOB MAN_SOURCE_TEXTS ${PROJECT_MAN_TEXTS_DIR}/*.dox )

foreach (_doxfile IN LISTS MAN_SOURCE_TEXTS )

	get_filename_component(_doxfilename ${_doxfile} NAME )
	file (COPY ${_doxfile} DESTINATION "${PROJECT_MAN_SRC_DIR}" )
endforeach()


## --- Generate the man source files from the source texts

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile-man.in ${DOXYFILE} @ONLY )

## Produce doxygen output

set (GENERATION_STAMP ${CMAKE_CURRENT_BINARY_DIR}/man_generated.stamp )

file (MAKE_DIRECTORY ${PROJECT_MAN_BUILD_DIR} )

add_custom_command (
	OUTPUT  ${GENERATION_STAMP}
	COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE}
	COMMAND cmake -E touch ${GENERATION_STAMP}
	WORKING_DIRECTORY ${PROJECT_MAN_DOXYGEN_OUT_DIR}
	DEPENDS ${MAN_SOURCE_FORMS} ${MAN_SOURCE_TEXTS}
	COMMENT "Generate manpages with Doxygen"
	VERBATIM
)

## Move the relevant files of the output to target directory

set (MANPAGES )
set (INSTALL_PAGES )
foreach (_toolname IN LISTS TOOL_NAMES )

	list (APPEND MANPAGES "${PROJECT_MAN_DOXYGEN_OUT_DIR}/1/${_toolname}.1" )
	list (APPEND INSTALL_PAGES "${PROJECT_MAN_BUILD_DIR}/${_toolname}.1" )
endforeach()

file (MAKE_DIRECTORY ${PROJECT_MAN_DOXYGEN_OUT_DIR} )

add_custom_command (
	OUTPUT ${INSTALL_PAGES}
	COMMAND cmake -E copy ${MANPAGES} "${PROJECT_MAN_BUILD_DIR}"
	DEPENDS ${GENERATION_STAMP}
	COMMENT "Copy manpages to build directory"
	VERBATIM
)

## Generate the manpages from text files
add_custom_target (generate_manpages DEPENDS ${MANPAGES} )


## --- Compress the generated manpages with gzip, iff gzip is available

if (GZIP_EXEC)

	include (FindPackageHandleStandardArgs )
	FIND_PACKAGE_HANDLE_STANDARD_ARGS (gzip DEFAULT_MSG GZIP_EXEC )

	mark_as_advanced (GZIP_EXEC )

	set (COMPRESSION_STAMP ${CMAKE_CURRENT_BINARY_DIR}/man_compressed.stamp )

	## Replace all *.1 pages with their compressed counterparts
	## (Keep the uncompressed sources with -k)
	add_custom_command (
		OUTPUT ${COMPRESSION_STAMP}
		COMMAND ${GZIP_EXEC} -f --best ${PROJECT_MAN_BUILD_DIR}/*.1
		COMMAND cmake -E touch ${COMPRESSION_STAMP}
		WORKING_DIRECTORY "${PROJECT_MAN_BUILD_DIR}"
		COMMENT "Compress manpages with gzip"
		DEPENDS ${GENERATION_STAMP}
	)

	## Take the filenames of the compressed files instead of the uncompressed
	string (REPLACE ".1" ".1.gz" INSTALL_PAGES "${INSTALL_PAGES}" )

	## Target for compressing the generated manpages
	add_custom_target (compress_manpages DEPENDS ${COMPRESSION_STAMP} )

	add_dependencies (compress_manpages generate_manpages )
	add_dependencies (provide_manpages  compress_manpages )

else() ## If gzip is not available, install man pages uncompressed

	message (WARNING "Gzip not found. Manpages will not be compressed." )

	add_dependencies (provide_manpages generate_manpages )

endif (GZIP_EXEC)


## --- Specify manpage installation

if (NOT SKIP_INSTALL_ALL AND NOT SKIP_INSTALL_FILES )

	install (FILES ${INSTALL_PAGES} DESTINATION "${INSTALL_MAN_DIR}/man1" )

endif()

add_custom_target (man DEPENDS provide_manpages )

