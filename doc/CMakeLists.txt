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

## Generated index.html
set (INDEXFILE "${DOC_GEN_DIR}/html/index.html" )


configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile-doc.in ${DOXYFILE} @ONLY )

add_custom_command (
	OUTPUT "${INDEXFILE}"
	COMMAND cmake -E rm -r "${DOC_GEN_DIR}/html"
	COMMAND Doxygen::doxygen "${DOXYFILE}"
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	DEPENDS ${DOXYFILE} ${DOC_SOURCES}
	COMMENT "Generating API documentation with Doxygen"
	VERBATIM
)

add_custom_target (doc DEPENDS "${INDEXFILE}" )

