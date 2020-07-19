## CMakeLists.txt for configuring and running tests

cmake_minimum_required (VERSION 3.7 )


## --- Global settings

## Directory for test sources
set (TEST_SOURCE_DIR "src" )

## Main test file (compiled only once, reused for each testcase)
set (MAIN_TEST_CPP "${TEST_SOURCE_DIR}/main.test.cpp" )


## --- Declare fixtures

## List of executable tests
set (TEST_SETS )

list (APPEND TEST_SETS clitokens   )
list (APPEND TEST_SETS config      )
list (APPEND TEST_SETS layouts     )
list (APPEND TEST_SETS tools-fs    )
list (APPEND TEST_SETS tools-calc  )
list (APPEND TEST_SETS tools-parse )


## --- Provide main object file

add_library (catch2_main OBJECT "${MAIN_TEST_CPP}" )
set_property (TARGET catch2_main PROPERTY CXX_STANDARD 17 )
target_link_libraries (catch2_main Catch2::Catch2 )


## --- Add common configuration for all testsuites (catch2 + project deps)

foreach (_testcase ${TEST_SETS} )

	## Define an executable for each single testfile

	add_executable (${_testcase}_test
		$<TARGET_OBJECTS:catch2_main>
		"${TEST_SOURCE_DIR}/${_testcase}.cpp" )

	target_include_directories (${_testcase}_test
		PRIVATE ${PROJECT_SOURCE_DIR}         ## project headers
		PRIVATE ${PROJECT_BUILD_SOURCE_DIR} ) ## generated sources

	target_link_libraries (${_testcase}_test
		Catch2::Catch2
		libarcstk::libarcstk
		libarcsdec::libarcsdec
		$<TARGET_OBJECTS:${PROJECT_NAME}-objects> )


	## Compile tests as C++17
	set_property (TARGET ${_testcase}_test PROPERTY CXX_STANDARD 17 )

	## Add test to ctest set
	add_test (
		NAME    ${_testcase}_test
		COMMAND ${_testcase}_test
			-o "${PROJECT_LOG_DIR}/report.${_testcase}.xml"
			-r junit
			WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/data"
	)

endforeach()

