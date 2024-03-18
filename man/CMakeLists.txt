## CMakeLists.txt for generating and compressing the man pages

find_package (Doxygen 1.8.0 )

find_program (GZIP_EXEC NAMES gzip )


## --- Targets, Files and Directories

## The generated doxyfile to use
set (DOXYFILE ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile )

## Directory for manpage source texts
set (MAN_TEXTS_DIR       "${CMAKE_CURRENT_SOURCE_DIR}/texts" )

## Directory for configured manpage sources
set (MAN_SRC_DIR         "${CMAKE_CURRENT_BINARY_DIR}/configured-doxfiles" )

## Directory for doxygen output (with all redundant pages)
set (MAN_DOXYGEN_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/doxygen-output" )

## Directory for generated man pages
set (MAN_BUILD_DIR       "${PROJECT_BINARY_DIR}/generated-manpages" )


## Compound target for creating the manpages (conatins all steps)
add_custom_target (provide_manpages ALL )


## --- Generate the dox file sources from the .dox.in files

## List of cmake-controlled manpage source files
file (GLOB MAN_SOURCE_TEMPLATES ${MAN_TEXTS_DIR}/*.dox.in )

if (${CMAKE_VERSION} VERSION_LESS "3.14.0" ) ## No NAME_WLE available

	foreach (_form IN LISTS MAN_SOURCE_TEMPLATES )

		get_filename_component(_form_NAME ${_form} NAME_WE )
		configure_file (${_form} "${MAN_SRC_DIR}/${_form_NAME}.dox" )
	endforeach()

else ()

	foreach (_form IN LISTS MAN_SOURCE_TEMPLATES )

		get_filename_component(_form_OUTFILE ${_form} NAME_WLE )
		configure_file (${_form} "${MAN_SRC_DIR}/${_form_OUTFILE}" )
	endforeach()

endif()


## List of manpage source text files (those not controlled by cmake)
file (GLOB MAN_SOURCE_TEXTS ${MAN_TEXTS_DIR}/*.dox )

foreach (_doxfile IN LISTS MAN_SOURCE_TEXTS )

	get_filename_component(_doxfilename ${_doxfile} NAME )
	file (COPY ${_doxfile} DESTINATION "${MAN_SRC_DIR}" )
endforeach()


## --- Generate the man source files from the manpage source text files

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile-man.in ${DOXYFILE} @ONLY )



## --- Generate doxygen output directory

add_custom_command (
	OUTPUT  ${MAN_DOXYGEN_OUT_DIR}
	PRE_BUILD
	COMMAND ${CMAKE_COMMAND}
	ARGS    -E make_directory ${MAN_DOXYGEN_OUT_DIR} )


add_custom_target (create_man_dox_out_dir DEPENDS ${MAN_DOXYGEN_OUT_DIR} )



## --- Generate manpage output directory

add_custom_command (
	OUTPUT  ${MAN_BUILD_DIR}
	PRE_BUILD
	COMMAND ${CMAKE_COMMAND}
	ARGS    -E make_directory ${MAN_BUILD_DIR} )


add_custom_target (create_man_dir DEPENDS ${MAN_BUILD_DIR} )



# List of expected man pages (output of doxygen run)
set (MANPAGES )
foreach (_toolname IN LISTS TOOL_NAMES )

	list (APPEND MANPAGES "${MAN_DOXYGEN_OUT_DIR}/1/${_toolname}.1" )
endforeach()

add_custom_command (
	OUTPUT  ${MANPAGES}
	COMMAND Doxygen::doxygen ${DOXYFILE}
	DEPENDS ${MAN_SOURCE_TEMPLATES} ${MAN_SOURCE_TEXTS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	COMMENT "Generate manpages with Doxygen"
	VERBATIM
)

## Generate the manpages from text files
add_custom_target (generate_manpages DEPENDS ${MANPAGES} )
add_dependencies  (generate_manpages create_man_dir create_man_dox_out_dir )


## Move the relevant files of the output to target directory

foreach (_toolname IN LISTS TOOL_NAMES )

	set (MANPAGE "${MAN_DOXYGEN_OUT_DIR}/1/${_toolname}.1" )

	add_custom_command (
		OUTPUT "${MAN_BUILD_DIR}/${_toolname}.1"
		COMMAND ${CMAKE_COMMAND} -E copy "${MANPAGE}"
			"${MAN_BUILD_DIR}/${_toolname}.1"
		DEPENDS "${MAN_DOXYGEN_OUT_DIR}/1/${_toolname}.1" ${MAN_BUILD_DIR}
		COMMENT "Copy ${_toolname}.1 to build directory"
		VERBATIM
	)

	add_custom_target (copy_${_toolname}.1
		## THIS dependency is the reason to keep the uncompressed sources!
		DEPENDS "${MAN_BUILD_DIR}/${_toolname}.1"
	)

	## COMMENTED OUT: seems redundant, calls generate_manpages for every tool
	#add_dependencies (copy_${_toolname}.1  generate_manpages )
	add_dependencies (provide_manpages     copy_${_toolname}.1 )
endforeach()

set (INSTALL_PAGES )

## --- Compress the generated manpages with gzip, iff gzip is available

if (GZIP_EXEC )

	include (FindPackageHandleStandardArgs )
	FIND_PACKAGE_HANDLE_STANDARD_ARGS (gzip DEFAULT_MSG GZIP_EXEC )

	mark_as_advanced (GZIP_EXEC )

	macro (add_zip_command IN_FILE SRC_DIR )

		add_custom_command (
			OUTPUT "${IN_FILE}.gz"
			COMMAND ${GZIP_EXEC} --force --best --keep ${IN_FILE}
			WORKING_DIRECTORY "${SRC_DIR}"
			COMMENT "Compress ${IN_FILE}"
			DEPENDS ${IN_FILE}
		)
	endmacro()

	## Add a compress command for each tool page

	foreach (_toolname IN LISTS TOOL_NAMES )

		add_zip_command ("${MAN_BUILD_DIR}/${_toolname}.1"
			"${MAN_BUILD_DIR}" )

		add_custom_target (compress_${_toolname}
			DEPENDS "${MAN_BUILD_DIR}/${_toolname}.1.gz"
		)

		list (APPEND INSTALL_PAGES
			"${MAN_BUILD_DIR}/${_toolname}.1.gz" )

		add_dependencies (compress_${_toolname} copy_${_toolname}.1 )
		add_dependencies (provide_manpages  compress_${_toolname} )
	endforeach()

else() ## If gzip is not available, install man pages uncompressed

	message (WARNING "Gzip not found. Manpages will not be compressed." )

	add_dependencies (provide_manpages generate_manpages )

	foreach (_toolname IN LISTS TOOL_NAMES )

		list (APPEND INSTALL_PAGES "${MAN_BUILD_DIR}/${_toolname}.1" )
	endforeach()

endif (GZIP_EXEC)


## --- Specify manpage installation

if (NOT SKIP_INSTALL_ALL AND NOT SKIP_INSTALL_FILES )

	install (FILES ${INSTALL_PAGES}
		DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/man/man1" )

endif()

add_custom_target (man )
add_dependencies  (man provide_manpages )

