cmake_minimum_required (VERSION 3.6 )

project (CMakeVirtualenv )


## Find python 3

if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.12" ) ## FindPython available

	find_package (Python3 COMPONENTS Interpreter Development REQUIRED )

	set(CALL_PYTHON ${Python3_EXECUTABLE} )

else()

	find_package (PythonInterp 3.0 REQUIRED )

	set(CALL_PYTHON ${PYTHON_EXECUTABLE} )

endif()


## Find virtualenv

find_program (VIRTUALENV virtualenv )

if (NOT VIRTUALENV )

    message (FATAL_ERROR "Could not find 'virtualenv' in $PATH" )
else()

	message (STATUS "Found 'virtualenv': ${VIRTUALENV}" )
endif()


## Create and setup an up-to-date virtual python environment

set (ENV_NAME   python-sandbox )
set (ENV_DIR    "${CMAKE_BINARY_DIR}/${ENV_NAME}" )


## Setup ENV_DIR as virtual environment directory
add_custom_command(
	OUTPUT  "${ENV_DIR}"
	COMMAND ${VIRTUALENV}
	ARGS    -p ${CALL_PYTHON} "${ENV_DIR}"
	VERBATIM
	COMMENT "Create python sandbox"
)
## Note:
## One could also do this by specifying the following:
##	COMMAND ${CALL_PYTHON}
##	ARGS    -m virtualenv "${ENV_DIR}"
## But it can fail on systems with python 2 present. When loaded, virtualenv is
## allowed to change the python interpreter. At least on Bionic Beaver, it
## fell back to python 2.7 which caused the m.css run to fail on some enconding
## issue.

add_custom_target (arcstools_python-virtualenv DEPENDS "${ENV_DIR}" )

## Export variables used in parent CMakeLists.txt
set (PYTHON_ENV_AVAILABLE "YES"                   PARENT_SCOPE )
set (PYTHON_ENV_DIRECTORY "${ENV_DIR}"            PARENT_SCOPE )
set (PYTHON_CMD           "${ENV_DIR}/bin/python" PARENT_SCOPE )


## Macro to Install and Update custom requirements in the sandbox.
##
## This is provided for use in other CMakeLists.txt.
macro (add_requirements_target UPDATE_TARGET REQUIREMENTS_FILE )

	set (STAMP_FILE "${PYTHON_ENV_DIRECTORY}/${UPDATE_TARGET}.update.stamp" )

	add_custom_command(
		OUTPUT  "${STAMP_FILE}"
		DEPENDS "${REQUIREMENTS_FILE}"
		COMMAND ${PYTHON_ENV_DIRECTORY}/bin/pip install
		ARGS    -r "${REQUIREMENTS_FILE}" --upgrade
		COMMAND ${CMAKE_COMMAND}
		ARGS    -E touch "${STAMP_FILE}"
		WORKING_DIRECTORY "${PYTHON_ENV_DIRECTORY}"
		VERBATIM
		COMMENT "Update requirements in python sandbox for ${UPDATE_TARGET}"
	)

	add_custom_target (${UPDATE_TARGET} DEPENDS "${STAMP_FILE}" )
	unset (STAMP_FILE )

	add_dependencies (${UPDATE_TARGET} arcstools_python-virtualenv )
endmacro()

