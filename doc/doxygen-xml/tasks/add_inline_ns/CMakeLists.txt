## CMake script for adding inline namespace names doxygen's index.xml
##
## vim:fdm=marker

cmake_minimum_required (VERSION 3.12 )

function (register_add_inline_ns XML_INPUT_PATH )

	message (STATUS "Add doxygen Task: add inline namespaces" )

	add_doxygen_xslt_task ("add_inline_ns" "${XML_INPUT_PATH}/index.xml" )

endfunction()

