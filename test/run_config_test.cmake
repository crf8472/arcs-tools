## Run config_test and delete temporary logfile

execute_process (
	COMMAND ${BIN_DIR}/config_test -o ${OUTFILE} -r ${REPORT}
	TIMEOUT 1000 ## choose less amount than specified in add_test
	RESULT_VARIABLE status
	WORKING_DIRECTORY ${DATA_DIR}
)

file (REMOVE logfile )

if(status)
	MESSAGE(FATAL_ERROR "Test executing status: ${status}" )
endif()

