## CMakeLists.txt for configuring and running tests

cmake_minimum_required (VERSION 3.7 )

## Always link library files by full path when a full path is given to the
## target_link_libraries() command.
cmake_policy (SET CMP0003 NEW )

## Link libraries by full path even in implicit directories.
cmake_policy (SET CMP0060 NEW )


## Directory for test sources
set (TEST_SOURCE_DIR "src" )


## --- Declare fixtures

## List of executable tests that do NOT create temporary files
set (TEST_SETS )

## default tests
list (APPEND TEST_SETS clitokens   )
list (APPEND TEST_SETS layouts     )
list (APPEND TEST_SETS tools-fs    )
list (APPEND TEST_SETS tools-calc  )
list (APPEND TEST_SETS tools-parse )
list (APPEND TEST_SETS table       )
## custom test scripts
list (APPEND TEST_SETS config      )
list (APPEND TEST_SETS format      )


## --- Add common configuration for all testsuites (catch2 + project deps)

foreach (_testcase ${TEST_SETS} )

	## Define an executable for each single testfile

	add_executable (${_testcase}_test
		"${TEST_SOURCE_DIR}/${_testcase}.cpp"
	)

	set_property (TARGET ${_testcase}_test PROPERTY CXX_STANDARD 17 )

	target_include_directories (${_testcase}_test
		PRIVATE ${PROJECT_SOURCE_DIR}         ## project headers
		PRIVATE ${PROJECT_BUILD_SOURCE_DIR}   ## generated sources
	)

	target_link_libraries (${_testcase}_test
		PRIVATE
		Catch2::Catch2WithMain
		libarcstk::libarcstk
		libarcsdec::libarcsdec
		-Wl,--disable-new-dtags  ## set RPATH instead of RUNPATH
		$<TARGET_OBJECTS:${PROJECT_NAME}-objects>
		## libarcstk and libarcsdec from build-tree, 0003 and 0060 are used
	)
	## Setting the RPATH forces loading the libarcstk.so from the build tree,
	## even if a system-wide libarcstk is installed. Using RUNPATH instead of
	## RPATH causes the testcase to load the system-wide libfile.

	## Add test to ctest set

	if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/run_${_testcase}_test.cmake)

		##  Found custom script file: test defined by custom script

		add_test (
			NAME ${_testcase}_test_script
			COMMAND ${CMAKE_COMMAND}
				-DOUTFILE=${PROJECT_BINARY_DIR}/report.${_testcase}.xml
				-DREPORT=junit
				-DBIN_DIR=${CMAKE_CURRENT_BINARY_DIR}
				-DDATA_DIR=${CMAKE_CURRENT_SOURCE_DIR}/data
				-P ${CMAKE_CURRENT_SOURCE_DIR}/run_${_testcase}_test.cmake
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data
		)

	else()

		## default test

		add_test (
			NAME    ${_testcase}_test
			COMMAND ${_testcase}_test
				-o "${PROJECT_BINARY_DIR}/report.${_testcase}.xml"
				-r junit
			WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/data"
		)
	endif()

endforeach()

