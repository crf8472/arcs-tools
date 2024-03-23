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
list (APPEND TEST_SETS config      ) ## custom test script
list (APPEND TEST_SETS layouts     )
list (APPEND TEST_SETS table       )
list (APPEND TEST_SETS tools-arid  )
list (APPEND TEST_SETS tools-calc  )
list (APPEND TEST_SETS tools-fs    )
#list (APPEND TEST_SETS tools-parse )
list (APPEND TEST_SETS tools-table )


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

	if (WITH_SUBMODULES )
		target_link_libraries (${_testcase}_test
			PRIVATE
			Catch2::Catch2WithMain
			libarcstk::libarcstk
			libarcsdec::libarcsdec
			-Wl,--disable-new-dtags ## set RPATH instead of RUNPATH
			$<TARGET_OBJECTS:${PROJECT_NAME}-objects>
		)
		## Setting RPATH forces loading the lib*.so files from the build tree,
		## even if system wide installed versions are present.
	else()
		target_link_libraries (${_testcase}_test
			PRIVATE
			Catch2::Catch2WithMain
			libarcstk::libarcstk
			libarcsdec::libarcsdec
			$<TARGET_OBJECTS:${PROJECT_NAME}-objects>
		)
		## Link against system wide binaries
	endif()

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

