#include "catch2/catch_test_macros.hpp"

#include <type_traits>  // for is_copy_constructible

#ifndef __ARCSTOOLS_TABLE_HPP__
#include "table.hpp"
#endif


TEST_CASE ( "StringTable", "[stringtable]" )
{
	using arcsapp::table::StringTable;
	using arcsapp::table::Align;

	StringTable t{ 5, 3 };

	t(0,0) = "foo";
	t(0,1) = "quux";
	t(0,2) = "bar";

	t(1,0) = "blubb";
	t(1,1) = "ti";
	t(1,2) = "ta";

	t(2,0) = "mor";
	t(2,1) = "quark";
	t(2,2) = "sem";

	t(3,0) = "trg";
	t(3,1) = "hkpn";
	t(3,2) = "tknr";

	t(4,0) = "plf";
	t(4,1) = "grb";
	t(4,2) = "st";

	t.set_align(1, Align::RIGHT);

	SECTION ( "Is copy-constructible" )
	{
		CHECK ( std::is_copy_constructible<StringTable>::value );
		//CHECK ( std::is_nothrow_copy_constructible<StringTable>::value );
	}

	SECTION ( "Is move-constructible" )
	{
		CHECK ( std::is_move_constructible<StringTable>::value );
		//CHECK ( std::is_trivially_move_constructible<StringTable>::value );
		//CHECK ( std::is_nothrow_move_constructible<StringTable>::value );
	}

	SECTION ( "Constructor parameter values are correctly initialized" )
	{
		CHECK ( 5 == t.rows() );
		CHECK ( 3 == t.cols() );
	}

	SECTION ( "Default values are correctly initialized" )
	{
		CHECK ( t.title().empty() );
		CHECK (  5 == t.default_max_height() );
		CHECK (  8 == t.default_max_width() );
		CHECK ( Align::LEFT  == t.align(0) );
		CHECK ( Align::RIGHT == t.align(1) );
		CHECK ( Align::LEFT  == t.align(2) );
	}

	SECTION ( "Setting and getting table title works" )
	{
		REQUIRE ( t.title().empty() );

		t.set_title("My Table");

		CHECK ( "My Table" == t.title() );
	}

	SECTION ( "Reading a specified legal cell with cell() works" )
	{
		CHECK ( "foo"   == t.cell(0, 0) );
		CHECK ( "quark" == t.cell(2, 1) );
		CHECK ( "hkpn"  == t.cell(3, 1) );
	}

	SECTION ( "Writing a specified legal cell value with set_cell() works" )
	{
		t.set_cell(1, 0, 127001);
		t.set_cell(2, 0, -5);
		t.set_cell(3, 0, 'r');

		CHECK ( "127001" == t(1, 0) );
		CHECK ( "-5"     == t(2, 0) );
		CHECK ( "r"      == t(3, 0) );
	}

	SECTION ( "Reading and writing a specified legal cell with cell() works" )
	{
		t.cell(1, 0) = "zappzerappi"; // greater than default_max_width()
		t.cell(3, 1) = "foo";

		CHECK ( "zappzerappi" == t.cell(1, 0) );
		CHECK ( "foo"         == t.cell(3, 1) );
	}

	SECTION ( "Reading a specified legal cell with operator () works" )
	{
		CHECK ( "foo"   == t(0, 0) );
		CHECK ( "quark" == t(2, 1) );
		CHECK ( "hkpn"  == t(3, 1) );
	}

	SECTION ( "Writing a specified legal cell with operator () works" )
	{
		t(1, 0) = "zappzerappi"; // greater than default_max_width()
		t(3, 1) = "foo";

		CHECK ( "zappzerappi" == t(1, 0) );
		CHECK ( "foo"         == t(3, 1) );
	}

	SECTION ( "Accessing a non-existent cell with operator () throws" )
	{
		REQUIRE ( !t.exists(5,0) ); // there is no row 5
									//
		CHECK_THROWS ( t(5,0) == "" );
	}

	SECTION ( "optimal_width() is correct" )
	{
		CHECK ( 5 == t.optimal_width(0) );
		CHECK ( 5 == t.optimal_width(1) );
		CHECK ( 4 == t.optimal_width(2) );
	}

	SECTION ( "Appending a row after the last row works" )
	{
		const auto not_a_row = 5;

		REQUIRE ( not_a_row == t.rows() ); // Guarantee to append new row

		t.cell(not_a_row, 0) = "cell in new row";
		t.cell(not_a_row, 1) = "second col";

		CHECK ( 6 == t.rows() );
		CHECK ( "cell in new row" == t(not_a_row, 0) );
		CHECK ( "second col"      == t(not_a_row, 1) );

		// Last row remains unmodified
		CHECK ( "plf" == t(4,0));
		CHECK ( "grb" == t(4,1));
		CHECK ( "st"  == t(4,2));
	}

	SECTION ( "Inserting rows after a row (not the last) works" )
	{
		REQUIRE ( 5 == t.rows());

		t.insert_rows_after(1, 2); // insert 1 row after row 2
		t(3,0) = "thr";
		t(3,1) = "tgl";
		t(3,2) = "arn";

		CHECK ( 6 == t.rows() );

		CHECK ( t(2,0) == "mor" );   // row 2 is unmodified
		CHECK ( t(2,1) == "quark" );
		CHECK ( t(2,2) == "sem" );

		CHECK ( t(3,0) == "thr" );   // row 3 is new
		CHECK ( t(3,1) == "tgl" );
		CHECK ( t(3,2) == "arn" );

		CHECK ( t(4,0) == "trg" );   // former row 3 is now row 4
		CHECK ( t(4,1) == "hkpn" );
		CHECK ( t(4,2) == "tknr" );

		CHECK ( t(5,0) == "plf" );   // former row 4 is now row 5 (last)
		CHECK ( t(5,1) == "grb" );
		CHECK ( t(5,2) == "st" );

		t.insert_rows_after(2, 3); // insert 2 rows after row 3
		t(4,0) = "ada";
		t(4,1) = "dot";
		t(4,2) = "algo";

		CHECK ( 8 == t.rows() );

		CHECK ( t(3,0) == "thr" );   // row 3 is unmodified
		CHECK ( t(3,1) == "tgl" );
		CHECK ( t(3,2) == "arn" );

		CHECK ( t(4,0) == "ada" );   // row 4 is new
		CHECK ( t(4,1) == "dot" );
		CHECK ( t(4,2) == "algo" );

		CHECK ( t(5,0) == "" );   // row 5 is new
		CHECK ( t(5,1) == "" );
		CHECK ( t(5,2) == "" );

		CHECK ( t(6,0) == "trg" );   // former row 4 is now row 6
		CHECK ( t(6,1) == "hkpn" );
		CHECK ( t(6,2) == "tknr" );

		CHECK ( t(7,0) == "plf" );   // former row 5 is now row 7 (last)
		CHECK ( t(7,1) == "grb" );
		CHECK ( t(7,2) == "st" );
	}

	SECTION ( "Appending a row to an empty table works" )
	{
		StringTable tbl{0,3};

		REQUIRE ( tbl.empty() );
		REQUIRE ( 0 == tbl.rows() );

		tbl.cell(0,0) = "foo";

		CHECK ( !tbl.empty() );
		CHECK ( 1 == tbl.rows() );

		tbl.cell(0,1) = "bar";
		tbl.cell(0,2) = "quux";

		CHECK ( 1 == tbl.rows() );
		CHECK ( "foo"  == tbl(0,0) );
		CHECK ( "bar"  == tbl(0,1) );
		CHECK ( "quux" == tbl(0,2) );
	}
}


