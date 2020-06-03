#include "catch2/catch.hpp"

#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#include "layouts.hpp"
#endif
#ifndef __ARCSTOOLS_OUTPUTFORMATS_HPP__
#include "printers.hpp"
#endif


TEST_CASE ( "HexLayout", "[hexlayout]" )
{
	using arcsapp::HexLayout;

	auto hex_layout = std::make_unique<HexLayout>();

	auto foo = hex_layout->format(3456, 3);

	CHECK ( foo == "d80" );
}


TEST_CASE ( "WithInternalFlags", "" )
{
	using arcsapp::WithInternalFlags;

	WithInternalFlags flags(0);
	flags.set_flag(4, true);

	REQUIRE ( not flags.flag(0) );
	REQUIRE ( not flags.flag(1) );
	REQUIRE ( not flags.flag(2) );
	REQUIRE ( not flags.flag(3) );
	REQUIRE ( flags.flag(4) );
	REQUIRE ( not flags.flag(5) );
	REQUIRE ( not flags.flag(6) );
	REQUIRE ( not flags.flag(7) );

	REQUIRE ( flags.only(4) );
	//REQUIRE ( flags.only_one_flag() );

	flags.set_flag(7, true);

	REQUIRE ( flags.flag(7) );
	REQUIRE ( not flags.only(4) );
}


TEST_CASE ( "WithMetadataFlagMethods", "" )
{
	using arcsapp::WithMetadataFlagMethods;

	WithMetadataFlagMethods flags(true, true, false, false);

	REQUIRE ( flags.track() );
	REQUIRE ( flags.offset() );
	REQUIRE ( not flags.length() );
	REQUIRE ( not flags.filename() );

	WithMetadataFlagMethods flags2(false, false, true, false);

	REQUIRE ( not flags2.track() );
	REQUIRE ( not flags2.offset() );
	REQUIRE ( flags2.length() );
	REQUIRE ( not flags2.filename() );
	//REQUIRE ( flags2.only_one_flag() );
}


TEST_CASE ( "ARIdTableFormat", "" )
{
	using arcsapp::ARIdTableFormat;

	ARIdTableFormat f(true, false, false, false, false, false);

	using ARID_FLAG = ARIdTableFormat::ARID_FLAG;

	REQUIRE ( f.rows() == 1 );
	REQUIRE ( f.columns() == 1 );
	REQUIRE ( f.has_only(ARID_FLAG::URL) );
	REQUIRE ( not f.has_only(ARID_FLAG::FILENAME) );
	REQUIRE ( not f.has_only(ARID_FLAG::TRACKS) );
	REQUIRE ( not f.has_only(ARID_FLAG::ID1) );
	REQUIRE ( not f.has_only(ARID_FLAG::ID2) );
	REQUIRE ( not f.has_only(ARID_FLAG::CDDBID) );
}


TEST_CASE ( "AlbumTableBase", "" )
{
	using arcsapp::WithMetadataFlagMethods;

	WithMetadataFlagMethods table(true, true, false, false);

	REQUIRE ( table.track() );
	REQUIRE ( table.offset() );
	REQUIRE ( not table.length() );
	REQUIRE ( not table.filename() );
}


TEST_CASE ( "StringTable", "[stringtable]" )
{
	using arcsapp::StringTable;

	StringTable table(2, 4, false, true); // no dynamic widths

	REQUIRE ( table.rows()    == 2 );
	REQUIRE ( table.columns() == 4 );

	// Set widths

	table.set_width(0, 5);
	table.set_width(1, 3); // smaller than text!
	table.set_width(2, 4);
	table.set_width(3, 10);

	CHECK ( table.width(0) ==  5 );
	CHECK ( table.width(1) ==  3 );
	CHECK ( table.width(2) ==  4 );
	CHECK ( table.width(3) == 10 );

	// Set contents

	table.update_cell(0, 0, "foo");
	table.update_cell(0, 1, "bar");
	table.update_cell(0, 2, "baz");
	table.update_cell(0, 3, "357");

	table.update_cell(1, 0, "xyz");
	table.update_cell(1, 1, "fubi");
	table.update_cell(1, 2, "quux");
	table.update_cell(1, 3, "855");

	CHECK ( table.cell(0, 0) == "foo" );
	CHECK ( table.cell(0, 1) == "bar" );
	CHECK ( table.cell(0, 2) == "baz" );
	CHECK ( table.cell(0, 3) == "357" );

	CHECK ( table.cell(1, 0) == "xyz" );
	CHECK ( table.cell(1, 1) == "fu~" ); // cut off
	CHECK ( table.cell(1, 2) == "quux" );
	CHECK ( table.cell(1, 3) == "855" );
}


