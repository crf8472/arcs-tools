## arcstk: Root CMake file for managing arcs-tools dependencies
## vim:fdm=marker

function (arcstk_setup_required_dependencies )

if (WITH_SUBMODULES )

	if (NOT HAS_PARENT )
		message (STATUS "Link to local submodules libarcstk and libarcsdec" )

		## This signals libarcsdec's CMakeLists.txt where to find it's libarcstk
		## depencency.
		set (SUBMODULES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs" )

		add_subdirectory (${SUBMODULES_DIR}/libarcstk  )
		add_subdirectory (${SUBMODULES_DIR}/libarcsdec )

		## Commented out: try to force RPATH to build tree .so's but no success
		#set (CMAKE_SKIP_BUILD_RPATH            FALSE )
		#set (CMAKE_BUILD_WITH_INSTALL_RPATH    FALSE )
		#set (CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE  )
	endif()
else()

	message (STATUS "Link to system-installed libarcstk and libarcsdec" )

	find_package (libarcstk  0.9.0 REQUIRED )
	find_package (libarcsdec 0.3.0 REQUIRED )
endif()


## --- Required: libarcstk

if (TARGET libarcstk::libarcstk )

	if (WITH_SUBMODULES )
		# Force to set RPATH instead of RUNPATH, point to .so's in build tree.
		# Note that this is for development and will not support installation.
		target_link_libraries (arcstk_objects
			PUBLIC libarcstk::libarcstk -Wl,--disable-new-dtags )
	else()
		target_link_libraries (arcstk_objects PUBLIC libarcstk::libarcstk )
	endif()
else()
	message (FATAL_ERROR "libarcstk targets are not present" )
endif()


## --- Required: libarcsdec

if (TARGET libarcsdec::libarcsdec )

	if (WITH_SUBMODULES )
		# Force to set RPATH instead of RUNPATH, point to .so's in build tree.
		# Note that this is for development and will not support installation.
		target_link_libraries (arcstk_objects
			PUBLIC libarcsdec::libarcsdec -Wl,--disable-new-dtags )
	else()
		target_link_libraries (arcstk_objects PUBLIC libarcsdec::libarcsdec )
	endif()
else()
	message (FATAL_ERROR "libarcsdec targets are not present" )
endif()

endfunction()

