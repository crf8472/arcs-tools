## Find libflac++ and libflac
##
## Defines the following variables:
##
## LIBFLAC_INCLUDE_DIRS
## LIBFLACPP_INCLUDE_DIRS
## LIBFLAC_LIBRARIES
## LIBFLACPP_LIBRARIES
## FLAC_FOUND
## FLAC_LIBRARIES
## FLAC_INCLUDE_DIRS
## FLAC_VERSION

# Earlier Result for FLAC++ Already in CMake Cache?
if (LIBFLACPP_INCLUDE_DIR AND LIBFLACPP_LIBRARY )

	set(FLAC_FIND_QUIETLY TRUE )
endif ()

## 1: use pkg-config to generate hints

find_package (PkgConfig QUIET )

if (PkgConfig_FOUND )

	PKG_CHECK_MODULES (PC_LIBFLACPP QUIET flac++ )
	PKG_CHECK_MODULES (PC_LIBFLAC   QUIET flac   )

	set (FLAC_VERSION ${PC_LIBFLACPP_VERSION} )

else ()

	message (WARNING
		"Consider installing pkg-config to find FLAC correctly" )

endif (PkgConfig_FOUND )

## 2: find includes

find_path (LIBFLACPP_INCLUDE_DIR
	NAMES "FLAC++/decoder.h"
	PATHS
		${PC_LIBFLACPP_INCLUDEDIR}
		${PC_LIBFLACPP_INCLUDE_DIRS}
	DOC "Header path required to include <FLAC++/*.h>"
)

find_path (LIBFLAC_INCLUDE_DIR
	NAMES "FLAC/stream_decoder.h"
	PATHS
		${PC_LIBFLAC_INCLUDEDIR}
		${PC_LIBFLAC_INCLUDE_DIRS}
	DOC "Header path required to include <FLAC/*.h>"
)

## 3: find library

find_library (LIBFLACPP_LIBRARY
	NAMES FLAC++
	PATHS
		${PC_LIBFLACPP_LIBDIR}
		${PC_LIBFLACPP_LIBRARY_DIRS}
	DOC "Location of libflac++ wrapper libraries"
)

find_library (LIBFLAC_LIBRARY
	NAMES FLAC
	PATHS
		${PC_LIBFLAC_LIBDIR}
		${PC_LIBFLAC_LIBRARY_DIRS}
	DOC "Location of libflac libraries"
)

## 4: handle REQUIRED and QUIET options, set _FOUND VARIABLE

include (FindPackageHandleStandardArgs )

find_package_handle_standard_args (FLAC
	REQUIRED_VARS	LIBFLACPP_LIBRARY LIBFLACPP_INCLUDE_DIR
					LIBFLAC_LIBRARY LIBFLAC_INCLUDE_DIR
					FLAC_VERSION
	VERSION_VAR		FLAC_VERSION
    FAIL_MESSAGE	DEFAULT_MSG )

## 5: set declared variables

set (LIBFLACPP_INCLUDE_DIRS ${LIBFLACPP_INCLUDE_DIR} )
set (LIBFLACPP_LIBRARIES    ${LIBFLACPP_LIBRARY} )
set (LIBFLAC_INCLUDE_DIRS   ${LIBFLAC_INCLUDE_DIR} )
set (LIBFLAC_LIBRARIES      ${LIBFLAC_LIBRARY} )

set (FLAC_LIBRARIES    ${LIBFLACPP_LIBRARIES} ${LIBFLAC_LIBRARIES} )
set (FLAC_INCLUDE_DIRS ${LIBFLACPP_INCLUDE_DIRS} ${LIBFLAC_INCLUDE_DIRS} )

mark_as_advanced (
	LIBFLACPP_INCLUDE_DIR
	LIBFLACPP_LIBRARY
	LIBFLAC_INCLUDE_DIR
	LIBFLAC_LIBRARY
)

