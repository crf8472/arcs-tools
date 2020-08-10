#include "catch2/catch.hpp"

#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#include "layouts.hpp"
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


TEST_CASE ( "HexLayout", "[hexlayout]" )
{
	using arcsapp::Checksum;
	using arcsapp::HexLayout;

	auto hex_layout = std::make_unique<HexLayout>();

	CHECK ( hex_layout->format(Checksum { 3456 }, 2) == "D80" );
	CHECK ( hex_layout->format(Checksum { 3456 }, 3) == "D80" );
	CHECK ( hex_layout->format(Checksum { 1023 }, 4) == "03FF" );
	CHECK ( hex_layout->format(Checksum { 1023 }, 6) == "0003FF" );
}


TEST_CASE ( "InternalFlags", "[internalflags]" )
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


TEST_CASE ( "WithMetadataFlagMethods", "[withmetadataflagmethods]" )
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


TEST_CASE ( "ARTripletLayout", "[artripletlayout]" )
{
	using arcsapp::ARTripletLayout;

	ARTripletLayout lyt;

	CHECK ( not lyt.format(8, { 0xFFAABBCC, 24, 0x12345678 }).empty() );
}


TEST_CASE ( "ARIdTableLayout", "[aridtablelayout]" )
{
	using arcsapp::ARIdTableLayout;

	ARIdTableLayout lyt(false, false, true, false, false, false, false, false);

	CHECK ( not lyt.fieldlabels() );
	CHECK ( not lyt.id() );
	CHECK (     lyt.url() );
	CHECK ( not lyt.filename() );
	CHECK ( not lyt.track_count() );
	CHECK ( not lyt.disc_id_1() );
	CHECK ( not lyt.disc_id_2() );
	CHECK ( not lyt.cddb_id() );

	CHECK ( not lyt.format({ 15, 0x001b9178, 0x014be24e, 0xb40d2d0f }, "")
			.empty() );
}


TEST_CASE ( "CalcAlbumTableLayout", "[calcalbumtablelayout]" )
{
	using arcsapp::CalcAlbumTableLayout;

	using arcstk::ARId;
	using arcstk::checksum::type;
	using arcstk::Checksum;
	using arcstk::Checksums;
	using arcstk::ChecksumSet;
	using arcstk::TOC;

	// Checksums

	ChecksumSet track01 { 5192 };
	track01.insert(type::ARCS2, Checksum(0xB89992E5));
	track01.insert(type::ARCS1, Checksum(0x98B10E0F));

	ChecksumSet track02 { 2165 };
	track02.insert(type::ARCS2, Checksum(0x4F77EB03));
	track02.insert(type::ARCS1, Checksum(0x475F57E9));

	auto checksums = Checksums { track01, track02 };

	// Filenames

	auto filenames = std::vector<std::string> { "foo", "bar" };

	// TOC (null)

	const TOC* toc = nullptr;

	// ARId

	auto arid = ARId { 2, 0x001b9178, 0x014be24e, 0xb40d2d0f };

	CalcAlbumTableLayout lyt(true, true, true, true, true, ";");

	CHECK ( lyt.label() );
	CHECK ( lyt.track() );
	CHECK ( lyt.offset() );
	CHECK ( lyt.length() );
	CHECK ( lyt.filename() );
	CHECK ( lyt.column_delimiter() == ";" );

	CHECK ( not lyt.format(checksums, filenames, toc, arid, true/*is album*/)
			.empty() );

	CHECK ( lyt.label() );
	CHECK ( lyt.track() );
	CHECK ( lyt.offset() );
	CHECK ( lyt.length() );
	CHECK ( lyt.filename() );
	CHECK ( lyt.column_delimiter() == ";" );
}


