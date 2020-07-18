#include "catch2/catch.hpp"

#ifndef __ARCSTOOLS_TOOLS_CALC_HPP__
#include "tools-calc.hpp"
#endif


TEST_CASE ( "audiofile_layout()", "" )
{
	using arcsapp::calc::audiofile_layout;

	using arcstk::lba_count;
	using arcstk::make_toc;

	SECTION ( "Audiolayout with no filenames" )
	{
		// "Bach: Organ Concertos", Simon Preston, DGG
		auto toc0 = make_toc(
			// track count
			15,
			// offsets
			std::vector<lba_count>{ 33, 5225, 7390, 23380, 35608, 49820, 69508,
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
			std::vector<lba_count>{ 33, 5225, 7390, 23380, 35608, 49820, 69508,
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
			std::vector<lba_count>{ 33, 5225, 7390, 23380, 35608, 49820, 69508,
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
			std::vector<lba_count>{ 33, 5225, 7390, 23380, 35608, 49820, 69508,
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

