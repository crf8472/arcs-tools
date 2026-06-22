## CMake script for building a python sandbox
##
## vim:fdm=marker

cmake_minimum_required (VERSION 3.10 )

project (CMakeVirtualenv )


## --- Find python 3 {{{1

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.12" ) ## FindPython available

	find_package (Python3 COMPONENTS Interpreter Development REQUIRED )

	set(CALL_PYTHON "${Python3_EXECUTABLE}" )

else()

	find_package (PythonInterp 3.0 REQUIRED )

	set(CALL_PYTHON "${PYTHON_EXECUTABLE}" )

endif()


## --- Find virtualenv {{{1

find_program (VIRTUALENV virtualenv )

if (NOT VIRTUALENV )

    message (FATAL_ERROR "Could not find 'virtualenv' in $PATH" )
else()

	message (STATUS "Found 'virtualenv': ${VIRTUALENV}" )
endif()


## --- Create and setup an up-to-date virtual python environment {{{1

set (ENV_NAME   python-sandbox )
set (ENV_DIR    "${CMAKE_BINARY_DIR}/${ENV_NAME}" )


## --- Setup ENV_DIR as virtual environment directory {{{1
add_custom_command(
	OUTPUT   "${ENV_DIR}"
	COMMAND  "${VIRTUALENV}"
	ARGS     -p "${CALL_PYTHON}" "${ENV_DIR}"
	COMMENT  "Create python sandbox"
	VERBATIM
)
## Note:
## One could also do this by specifying the following:
##	COMMAND "${CALL_PYTHON}"
##	ARGS    -m virtualenv "${ENV_DIR}"
## But it can fail on systems with python 2 present. When loaded, virtualenv is
## allowed to change the python interpreter. At least on Bionic Beaver, it
## fell back to python 2.7 which caused the m.css run to fail on some enconding
## issue.

## --- Define target and Export Variables  {{{1
add_custom_target (arcstk_virtualenv_create_sandbox DEPENDS "${ENV_DIR}" )

## Export variables used in parent CMakeLists.txt
set (PYTHON_ENV_AVAILABLE "YES"                   PARENT_SCOPE )
set (PYTHON_ENV_DIRECTORY "${ENV_DIR}"            PARENT_SCOPE )
set (PYTHON_CMD           "${ENV_DIR}/bin/python" PARENT_SCOPE )
## 1}}}

## Install and Update custom requirements in the sandbox.
##
## This is provided for use in other CMakeLists.txt.
function (add_requirements_target UPDATE_TARGET REQUIREMENTS_FILE )

	add_custom_target (${UPDATE_TARGET} )

	add_custom_command(
		TARGET  ${UPDATE_TARGET} PRE_BUILD
		COMMAND "${PYTHON_ENV_DIRECTORY}/bin/pip"
		ARGS    install -r "${REQUIREMENTS_FILE}" --upgrade
		WORKING_DIRECTORY "${PYTHON_ENV_DIRECTORY}"
		COMMENT "Update requirements in python sandbox for ${UPDATE_TARGET}"
		VERBATIM
	)

	add_dependencies (${UPDATE_TARGET} arcstk_virtualenv_create_sandbox )
endfunction()

