#include "catch2/catch_test_macros.hpp"

#ifndef __ARCSTOOLS_TOOLS_CALC_HPP__
#include "tools-calc.hpp"
#endif


TEST_CASE ( "audiofile_layout()", "" )
{
	using arcsapp::calc::audiofile_layout;

	using arcstk::lba_count_t;
	using arcstk::make_toc;

	SECTION ( "Audiolayout with no filenames" )
	{
		// "Bach: Organ Concertos", Simon Preston, DGG
		auto toc0 = make_toc(
			// track count
			15,
			// offsets
			std::vector<lba_count_t>{ 33, 5225, 7390, 23380, 35608, 49820, 69508,
			87733, 106333, 139495, 157863, 198495, 213368, 225320, 234103 },
			// leadout
			253038
		);

		auto tuple = audiofile_layout(*toc0);

		CHECK ( std::get<0>(tuple) == true  );
		CHECK ( std::get<1>(tuple) == false );

		auto filenames = std::get<2>(tuple);
		CHECK ( filenames.empty() );
	}

	SECTION ( "Audiolayout with one (reocurring) filename" )
	{
		// "Bach: Organ Concertos", Simon Preston, DGG
		auto toc0 = make_toc(
			// track count
			15,
			// offsets
			std::vector<lba_count_t>{ 33, 5225, 7390, 23380, 35608, 49820, 69508,
			87733, 106333, 139495, 157863, 198495, 213368, 225320, 234103 },
			// leadout
			253038,
			// filenames
			std::vector<std::string>{ "file", "file", "file", "file",
			"file", "file", "file", "file", "file", "file",
			"file", "file", "file", "file", "file" }
		);

		auto tuple = audiofile_layout(*toc0);

		CHECK ( std::get<0>(tuple) == true );
		CHECK ( std::get<1>(tuple) == true );

		auto filenames = std::get<2>(tuple);
		CHECK ( filenames.size() == 1 );
	}

	SECTION ( "Audiolayout with multiple filename and exact mapping" )
	{
		// "Bach: Organ Concertos", Simon Preston, DGG
		auto toc0 = make_toc(
			// track count
			15,
			// offsets
			std::vector<lba_count_t>{ 33, 5225, 7390, 23380, 35608, 49820, 69508,
			87733, 106333, 139495, 157863, 198495, 213368, 225320, 234103 },
			// leadout
			253038,
			// filenames
			std::vector<std::string>{ "file1", "file2", "file3", "file4",
			"file5", "file6", "file7", "file8", "file9", "file10",
			"file11", "file12", "file13", "file14", "file15" }
		);

		auto tuple = audiofile_layout(*toc0);

		CHECK ( std::get<0>(tuple) == false );
		CHECK ( std::get<1>(tuple) == true  );

		auto filenames = std::get<2>(tuple);
		CHECK ( filenames.size() == 15 );
	}

	SECTION ( "Audiolayout with multiple not pairwise distinct filenames" )
	{
		// "Bach: Organ Concertos", Simon Preston, DGG
		auto toc0 = make_toc(
			// track count
			15,
			// offsets
			std::vector<lba_count_t>{ 33, 5225, 7390, 23380, 35608, 49820, 69508,
			87733, 106333, 139495, 157863, 198495, 213368, 225320, 234103 },
			// leadout
			253038,
			// filenames
			std::vector<std::string>{ "file1", "file1", "file2", "file2",
			"file2", "file3", "file3", "file4", "file4", "file4",
			"file4", "file5", "file5", "file5", "file5" }
		);

		auto tuple = audiofile_layout(*toc0);

		CHECK ( std::get<0>(tuple) == false );
		CHECK ( std::get<1>(tuple) == false );

		auto filenames = std::get<2>(tuple);
		CHECK ( filenames.size() == 15 );
	}

}


TEST_CASE ( "ARCSMultifileAlbumCalculator", "[arcsmultifilealbumcalculator]" )
{
	using arcsapp::calc::ARCSMultifileAlbumCalculator;

	// TODO This needs mocking
}


TEST_CASE ( "HexLayout", "[hexlayout]" )
{
	using arcstk::Checksum;
	using arcsapp::calc::HexLayout;

	auto hex_layout = std::make_unique<HexLayout>();

	CHECK ( hex_layout->format(Checksum { 3456 }, 2) == "D80" );
	CHECK ( hex_layout->format(Checksum { 3456 }, 3) == "D80" );
	CHECK ( hex_layout->format(Checksum { 1023 }, 4) == "03FF" );
	CHECK ( hex_layout->format(Checksum { 1023 }, 6) == "0003FF" );
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

