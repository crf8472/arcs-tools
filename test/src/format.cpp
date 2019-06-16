#include "catch2/catch.hpp"

#ifndef __ARCSTOOLS_FORMAT_HPP__
#include "format.hpp"
#endif


TEST_CASE ( "DefaultLines", "[defaultlines]" )
{
	SECTION ( "Construct DefaultLines" )
	{
		std::unique_ptr<DefaultLines> lines = std::make_unique<DefaultLines>();
	}
}


TEST_CASE ( "HexLayout", "[hexlayout]" )
{
	auto hex_layout = std::make_unique<HexLayout>();

	auto foo = hex_layout->format(3456, 3);

	CHECK ( foo == "d80" );
}


TEST_CASE ( "TableFormatBase", "[tableformatbase]" )
{
	StringTableBase table(2, 4);

	CHECK ( table.rows()    == 2 );
	CHECK ( table.columns() == 4 );


	table.set_column_width(0, 5);
	table.set_column_width(1, 3);
	table.set_column_width(2, 4);
	//table.set_column_width(3, 10); // default
	//
	CHECK ( table.column_width(0) == 5 );
	CHECK ( table.column_width(1) == 3 );
	CHECK ( table.column_width(2) == 4 );


	table.update_cell(0, 0, "foo");
	table.update_cell(0, 1, "bar");
	table.update_cell(0, 2, "baz");
	table.update_cell(0, 3, 357);
	table.update_cell(1, 0, "foo");
	table.update_cell(1, 1, "bar");
	table.update_cell(1, 2, "quux");

	auto hex_layout = std::make_unique<HexLayout>();
	hex_layout->set_show_base(true);
	table.register_layout(3, hex_layout.get());

	CHECK ( hex_layout->shows_base() );

	table.update_cell(1, 3, 855);


	CHECK ( table.cell(0, 0) == "foo  " );
	CHECK ( table.cell(0, 1) == "bar" );
	CHECK ( table.cell(0, 2) == "baz " );
	CHECK ( table.cell(0, 3) == "       357" );
	CHECK ( table.cell(1, 0) == "foo  " );
	CHECK ( table.cell(1, 1) == "bar" );
	CHECK ( table.cell(1, 2) == "quux" );
	CHECK ( table.cell(1, 3) == "000000x357" );  // == 0x357


	table.set_alignment(3, false);

	CHECK ( table.alignment(0) == true  );
	CHECK ( table.alignment(1) == true  );
	CHECK ( table.alignment(2) == true  );
	CHECK ( table.alignment(3) == false );


	auto lines { table.print() };

	CHECK ( lines->size() == 3 );
	// line 0 is the header
	CHECK ( lines->get(1) == "foo   bar baz         357 " ); // dec
	CHECK ( lines->get(2) == "foo   bar quux 000000x357 " ); // hex
}


