#include "catch2/catch.hpp"

#ifndef __ARCSTOOLS_FORMAT_HPP__
#include "format.hpp"
#endif


TEST_CASE ( "HexLayout", "[hexlayout]" )
{
	auto hex_layout = std::make_unique<HexLayout>();

	auto foo = hex_layout->format(3456, 3);

	CHECK ( foo == "d80" );
}


TEST_CASE ( "WithMetadataFlagMethods", "" )
{
	WithMetadataFlagMethods flags(true, true, false, false);

	REQUIRE ( flags.track() );
	REQUIRE ( flags.offset() );
	REQUIRE ( not flags.length() );
	REQUIRE ( not flags.filename() );
}


TEST_CASE ( "AlbumTableBase", "" )
{
	WithMetadataFlagMethods table(true, true, false, false);

	REQUIRE ( table.track() );
	REQUIRE ( table.offset() );
	REQUIRE ( not table.length() );
	REQUIRE ( not table.filename() );
}


TEST_CASE ( "StringTableBase", "[stringtablebase]" )
{
	StringTableBase table(2, 4);

	REQUIRE ( table.rows()    == 2 );
	REQUIRE ( table.columns() == 4 );

	// Set widths

	table.set_width(0, 5);
	table.set_width(1, 3);
	table.set_width(2, 4);
	//table.set_width(3, 10); // default

	CHECK ( table.width(0) == 5 );
	CHECK ( table.width(1) == 3 );
	CHECK ( table.width(2) == 4 );

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
	CHECK ( table.cell(1, 1) == "fubi" );
	CHECK ( table.cell(1, 2) == "quux" );
	CHECK ( table.cell(1, 3) == "855" );

	//table.set_alignment(3, false);

	//CHECK ( table.alignment(0) == true  );
	//CHECK ( table.alignment(1) == true  );
	//CHECK ( table.alignment(2) == true  );
	//CHECK ( table.alignment(3) == false );
}


