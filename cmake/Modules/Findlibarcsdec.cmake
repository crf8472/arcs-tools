# libarcsdec_FOUND
# libarcsdec_VERSION
# libarcsdec_INCLUDE_DIRS
# libarcsdec_LIBRARIES

if (LIBARCSDEC_VERSION AND LIBARCSDEC_INCLUDE_DIR AND LIBARCSDEC_LIBRARY )

	## already in cache
	set(libarcsdec_FIND_QUIETLY TRUE )
endif ()

## Try to use pkg-config to generate path hints

find_package (PkgConfig QUIET )

if (PkgConfig_FOUND )

	pkg_check_modules (PC_LIBARCSDEC QUIET libarcsdec )

	set (LIBARCSDEC_VERSION ${PC_LIBARCSDEC_VERSION} )
else()

	message (WARNING "You will need pkg-config to correctly detect libarcsdec" )
endif ()


find_path (LIBARCSDEC_INCLUDE_DIR
	NAME "arcsdec/calculators.hpp"
	PATHS
		${PC_LIBARCSDEC_INCLUDEDIR}
		${PC_LIBARCSDEC_INCLUDE_DIRS}
	DOC "Headers of libarcsdec"
)

find_library (LIBARCSDEC_LIBRARY
	NAMES "libarcsdec" "arcsdec"
	PATHS
		${PC_LIBARCSDEC_LIBDIR}
		${PC_LIBARCSDEC_LIBRARY_DIRS}
	DOC "Link library for libarcsdec"
)

mark_as_advanced (
	LIBARCSDEC_INCLUDE_DIR
	LIBARCSDEC_LIBRARY
	LIBARCSDEC_VERSION
)

include (FindPackageHandleStandardArgs )

find_package_handle_standard_args (libarcsdec
	REQUIRED_VARS LIBARCSDEC_LIBRARY LIBARCSDEC_INCLUDE_DIR
	VERSION_VAR   LIBARCSDEC_VERSION
)

## Export oldstyle variables for backwards compatibility
if (libarcsdec_FOUND )
	set (libarcsdec_INCLUDE_DIRS ${LIBARCSDEC_INCLUDE_DIR} )
	set (libarcsdec_LIBRARIES    ${LIBARCSDEC_LIBRARY} )
	set (libarcsdec_VERSION      ${LIBARCSDEC_VERSION} )
endif()

## Export "modern" target
## Can be used more or less as if CONFIG whould have been used instead of MODULE
## (has not Release/Debug configuration support though)
add_library (libarcsdec::libarcsdec SHARED IMPORTED )

set_target_properties (libarcsdec::libarcsdec
	PROPERTIES
	INTERFACE_INCLUDE_DIRECTORIES "${libarcsdec_INCLUDE_DIRS}"
	INTERFACE_LINK_LIBRARIES      "${libarcsdec_LIBRARIES}" ## TODO Necessary ?
	IMPORTED_LOCATION             "${libarcsdec_LIBRARIES}"
)