TEST_CASE ( "String functions", "[stringfunctions]" )
{
	SECTION ( "Splitting into equal length parts works" )
	{
		using arcsapp::details::split;

		std::string str = "PCM, WAF, FOO,FLAC,RIFF,ANDSOON";

		const auto tokens { split(str, 9) };

		CHECK ( 4 == tokens.size() );
		CHECK ( "PCM, WAF," == tokens[0] );
		CHECK ( " FOO,FLAC" == tokens[1] );
		CHECK ( ",RIFF,AND" == tokens[2] );
		CHECK ( "SOON"      == tokens[3] );
	}

	SECTION ( "Trimming works" )
	{
		using arcsapp::details::trim;

		const auto s1 { trim(" Foo") };
		CHECK ( "Foo" == s1 );

		const auto s2 { trim("    Foo") };
		CHECK ( "Foo" == s2 );

		const auto s3 { trim("Foo ") };
		CHECK ( "Foo" == s3 );

		const auto s4 { trim("Foo     ") };
		CHECK ( "Foo" == s4 );

		const auto s5 { trim("Foo Bar") };
		CHECK ( "Foo Bar" == s5 );

		const auto s6 { trim("Foo   Bar  Quux") };
		CHECK ( "Foo   Bar  Quux" == s6 );

		const auto s7 { trim("  Foo   Bar  Quux   ") };
		CHECK ( "Foo   Bar  Quux" == s7 );
	}
}


