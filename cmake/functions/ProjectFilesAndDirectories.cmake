## arcstk: Define project directory layout and paths
## vim:fdm=marker

## BUILD TREE

## Root directory for out-of-source build
set (ARCSTK_BINARY_DIR          "${CMAKE_CURRENT_BINARY_DIR}" )

## Target directory for non-cmake logs
set (ARCSTK_LOG_BINARY_DIR      "${ARCSTK_BINARY_DIR}/log" )

## Target directory for generated sources and configuration
set (ARCSTK_GENSRC_BINARY_DIR   "${ARCSTK_BINARY_DIR}/generated-sources" )


## SOURCE TREE

## Source directory for markdown files
set (ARCSTK_ROOT_DIR            "${CMAKE_CURRENT_SOURCE_DIR}" )

## Root directory for sources
set (ARCSTK_SOURCE_DIR          "${CMAKE_CURRENT_SOURCE_DIR}/src" )

## Root directory for public/exported headers
set (ARCSTK_INCLUDE_SOURCE_DIR  "${ARCSTK_SOURCE_DIR}" )

