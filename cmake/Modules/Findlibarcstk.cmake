## libarcstk_FOUND
## libarcstk_VERSION
## libarcstk_INCLUDE_DIR
## libarcstk_LIBRARIES

if (LIBARCSTK_VERSION AND LIBARCSTK_INCLUDE_DIR AND LIBARCSTK_LIBRARY )

	## already in cache
	set(libarcstk_FIND_QUIETLY TRUE )
endif ()

## Try to use pkg-config to generate path hints

find_package (PkgConfig QUIET )

if (PkgConfig_FOUND )

	pkg_check_modules (PC_LIBARCSTK QUIET libarcstk )

	set (LIBARCSTK_VERSION ${PC_LIBARCSTK_VERSION} )
else()

	message (WARNING "You will need pkg-config to correctly detect libarcstk" )
endif ()


find_path (LIBARCSTK_INCLUDE_DIR
	NAME "arcstk/calculate.hpp"
	PATHS
		${PC_LIBARCSTK_INCLUDEDIR}
		${PC_LIBARCSTK_INCLUDE_DIRS}
	DOC "Headers of libarcstk"
)

find_library (LIBARCSTK_LIBRARY
	NAMES "libarcstk" "arcstk"
	PATHS
		${PC_LIBARCSTK_LIBDIR}
		${PC_LIBARCSTK_LIBRARY_DIRS}
	DOC "Link library for libarcstk"
)

mark_as_advanced (
	LIBARCSTK_INCLUDE_DIR
	LIBARCSTK_LIBRARY
	LIBARCSTK_VERSION
)

include (FindPackageHandleStandardArgs )

find_package_handle_standard_args (libarcstk
	REQUIRED_VARS LIBARCSTK_LIBRARY LIBARCSTK_VERSION LIBARCSTK_INCLUDE_DIR
	VERSION_VAR   LIBARCSTK_VERSION
)

## Export oldstyle variables for backwards compatibility
if (libarcstk_FOUND )
	set (libarcstk_VERSION      "${LIBARCSTK_VERSION}" )
	set (libarcstk_LIBRARIES    "${LIBARCSTK_LIBRARY}" )
	set (libarcstk_INCLUDE_DIRS "${LIBARCSTK_INCLUDE_DIR}" )
endif()

## Export "modern" target
## Can be used more or less as if CONFIG whould have been used instead of MODULE
## (has not Release/Debug configuration support though)
add_library (libarcstk::libarcstk SHARED IMPORTED )

set_target_properties (libarcstk::libarcstk
	PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES "${libarcstk_INCLUDE_DIRS}"
	INTERFACE_LINK_LIBRARIES      "${libarcstk_LIBRARIES}" ## TODO Necessary ?
	IMPORTED_LOCATION             "${libarcstk_LIBRARIES}"
)