TEST_CASE ( "DefaultSplitter", "[defaultsplitter]" )
{
	using arcsapp::table::DefaultSplitter;
}


TEST_CASE ( "StringTableLayout", "[stringtablelayout]" )
{
	using arcsapp::table::StringTableLayout;

	SECTION ( "Default Constructor works" )
	{
		StringTableLayout l;

		CHECK ( l.title() );
		CHECK ( l.row_labels() );
		CHECK ( l.col_labels() );

		CHECK ( !l.top_delims() );
		CHECK ( !l.row_header_delims() );
		CHECK ( !l.row_inner_delims() );
		CHECK ( !l.bottom_delims() );

		CHECK ( !l.left_outer_delims() );
		CHECK ( !l.col_labels_delims() );
		CHECK ( l.col_inner_delims() );
		CHECK ( !l.right_outer_delims() );

		CHECK ( "=" == l.top_delim() );
		CHECK ( "=" == l.row_header_delim() );
		CHECK ( "-" == l.row_inner_delim() );
		CHECK ( "=" == l.bottom_delim() );

		CHECK ( "|" == l.left_outer_delim() );
		CHECK ( "|" == l.col_labels_delim() );
		CHECK ( " " == l.col_inner_delim() );
		CHECK ( "|" == l.right_outer_delim() );

		CHECK ( l.splitter() != nullptr );
	}

	SECTION ( "Copy constructor works" )
	{
		StringTableLayout l;
		// TODO Change some values to others than default
		//l.set_title(false);
		//l.row_labels(false);
		//l.left_outer_delims(true);
		//l.col_labels_delims(true);
		//l.col_inner_delims(false);

		StringTableLayout m = l;

		CHECK ( m.title()      == l.title() );
		CHECK ( m.row_labels() == l.row_labels() );
		CHECK ( m.col_labels() == l.col_labels() );

		CHECK ( m.top_delims()        == l.top_delims() );
		CHECK ( m.row_header_delims() == l.row_header_delims() );
		CHECK ( m.row_inner_delims()  == l.row_inner_delims() );
		CHECK ( m.bottom_delims()     == l.bottom_delims() );

		CHECK ( m.left_outer_delims()  == l.left_outer_delims() );
		CHECK ( m.col_labels_delims()  == l.col_labels_delims() );
		CHECK ( m.col_inner_delims()   == l.col_inner_delims() );
		CHECK ( m.right_outer_delims() == l.right_outer_delims() );

		CHECK ( m.top_delim()        == l.top_delim() );
		CHECK ( m.row_header_delim() == l.row_header_delim() );
		CHECK ( m.row_inner_delim()  == l.row_inner_delim() );
		CHECK ( m.bottom_delim()     == l.bottom_delim() );

		CHECK ( m.left_outer_delim()  == l.left_outer_delim() );
		CHECK ( m.col_labels_delim()  == l.col_labels_delim() );
		CHECK ( m.col_inner_delim()   == l.col_inner_delim() );
		CHECK ( m.right_outer_delim() == l.right_outer_delim() );

		CHECK ( m.splitter() != nullptr );
	}
}

