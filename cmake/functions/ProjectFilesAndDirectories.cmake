
## Source directory for markdown files
set (PROJECT_ROOT_DIR           "${CMAKE_CURRENT_SOURCE_DIR}" )

## Root directory for sources
set (PROJECT_SOURCE_DIR         "${CMAKE_CURRENT_SOURCE_DIR}/src" )

## Root directory for public/exported headers
set (PROJECT_LOCAL_INCLUDE_DIR  "${PROJECT_SOURCE_DIR}" )

## Root directory for out-of-source build
set (PROJECT_BINARY_DIR         "${CMAKE_CURRENT_BINARY_DIR}" )

## Target directory for non-cmake logs
set (PROJECT_LOG_DIR            "${PROJECT_BINARY_DIR}/log" )

## Target directory for generated sources and configuration
set (PROJECT_BUILD_SOURCE_DIR   "${PROJECT_BINARY_DIR}/generated-sources" )

