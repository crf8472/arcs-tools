## CMake script for adding inline namespace names doxygen's class*.xml files
##
## vim:fdm=marker

cmake_minimum_required (VERSION 3.12 )

function (register_add_inline_ns_names XML_INPUT_PATH )

	message (STATUS "Add doxygen Task: add inline namespace names to classes" )

	## Note that '_1_1' denotes '::' and each original underscore within a
	## namespace name is escaped with another underscore, thus
	## 'arcsdec::v_1_0_0::' becomes 'arcsdec_1_1v__1__0__0_1_1'
	set (_sep   "_1_1" )
	set (_v100 "arcsapp${_sep}v__1__0__0${_sep}" )

	## For template classes
	set (_langle "_3_01" )
	set (_rangle "_01_4" )
	#set (_ttype  "T_00_01" )
	#set (_tf     "F_00_01" )

	set (_XML_CLASSFILES
		## Namespaces
		# "${XML_INPUT_PATH}/namespace${_v100}ansi.xml"
		# "${XML_INPUT_PATH}/namespace${_v100}arid.xml"
		# "${XML_INPUT_PATH}/namespace${_v100}calc.xml"
		# "${XML_INPUT_PATH}/namespace${_v100}cli.xml"
		# "${XML_INPUT_PATH}/namespace${_v100}dbar.xml"
		# "${XML_INPUT_PATH}/namespace${_v100}details.xml"
		# "${XML_INPUT_PATH}/namespace${_v100}file.xml"
		# "${XML_INPUT_PATH}/namespace${_v100}info.xml"
		# "${XML_INPUT_PATH}/namespace${_v100}input.xml"
		# "${XML_INPUT_PATH}/namespace${_v100}table.xml"
		# "${XML_INPUT_PATH}/namespace${_v100}valid.xml"
		# "${XML_INPUT_PATH}/namespacearcsapp${_sep}v__1__0__0.xml"
		# "${XML_INPUT_PATH}/namespacearcsapp.xml"
		## Classes
		## Listed in tab 'Classes':
		# "${XML_INPUT_PATH}/class${_v100}ansi${_sep}Modifier.xml"
		# "${XML_INPUT_PATH}/class${_v100}ApplicationFactory.xml"
		# "${XML_INPUT_PATH}/class${_v100}Application.xml"
		# "${XML_INPUT_PATH}/class${_v100}ARCalcApplicationBase.xml"
		# "${XML_INPUT_PATH}/class${_v100}ARCalcApplication.xml"
		# "${XML_INPUT_PATH}/class${_v100}ARCalcConfiguratorBase.xml"
		# "${XML_INPUT_PATH}/class${_v100}ARCalcConfigurator.xml"
		# "${XML_INPUT_PATH}/class${_v100}arid${_sep}ARIdLayout.xml"
		# "${XML_INPUT_PATH}/class${_v100}arid${_sep}ARIdTableLayout.xml"
		# "${XML_INPUT_PATH}/class${_v100}arid${_sep}RichARId.xml"
		# "${XML_INPUT_PATH}/class${_v100}ARIdApplication.xml"
		# "${XML_INPUT_PATH}/class${_v100}ARIdConfigurator.xml"
		# "${XML_INPUT_PATH}/class${_v100}ARIdOptions.xml"
		# "${XML_INPUT_PATH}/class${_v100}ARParseApplication.xml"
		# "${XML_INPUT_PATH}/class${_v100}ARVerifyApplication.xml"
		# "${XML_INPUT_PATH}/class${_v100}ARVerifyConfigurator.xml"
		# "${XML_INPUT_PATH}/class${_v100}calc${_sep}ChecksumCalculator.xml"
		# "${XML_INPUT_PATH}/class${_v100}calc${_sep}HexLayout.xml"
		# "${XML_INPUT_PATH}/class${_v100}CalcTableCreator.xml"
		# "${XML_INPUT_PATH}/class${_v100}CALC.xml"
		# "${XML_INPUT_PATH}/class${_v100}CellDecorator.xml"
		# "${XML_INPUT_PATH}/class${_v100}cli${_sep}CallSyntaxException.xml"
		# "${XML_INPUT_PATH}/class${_v100}ColorizingVerifyTableCreator.xml"
		# "${XML_INPUT_PATH}/class${_v100}ColorRegistry.xml"
		# "${XML_INPUT_PATH}/class${_v100}ColorSpecParser.xml"
		# "${XML_INPUT_PATH}/class${_v100}ConfigurationException.xml"
		# "${XML_INPUT_PATH}/class${_v100}Configuration.xml"
		# "${XML_INPUT_PATH}/class${_v100}Configurator.xml"
		# "${XML_INPUT_PATH}/class${_v100}dbar${_sep}DBARTripletLayout.xml"
		# "${XML_INPUT_PATH}/class${_v100}dbar${_sep}PrintParseHandler.xml"
		# "${XML_INPUT_PATH}/class${_v100}DefaultConfigurator.xml"
		# "${XML_INPUT_PATH}/class${_v100}details${_sep}DecoratorRegistry.xml"
		# "${XML_INPUT_PATH}/class${_v100}FieldCreator.xml"
		# "${XML_INPUT_PATH}/class${_v100}FlagStore.xml"
		# "${XML_INPUT_PATH}/class${_v100}Flags.xml"
		# "${XML_INPUT_PATH}/class${_v100}info${_sep}AvailableFileReaders.xml"
		# "${XML_INPUT_PATH}/class${_v100}info${_sep}InfoResultComposer.xml"
		# "${XML_INPUT_PATH}/class${_v100}info${_sep}StringTableLayout.xml"
		# "${XML_INPUT_PATH}/class${_v100}info${_sep}StringTable.xml"
		# "${XML_INPUT_PATH}/class${_v100}input${_sep}ChecksumValuesParser.xml"
		# "${XML_INPUT_PATH}/class${_v100}input${_sep}ChecksumValuesSource.xml"
		# "${XML_INPUT_PATH}/class${_v100}input${_sep}DBARParser.xml"
		# "${XML_INPUT_PATH}/class${_v100}input${_sep}EmptyChecksumSource.xml"
		# "${XML_INPUT_PATH}/class${_v100}input${_sep}InputStringParser.xml"
		# "${XML_INPUT_PATH}/class${_v100}input${_sep}StdIn.xml"
		# "${XML_INPUT_PATH}/class${_v100}input${_sep}StringParser.xml"
		# "${XML_INPUT_PATH}/class${_v100}input${_sep}VectorIStream.xml"
		# "${XML_INPUT_PATH}/class${_v100}InputStringParser.xml"
		# "${XML_INPUT_PATH}/class${_v100}LabelStore.xml"
		# "${XML_INPUT_PATH}/class${_v100}Layout.xml"
		# "${XML_INPUT_PATH}/class${_v100}MatchDecorator.xml"
		# "${XML_INPUT_PATH}/class${_v100}MonochromeVerifyTableCreator.xml"
		# "${XML_INPUT_PATH}/class${_v100}Options.xml"
		# "${XML_INPUT_PATH}/class${_v100}Option.xml"
		# "${XML_INPUT_PATH}/class${_v100}Output.xml"
		# "${XML_INPUT_PATH}/class${_v100}PropertyStore.xml"
		# "${XML_INPUT_PATH}/class${_v100}RegisterApplicationType.xml"
		# "${XML_INPUT_PATH}/class${_v100}ResultBuffer.xml"
		# "${XML_INPUT_PATH}/class${_v100}ResultList.xml"
		# "${XML_INPUT_PATH}/class${_v100}ResultObject.xml"
		# "${XML_INPUT_PATH}/class${_v100}Result.xml"
		# "${XML_INPUT_PATH}/class${_v100}StringParser.xml"
		# "${XML_INPUT_PATH}/class${_v100}table${_sep}AddField${_langle}ATTR${_sep}CHECKSUM__ARCS1${_rangle}.xml"
		# "${XML_INPUT_PATH}/class${_v100}table${_sep}AddField${_langle}ATTR${_sep}CHECKSUM__ARCS2${_rangle}.xml"
		# "${XML_INPUT_PATH}/class${_v100}table${_sep}AddField${_langle}ATTR${_sep}FILENAME${_rangle}.xml"
		# "${XML_INPUT_PATH}/class${_v100}table${_sep}AddField${_langle}ATTR${_sep}LENGTH${_rangle}.xml"
		# "${XML_INPUT_PATH}/class${_v100}table${_sep}AddField${_langle}ATTR${_sep}OFFSET${_rangle}.xml"
		# "${XML_INPUT_PATH}/class${_v100}table${_sep}AddField${_langle}ATTR${_sep}THEIRS${_rangle}.xml"
		# "${XML_INPUT_PATH}/class${_v100}table${_sep}AddField${_langle}ATTR${_sep}TRACK${_rangle}.xml"
		# "${XML_INPUT_PATH}/class${_v100}table${_sep}AddField.xml"
		# "${XML_INPUT_PATH}/class${_v100}table${_sep}AddRecords.xml"
		# "${XML_INPUT_PATH}/class${_v100}table${_sep}ARIdLayout.xml"
		# "${XML_INPUT_PATH}/class${_v100}table${_sep}CellDecorator.xml"
		# "${XML_INPUT_PATH}/class${_v100}table${_sep}ColTableComposerBuilder.xml"
		# "${XML_INPUT_PATH}/class${_v100}table${_sep}ColTableComposer.xml"
		# "${XML_INPUT_PATH}/class${_v100}table${_sep}DecoratedStringTable.xml"
		# "${XML_INPUT_PATH}/class${_v100}table${_sep}DecorationInterface.xml"
		# "${XML_INPUT_PATH}/class${_v100}table${_sep}DefaultSplitter.xml"
		# "${XML_INPUT_PATH}/class${_v100}table${_sep}FieldCreator.xml"
		# "${XML_INPUT_PATH}/class${_v100}table${_sep}PrintableTable.xml"
		# "${XML_INPUT_PATH}/class${_v100}table${_sep}RecordInterface.xml"
		# "${XML_INPUT_PATH}/class${_v100}table${_sep}RichARId.xml"
		# "${XML_INPUT_PATH}/class${_v100}table${_sep}RowTableComposerBuilder.xml"
		# "${XML_INPUT_PATH}/class${_v100}table${_sep}RowTableComposer.xml"
		# "${XML_INPUT_PATH}/class${_v100}table${_sep}StringSplitter.xml"
		# "${XML_INPUT_PATH}/class${_v100}table${_sep}StringTableLayout.xml"
		# "${XML_INPUT_PATH}/class${_v100}table${_sep}StringTable.xml"
		# "${XML_INPUT_PATH}/class${_v100}table${_sep}TableComposerBuilder.xml"
		# "${XML_INPUT_PATH}/class${_v100}table${_sep}TableComposer.xml"
		# "${XML_INPUT_PATH}/class${_v100}table${_sep}TableCreator.xml"
		# "${XML_INPUT_PATH}/class${_v100}table${_sep}TablePrinter.xml"
		# "${XML_INPUT_PATH}/class${_v100}TableComposer.xml"
		# "${XML_INPUT_PATH}/class${_v100}TableCreator.xml"
		# "${XML_INPUT_PATH}/class${_v100}valid${_sep}Validate.xml"              ## remove v_1_0_0::
		"${XML_INPUT_PATH}/class${_v100}VerifyTableCreator.xml"
		"${XML_INPUT_PATH}/class${_v100}VERIFY.xml"
		## Not required for success:
		#
		## Structs
		## Listed in tab 'Classes':
		# "${XML_INPUT_PATH}/struct${_v100}ApplicationFactory${_sep}RetrieveName.xml"
		# "${XML_INPUT_PATH}/struct${_v100}calc${_sep}IdSelection.xml"
		# "${XML_INPUT_PATH}/struct${_v100}calc${_sep}ToCFiles.xml"            ## do not modify
		# "${XML_INPUT_PATH}/struct${_v100}CALCBASE.xml"
		# "${XML_INPUT_PATH}/struct${_v100}cli${_sep}OP__VALUE.xml"
		# "${XML_INPUT_PATH}/struct${_v100}FORMATBASE.xml"
		# "${XML_INPUT_PATH}/struct${_v100}info${_sep}AudioFormatComposer.xml"
		# "${XML_INPUT_PATH}/struct${_v100}info${_sep}ToCFormatComposer.xml"   ## do not modify
		#"${XML_INPUT_PATH}/struct${_v100}OPTION.xml"                          ## do not modify
		## Not required for success:
	)

	add_doxygen_xslt_task ("add_inline_ns_names"
		"${_XML_CLASSFILES}" "${XML_INPUT_PATH}" )

	#unset (_ttype )
	#unset (_tf )
	unset (_rangle )
	unset (_langle )
	unset (_v100 )
	unset (_sep )

	unset (_XML_CLASSFILES )

	## TODO May be we can glob exactly the relevant files?
	## Like:
	##
	## file (GLOB _XMLFILES
	##	"${XML_INPUT_PATH}/structarcsdec*.xml"
	##	"${XML_INPUT_PATH}/classarcsdec*.xml")
	##
	## But at the time when this function is executed, they are not yet created.

endfunction()

