#include "catch2/catch.hpp"

#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#include "layouts.hpp"
#endif



TEST_CASE ( "optimal_width()", "TODO" )
{
	// TODO
}


TEST_CASE ( "HexLayout", "[hexlayout]" )
{
	using arcsapp::Checksum;
	using arcsapp::HexLayout;

	auto hex_layout = std::make_unique<HexLayout>();

	auto foo = hex_layout->format(Checksum { 3456 }, 3);

	CHECK ( foo == "D80" );
}


TEST_CASE ( "InternalFlags", "" )
{
	using arcsapp::InternalFlags;

	InternalFlags flags(0);
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

	WithMetadataFlagMethods flags(true, true, true, false, false);

	REQUIRE (     flags.label()    );
	REQUIRE (     flags.track()    );
	REQUIRE (     flags.offset()   );
	REQUIRE ( not flags.length()   );
	REQUIRE ( not flags.filename() );

	WithMetadataFlagMethods flags2(false, false, false, true, false);

	REQUIRE ( not flags2.label()    );
	REQUIRE ( not flags2.track()    );
	REQUIRE ( not flags2.offset()   );
	REQUIRE (     flags2.length()   );
	REQUIRE ( not flags2.filename() );
	//REQUIRE ( flags2.only_one_flag() );
}


TEST_CASE ( "TypedColsTableBase", "TODO" )
{
	// TODO
}


TEST_CASE ( "TypedRowsTableBase", "TODO" )
{
	// TODO
}


TEST_CASE ( "StringTable", "[stringtable]" )
{
	using arcsapp::StringTable;

	StringTable table(2, 4, false, true); // no dynamic widths

	REQUIRE ( table.rows()    == 2 );
	REQUIRE ( table.columns() == 4 );

	// Set widths

	table.set_width(0, 5);
	table.set_width(1, 3);
	table.set_width(2, 4);
	table.set_width(3, 10);

	REQUIRE ( table.width(0) ==  5 );
	REQUIRE ( table.width(1) ==  3 );
	REQUIRE ( table.width(2) ==  4 );
	REQUIRE ( table.width(3) == 10 );

	// Set contents

	table.update_cell(0, 0, "foo");
	table.update_cell(0, 1, "bar");
	table.update_cell(0, 2, "baz");
	table.update_cell(0, 3, "357");

	table.update_cell(1, 0, "xyz");
	table.update_cell(1, 1, "fubi"); // exceeds col width
	table.update_cell(1, 2, "quux");
	table.update_cell(1, 3, "855");

	REQUIRE ( table.cell(0, 0) == "foo" );
	REQUIRE ( table.cell(0, 1) == "bar" );
	REQUIRE ( table.cell(0, 2) == "baz" );
	REQUIRE ( table.cell(0, 3) == "357" );

	REQUIRE ( table.cell(1, 0) == "xyz" );
	REQUIRE ( table.cell(1, 1) == "fu~" ); // cut off
	REQUIRE ( table.cell(1, 2) == "quux" );
	REQUIRE ( table.cell(1, 3) == "855" );

	SECTION ( "Copy construct" )
	{
		auto table_copy { table };

		CHECK ( table_copy.cell(0, 0) == "foo" );
		CHECK ( table_copy.cell(0, 1) == "bar" );
		CHECK ( table_copy.cell(0, 2) == "baz" );
		CHECK ( table_copy.cell(0, 3) == "357" );

		CHECK ( table_copy.cell(1, 0) == "xyz" );
		CHECK ( table_copy.cell(1, 1) == "fu~" ); // cut off
		CHECK ( table_copy.cell(1, 2) == "quux" );
		CHECK ( table_copy.cell(1, 3) == "855" );
	}

	SECTION ( "Move construct" )
	{
		auto table_moved { std::move(table) };

		CHECK ( table_moved.cell(0, 0) == "foo" );
		CHECK ( table_moved.cell(0, 1) == "bar" );
		CHECK ( table_moved.cell(0, 2) == "baz" );
		CHECK ( table_moved.cell(0, 3) == "357" );

		CHECK ( table_moved.cell(1, 0) == "xyz" );
		CHECK ( table_moved.cell(1, 1) == "fu~" ); // cut off
		CHECK ( table_moved.cell(1, 2) == "quux" );
		CHECK ( table_moved.cell(1, 3) == "855" );
	}
}


TEST_CASE ( "ARTripletLayout", "TODO" )
{
	using arcsapp::ARTripletLayout;

	ARTripletLayout lyt;
}


TEST_CASE ( "ARIdTableLayout", "TODO" )
{
	using arcsapp::ARIdTableLayout;
	//using ARID_FLAG = ARIdTableLayout::ARID_FLAG;

	ARIdTableLayout lyt(false, false, true, false, false, false, false, false);
}


TEST_CASE ( "CalcAlbumTableLayout", "TODO" )
{
	// TODO
}


TEST_CASE ( "CalcTracksTableLayout", "TODO" )
{
	// TODO
}


TEST_CASE ( "VerifyTableLayout", "TODO" )
{
	// TODO
}
