#include "catch2/catch_test_macros.hpp"

#ifndef __ARCSTOOLS_TABLE_HPP__
#include "table.hpp"
#endif


TEST_CASE ( "optimal_width()", "" )
{
	using arcsapp::optimal_width;
	using std::vector;
	using std::string;

	CHECK ( 4 == optimal_width(vector<string>{"foo1", "bar", "quux"}) );
	CHECK ( 4 == optimal_width(vector<string>{"foo", "bar", "quux"}) );
	CHECK ( 1 == optimal_width(vector<string>{"a", "b", ""}) );
	CHECK ( 0 == optimal_width(vector<string>{"", "", ""}) );
}


TEST_CASE ( "StringTable", "[stringtable]" )
{
	using arcsapp::StringTable;

	StringTable table(2, 4, true);

	REQUIRE ( table.rows()    == 2 );
	REQUIRE ( table.columns() == 4 );

	REQUIRE ( not table.has_dynamic_width(0) );
	REQUIRE ( not table.has_dynamic_width(1) );
	REQUIRE ( not table.has_dynamic_width(2) );
	REQUIRE ( not table.has_dynamic_width(3) );

	// Set widths

	table.set_width(0, 5); // no dynamic widths
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
	REQUIRE ( table.cell(1, 1) == "fubi" ); // cut off in print: fub~
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
		CHECK ( table_copy.cell(1, 1) == "fubi" ); // cut off in print: fub~
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
		CHECK ( table_moved.cell(1, 1) == "fubi" ); // cut off in print: fub~
		CHECK ( table_moved.cell(1, 2) == "quux" );
		CHECK ( table_moved.cell(1, 3) == "855" );
	}

	SECTION ( "Copy assignment" )
	{
		auto table_copy = table;

		CHECK ( table_copy.cell(0, 0) == "foo" );
		CHECK ( table_copy.cell(0, 1) == "bar" );
		CHECK ( table_copy.cell(0, 2) == "baz" );
		CHECK ( table_copy.cell(0, 3) == "357" );

		CHECK ( table_copy.cell(1, 0) == "xyz" );
		CHECK ( table_copy.cell(1, 1) == "fubi" ); // cut off in print: fub~
		CHECK ( table_copy.cell(1, 2) == "quux" );
		CHECK ( table_copy.cell(1, 3) == "855" );
	}

	SECTION ( "Move assignment" )
	{
		auto table_moved = std::move(table);

		CHECK ( table_moved.cell(0, 0) == "foo" );
		CHECK ( table_moved.cell(0, 1) == "bar" );
		CHECK ( table_moved.cell(0, 2) == "baz" );
		CHECK ( table_moved.cell(0, 3) == "357" );

		CHECK ( table_moved.cell(1, 0) == "xyz" );
		CHECK ( table_moved.cell(1, 1) == "fubi" ); // cut off in print: fub~
		CHECK ( table_moved.cell(1, 2) == "quux" );
		CHECK ( table_moved.cell(1, 3) == "855" );
	}
}