TEST_CASE ( "CalcTracksTableLayout", "[calctrackstablelayout]" )
{
	using arcsapp::CalcTracksTableLayout;

	using arcstk::ARId;
	using arcstk::checksum::type;
	using arcstk::Checksum;
	using arcstk::Checksums;
	using arcstk::ChecksumSet;
	using arcstk::TOC;

	// Checksums

	ChecksumSet track01 { 5192 };
	track01.insert(type::ARCS2, Checksum(0xB89992E5));
	track01.insert(type::ARCS1, Checksum(0x98B10E0F));

	ChecksumSet track02 { 2165 };
	track02.insert(type::ARCS2, Checksum(0x4F77EB03));
	track02.insert(type::ARCS1, Checksum(0x475F57E9));

	auto checksums = Checksums { track01, track02 };

	// Filenames

	auto filenames = std::vector<std::string> { "foo", "bar" };

	// TOC (null)

	const TOC* toc = nullptr;

	// ARId

	auto arid = ARId { 2, 0x001b9178, 0x014be24e, 0xb40d2d0f };

	CalcTracksTableLayout lyt(true, true, true, true, true, ";");

	CHECK ( lyt.label() );
	CHECK ( lyt.track() );
	CHECK ( lyt.offset() );
	CHECK ( lyt.length() );
	CHECK ( lyt.filename() );
	CHECK ( lyt.column_delimiter() == ";" );

	// All ok (TOC is null)

	CHECK ( not lyt.format(checksums, filenames, toc, arid, true/*is album*/)
			.empty() );

	CHECK ( lyt.label() );
	CHECK ( lyt.track() );
	CHECK ( lyt.offset() );
	CHECK ( lyt.length() );
	CHECK ( lyt.filename() );
	CHECK ( lyt.column_delimiter() == ";" );

	// Forbidden: no TOC + no filenames

	CHECK_THROWS ( not lyt.format(checksums, std::vector<std::string>{}, toc,
				arid, true/*is album*/).empty() );
}


TEST_CASE ( "VerifyTableLayout", "[verifytablelayout]" )
{
	using arcsapp::VerifyTableLayout;

	using arcstk::ARId;
	using arcstk::checksum::type;
	using arcstk::Checksum;
	using arcstk::Checksums;
	using arcstk::ChecksumSet;
	using arcstk::Match;
	using arcstk::TOC;
	using arcstk::details::create_match;

	// Checksums

	ChecksumSet track01 { 5192 };
	track01.insert(type::ARCS2, Checksum(0xB89992E5));
	track01.insert(type::ARCS1, Checksum(0x98B10E0F));

	ChecksumSet track02 { 2165 };
	track02.insert(type::ARCS2, Checksum(0x4F77EB03));
	track02.insert(type::ARCS1, Checksum(0x475F57E9));

	auto actual_checksums = Checksums { track01, track02 };

	// Filenames

	auto filenames = std::vector<std::string> { "foo", "bar" };

	// Reference Checksums

	auto ref_checksums = std::vector<Checksum> {
		Checksum(0xB89992E5), Checksum(0x4F77EB03) };

	// Match

	auto match = create_match(2, 2); // 2 blocks, 2 tracks/block
	match->verify_id(0);
	match->verify_track(0, 0, true);
	match->verify_track(0, 1, true);

	// TOC (null)

	const TOC* toc = nullptr;

	// ARId

	auto arid = ARId { 2, 0x001b9178, 0x014be24e, 0xb40d2d0f };

	VerifyTableLayout lyt(true, true, true, true, true, ";");

	CHECK ( lyt.label() );
	CHECK ( lyt.track() );
	CHECK ( lyt.offset() );
	CHECK ( lyt.length() );
	CHECK ( lyt.filename() );
	CHECK ( lyt.column_delimiter() == ";" );

	// All ok (TOC is null)

	CHECK ( not lyt.format(actual_checksums, filenames, ref_checksums,
				match.get(), 0/*block*/, true/*v2*/, toc, arid).empty() );

	CHECK ( lyt.label() );
	CHECK ( lyt.track() );
	CHECK ( lyt.offset() );
	CHECK ( lyt.length() );
	CHECK ( lyt.filename() );
	CHECK ( lyt.column_delimiter() == ";" );

	// Forbidden: no filenames

	CHECK_THROWS ( not lyt.format(actual_checksums, std::vector<std::string>{},
				ref_checksums, match.get(), 0, true, toc, arid).empty() );
}

