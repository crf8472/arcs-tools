#include "catch2/catch_test_macros.hpp"

/**
 * \file
 *
 * \brief Fixtures for app-id.hpp.
 */

#ifndef ARCSTOOLS_APPARID_HPP_
#include "app-id.hpp"             // TO BE TESTED
#endif


TEST_CASE ( "ARIdConfigurator", "[ARIdConfigurator]" )
{
	using arcsapp::ARIdConfigurator;
	using arcsapp::ARIdOptions;
	using arcsapp::FORMATBASE;
	using arcsapp::OPTION;

	SECTION ("List of supported options is sound and complete")
	{
		ARIdConfigurator conf1;
		using std::end;

		const auto supported { conf1.supported_options() };

		CHECK ( 18 == supported.size() );

		// OPTION::NONE is not a cli-supported option
		CHECK ( contains(OPTION::HELP,                    supported) );
		CHECK ( contains(OPTION::VERSION,                 supported) );
		CHECK ( contains(OPTION::VERBOSITY,               supported) );
		CHECK ( contains(OPTION::QUIET,                   supported) );
		CHECK ( contains(OPTION::LOGFILE,                 supported) );
		CHECK ( contains(OPTION::OUTFILE,                 supported) );

		CHECK ( contains(FORMATBASE::LIST_TOC_FORMATS,    supported) );
		CHECK ( contains(FORMATBASE::LIST_AUDIO_FORMATS,  supported) );
		CHECK ( contains(FORMATBASE::READERID,            supported) );
		CHECK ( contains(FORMATBASE::PARSERID,            supported) );

		CHECK ( contains(ARIdOptions::CDDBID,             supported) );
		CHECK ( contains(ARIdOptions::URL,                supported) );
		CHECK ( contains(ARIdOptions::DBID,               supported) );
		CHECK ( contains(ARIdOptions::FILENAME,           supported) );
		CHECK ( contains(ARIdOptions::PROFILE,            supported) );
		CHECK ( contains(ARIdOptions::URLPREFIX,          supported) );
		// ARIdOptions::ID is not a cli-supported option
		CHECK ( contains(ARIdOptions::AUDIOFILE,          supported) );
		CHECK ( contains(ARIdOptions::NOLABELS,           supported) );

		//
		CHECK ( ! contains(ARIdOptions::ID,               supported) );
		CHECK ( ! contains(OPTION::NONE,                  supported) );
	}
}


TEST_CASE ( "ARIdApplication", "[aridapplication]" )
{
	using arcsapp::ARIdApplication;
	using arcsapp::ARIdConfigurator;

	auto app = ARIdApplication {/*default*/};

	SECTION ( "Name is 'id'" )
	{
		CHECK ( "id" == app.name() );
	}

	// TODO test following functions
	// call_syntax()
	// create_configurator()

	// TODO test following functions with default config
	// calculation_requested()
	// requested_types()
	// run_calculation()

}

