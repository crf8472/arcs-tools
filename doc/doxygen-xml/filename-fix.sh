#!/usr/bin/env bash

## Remove references to inline namespace v_1_0_0 from XML filenames.
## Those filenames are used as refids within the XML documents.

xml_remove_vns()
{
	local FNAME="$1"
	local NEWFNAME="${FNAME/_1_1v__1__0__0/}"

	if [[ ! -e "${NEWFNAME}" ]] ; then
		mv -nf -- "${FNAME}" "${NEWFNAME}"
	fi
}

rm -f xml/namespacearcsapp_1_1v__1__0__0.xml

for FILE in xml/*.xml; do
	xml_remove_vns "${FILE}"
done

