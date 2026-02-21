## CMake script for inserting missing inline namespace to some compounds' names
##
## vim:fdm=marker

cmake_minimum_required (VERSION 3.12 )

function (register_compound-fix_vns XML_INPUT_PATH )

	message (STATUS
		"Add doxygen Task: fix inline namespaces in compound names" )

	set (_sep_     "_1_1" )
	set (_arcsapp_ "arcsapp")
	set (_v100_    "v__1__0__0" )
	set (_arcsapp_v100_ "${_arcsapp_}${_sep_}${_v100_}${_sep_}" )

	## refids of the compounds to fix
	set (_XML_COMPOUNDS
		"class${_arcsapp_v100_}VERIFY" ;
		"class${_arcsapp_v100_}VerifyTableCreator" ;
		"namespace${_arcsapp_v100_}valid" ;
	)

	## Expand the compounds' refids to XML filenames
	list (TRANSFORM _XML_COMPOUNDS REPLACE "(.+)" "${XML_INPUT_PATH}/\\1.xml" )

	add_doxygen_xslt_task ("compound-fix_vns" "${_XML_COMPOUNDS}"
		"${XML_INPUT_PATH}" )

	unset (_XML_COMPOUNDS )
	unset (_sep_ )
	unset (_arcsapp_ )
	unset (_v100_ )
	unset (_arcsapp_v100_ )

endfunction()

