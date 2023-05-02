#include "catch2/catch_test_macros.hpp"

#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>   // for Checksum
#endif

#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#include "layouts.hpp"
#endif


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


TEST_CASE ( "HexLayout", "[hexlayout]" )
{
	using arcstk::Checksum;
	using arcsapp::HexLayout;

	auto hex_layout = std::make_unique<HexLayout>();

	CHECK ( hex_layout->format(Checksum { 3456 }, 2) == "D80" );
	CHECK ( hex_layout->format(Checksum { 3456 }, 3) == "D80" );
	CHECK ( hex_layout->format(Checksum { 1023 }, 4) == "03FF" );
	CHECK ( hex_layout->format(Checksum { 1023 }, 6) == "0003FF" );
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


//TEST_CASE ( "CalcResultFormatter", "[calcresultformatter]" )
//{
//	using arcsapp::CalcResultFormatter;
//
//	using arcstk::ARId;
//	using arcstk::checksum::type;
//	using arcstk::Checksum;
//	using arcstk::Checksums;
//	using arcstk::ChecksumSet;
//	using arcstk::TOC;
//
//	// Checksums
//
//	ChecksumSet track01 { 5192 };
//	track01.insert(type::ARCS2, Checksum(0xB89992E5));
//	track01.insert(type::ARCS1, Checksum(0x98B10E0F));
//
//	ChecksumSet track02 { 2165 };
//	track02.insert(type::ARCS2, Checksum(0x4F77EB03));
//	track02.insert(type::ARCS1, Checksum(0x475F57E9));
//
//	auto checksums = Checksums { track01, track02 };
//
//	// Filenames
//
//	auto filenames = std::vector<std::string> { "foo", "bar" };
//
//	// TOC (null)
//
//	const TOC* toc = nullptr;
//
//	// ARId
//
//	auto arid = ARId { 2, 0x001b9178, 0x014be24e, 0xb40d2d0f };
//
//	CalcResultFormatter lyt;//(true, true, true, true, true, ";");
//
//	CHECK ( !lyt.label() );
//	CHECK ( !lyt.track() );
//	CHECK ( !lyt.offset() );
//	CHECK ( !lyt.length() );
//	CHECK ( !lyt.filename() );
//
//	//CHECK ( !lyt.format(checksums, filenames, toc, arid).empty() );
//}
//
//
//TEST_CASE ( "VerifyResultFormatter", "[verifyresultformatter]" )
//{
//	using arcsapp::VerifyResultFormatter;
//
//	using arcstk::ARId;
//	using arcstk::checksum::type;
//	using arcstk::Checksum;
//	using arcstk::Checksums;
//	using arcstk::ChecksumSet;
//	using arcstk::Match;
//	using arcstk::TOC;
//	using arcstk::details::create_match;
//
//	// Checksums
//
//	ChecksumSet track01 { 5192 };
//	track01.insert(type::ARCS2, Checksum(0xB89992E5));
//	track01.insert(type::ARCS1, Checksum(0x98B10E0F));
//
//	ChecksumSet track02 { 2165 };
//	track02.insert(type::ARCS2, Checksum(0x4F77EB03));
//	track02.insert(type::ARCS1, Checksum(0x475F57E9));
//
//	auto actual_checksums = Checksums { track01, track02 };
//
//	// Filenames
//
//	auto filenames = std::vector<std::string> { "foo", "bar" };
//
//	// Reference Checksums
//
//	auto ref_checksums = std::vector<Checksum> {
//		Checksum(0xB89992E5), Checksum(0x4F77EB03) };
//
//	// Match
//
//	auto match = create_match(2, 2); // 2 blocks, 2 tracks/block
//	match->verify_id(0);
//	match->verify_track(0, 0, true);
//	match->verify_track(0, 1, true);
//
//	// TOC (null)
//
//	const TOC* toc = nullptr;
//
//	// ARId
//
//	auto arid = ARId { 2, 0x001b9178, 0x014be24e, 0xb40d2d0f };
//
//	VerifyResultFormatter lyt;//(true, true, true, true, true, ";");
//
//	CHECK ( !lyt.label() );
//	CHECK ( !lyt.track() );
//	CHECK ( !lyt.offset() );
//	CHECK ( !lyt.length() );
//	CHECK ( !lyt.filename() );
//
//	// All ok (TOC is null)
//
//	//CHECK ( not lyt.format(actual_checksums, filenames, ref_checksums,
//	//			match.get(), 0/*block*/, true/*v2*/, toc, arid).empty() );
//
//	// Forbidden: no filenames
//
//	//CHECK_THROWS ( not lyt.format(actual_checksums, std::vector<std::string>{},
//	//			ref_checksums, match.get(), 0, true, toc, arid).empty() );
//}
//
