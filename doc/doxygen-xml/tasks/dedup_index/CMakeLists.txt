## CMake script for deduplicating doxygen's index.xml
##
## vim:fdm=marker

## In index.xml, every class in an inline namespace occurrs twice: once as
## member of the owning namespace where they are inlined to and a second time as
## members of the specific inline namespace. It did not find a way to suppress
## or avoid this.
## https://github.com/doxygen/doxygen/issues/5914
## https://github.com/doxygen/doxygen/pull/10070

cmake_minimum_required (VERSION 3.12 )

function (register_dedup_index XML_INPUT_PATH )

	message (STATUS "Doxygen Task: doxygen requires index deduplication" )

	add_doxygen_xslt_task ("dedup_index" "${XML_INPUT_PATH}/index.xml" )

endfunction()

