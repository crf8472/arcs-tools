## CMakeLists.txt for building the documentation

find_package (Doxygen 1.8.0 COMPONENTS REQUIRED dot )


message (STATUS
	"Note: to build documentation do 'cmake --build . --target doc'" )


## Path to documentation texts
set (PROJECT_DOC_TEXTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/texts" )

## List of documentation text files
file (GLOB DOC_TEXTS ${PROJECT_DOC_TEXTS_DIR}/*.md )

## Sources for documentation (texts and headers)
set (DOC_SOURCES ${EXPORTED_HEADERS} ${DOC_TEXTS} )

## "Target directory for generated documentation"
set (DOC_GEN_DIR "${PROJECT_BINARY_DIR}/generated-docs" )

## The generated doxyfile to use
set (DOXYFILE ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile )

## Output directory for doxygen
set (DOXYGEN_OUTPUT_DIRECTORY "${DOC_GEN_DIR}/doxygen" )

## Generated index.html
set (INDEXFILE "${DOXYGEN_OUTPUT_DIRECTORY}/html/index.html" )



## --- Generate documentation output directory

add_custom_command (
	OUTPUT  ${DOC_GEN_DIR}
	PRE_BUILD
	COMMAND ${CMAKE_COMMAND}
	ARGS    -E make_directory ${DOC_GEN_DIR} )


add_custom_target (create_output_dir DEPENDS ${DOC_GEN_DIR} )


## --- Generate base doxyfile


configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile-doc.in ${DOXYFILE} @ONLY )

add_custom_command (
	OUTPUT "${INDEXFILE}"
	COMMAND Doxygen::doxygen "${DOXYFILE}"
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	DEPENDS ${DOXYFILE} ${DOC_SOURCES}
	COMMENT "Generating API documentation with Doxygen"
	VERBATIM
)

add_custom_target (doc DEPENDS "${INDEXFILE}" )
add_dependencies  (doc create_output_dir )

