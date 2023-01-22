## CMake file for single-header version of Catch2

cmake_minimum_required (VERSION 3.0.2 )

find_package (Git REQUIRED )

## --- Catch2 identifiers

set (CATCH2_REPO_URL    "https://github.com/catchorg/Catch2.git" )
set (CATCH2_VERSION_TAG "v3.1.0" )

## --- Define Catch2 to be checked out/updated while configuring

if (${CMAKE_VERSION} VERSION_LESS "3.11.0" ) ## No FetchContent module available

	include (ExternalProject )

	ExternalProject_Add(
		catch2
		GIT_REPOSITORY    ${CATCH2_REPO_URL}
		GIT_TAG           ${CATCH2_VERSION_TAG}
		GIT_SHALLOW       true
		GIT_PROGRESS      true
		TIMEOUT           10
		UPDATE_COMMAND    ${GIT_EXECUTABLE} pull
		CONFIGURE_COMMAND ""
		BUILD_COMMAND     ""
		INSTALL_COMMAND   ""
		LOG_DOWNLOAD      ON
	)

	ExternalProject_Get_Property(catch2 SOURCE_DIR BINARY_DIR TMP_DIR )

	## --- Actually fetch Catch2 using the config created by ExternalProject_Add
	execute_process(
		COMMAND ${CMAKE_COMMAND} -P "${TMP_DIR}/catch2-gitclone.cmake"
		WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
	)

	## --- Load Catch2 CMake config
	add_subdirectory ("${SOURCE_DIR}" "${BINARY_DIR}" )

	unset (SOURCE_DIR )
	unset (BINARY_DIR )
	unset (TMP_DIR )

else()

	## --- Fetch Catch2

	include (FetchContent )

	FetchContent_Declare(
		catch2
		GIT_REPOSITORY    ${CATCH2_REPO_URL}
		GIT_TAG           ${CATCH2_VERSION_TAG}
	)

	if (${CMAKE_VERSION} VERSION_LESS "3.14.0" )

		FetchContent_GetProperties (catch2 )
		if (NOT catch2_POPULATED )
			FetchContent_Populate (catch2 )
			add_subdirectory ("${catch2_SOURCE_DIR}" "${catch2_BINARY_DIR}" )
		endif()
	else()

		FetchContent_MakeAvailable(catch2)
	endif()

endif()

