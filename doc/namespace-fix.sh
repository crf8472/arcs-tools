#!/usr/bin/env bash
##
## Insert inline namespace name to all classes where doxygen 1.13.2 ignored it.

## Names of the classes known to be broken
CLASSES=( 'WithInternalFlags' 'VerifyTableCreator' 'VERIFY' )
NS='arcsapp'

PREFIX="generated-docs/mcss/xml/class${NS}_1_1v__1__0__0_1_1"

for CLASS in ${CLASSES[@]}; do
	FILE="${PREFIX}${CLASS}.xml"
	printf "Patch %s\n" "${FILE}"
	sed -i "s/\(${NS}\)::\(${CLASS}\)/\1::v_1_0_0::\2/g" "${FILE}"
done

