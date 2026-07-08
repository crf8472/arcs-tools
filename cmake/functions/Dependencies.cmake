## arcstk: Root CMake file for managing arcs-tools dependencies
## vim:fdm=marker

function (arcstk_setup_required_dependencies )

if (WITH_SUBMODULES )

	## Avoid WITH_TESTS just falling through to libarcsdec + libarcstk
	option (WITH_LIBARCSTK_TESTS  "Build unit tests of libarcstk"  OFF )
	option (WITH_LIBARCSDEC_TESTS "Build unit tests of libarcsdec" OFF )

	if (NOT HAS_PARENT )

		message (STATUS "Link to local submodules libarcstk and libarcsdec" )

		## This signals libarcsdec's CMakeLists.txt where to find it's libarcstk
		## depencency.
		set (SUBMODULES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs" )

		## backup original options
		set (WITH_ARCSTK_TESTS ${WITH_TESTS}
			CACHE BOOL "Backup value of CLI option -DWITH_TESTS" FORCE )

		## override + deactivate options for libarcstk
		set (WITH_TESTS ${WITH_LIBARCSTK_TESTS}
			CACHE BOOL "Override value of CLI option -DWITH_TESTS for libarcstk"
			FORCE )

		add_subdirectory (${SUBMODULES_DIR}/libarcstk  )

		## override + deactivate options for libarcsdec
		set (WITH_TESTS ${WITH_LIBARCSDEC_TESTS}
			CACHE BOOL "Override value of CLI option -DWITH_TESTS for libarcsdec"
			FORCE )

		add_subdirectory (${SUBMODULES_DIR}/libarcsdec )

		## restore original options
		set (WITH_TESTS ${WITH_ARCSTK_TESTS}
			CACHE BOOL "Build unit tests" FORCE )
	endif()
else()

	message (STATUS "Link to system-installed libarcstk and libarcsdec" )

	find_package (libarcstk  0.9.0 REQUIRED )
	find_package (libarcsdec 0.3.0 REQUIRED )
endif()


## --- Required: libarcstk

if (TARGET libarcstk::libarcstk )

	target_link_libraries (arcstk_objects PUBLIC libarcstk::libarcstk )
	# if (WITH_SUBMODULES )
	# 	# Force to set RPATH instead of RUNPATH, point to .so's in build tree.
	# 	# Note that this is for development and will not support installation.
	# 	target_link_libraries (arcstk_objects
	# 		PUBLIC libarcstk::libarcstk -Wl,--disable-new-dtags )
	# else()
	# 	target_link_libraries (arcstk_objects PUBLIC libarcstk::libarcstk )
	# endif()
else()
	message (FATAL_ERROR "libarcstk targets are not present" )
endif()


## --- Required: libarcsdec

if (TARGET libarcsdec::libarcsdec )

	target_link_libraries (arcstk_objects PUBLIC libarcsdec::libarcsdec )
	# if (WITH_SUBMODULES )
	# 	# Force to set RPATH instead of RUNPATH, point to .so's in build tree.
	# 	# Note that this is for development and will not support installation.
	# 	target_link_libraries (arcstk_objects
	# 		PUBLIC libarcsdec::libarcsdec -Wl,--disable-new-dtags )
	# else()
	# 	target_link_libraries (arcstk_objects PUBLIC libarcsdec::libarcsdec )
	# endif()
else()
	message (FATAL_ERROR "libarcsdec targets are not present" )
endif()

endfunction()

