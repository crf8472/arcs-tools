#include "catch2/catch_test_macros.hpp"

#ifndef __ARCSTOOLS_APPARID_HPP__
#include "app-id.hpp"
#endif


TEST_CASE ( "ARIdConfigurator", "[ARIdConfigurator]" )
{
	using arcsapp::ARIdConfigurator;
	using arcsapp::OPTION;
	using arcsapp::ARIdOptions;

	SECTION ("List of supported options is sound and complete")
	{
		ARIdConfigurator conf1;
		using std::end;

		const auto supported { conf1.supported_options() };

		CHECK ( 18 == supported.size() );

		CHECK ( contains(ARIdOptions::READERID, supported) );
		CHECK ( contains(ARIdOptions::PARSERID, supported) );
		CHECK ( contains(ARIdOptions::LIST_TOC_FORMATS, supported) );
		CHECK ( contains(ARIdOptions::LIST_AUDIO_FORMATS, supported) );
		CHECK ( contains(ARIdOptions::CDDBID, supported) );
		CHECK ( contains(ARIdOptions::DBID, supported) );
		CHECK ( contains(ARIdOptions::FILENAME, supported) );
		CHECK ( contains(ARIdOptions::NOLABELS, supported) );
		CHECK ( contains(ARIdOptions::URL, supported) );
		CHECK ( contains(ARIdOptions::PROFILE, supported) );
		CHECK ( contains(ARIdOptions::URLPREFIX, supported) );
		CHECK ( contains(ARIdOptions::AUDIOFILE, supported) );

		CHECK ( contains(OPTION::HELP, supported) );
		CHECK ( contains(OPTION::VERSION, supported) );
		CHECK ( contains(OPTION::VERBOSITY, supported) );
		CHECK ( contains(OPTION::QUIET, supported) );
		CHECK ( contains(OPTION::LOGFILE, supported) );
		CHECK ( contains(OPTION::OUTFILE, supported) );
	}
}

