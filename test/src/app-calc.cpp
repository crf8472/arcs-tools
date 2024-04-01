#include "catch2/catch_test_macros.hpp"

#ifndef __ARCSTOOLS_APPARCALC_HPP__
#include "app-calc.hpp"
#endif


TEST_CASE ( "ARCalcConfigurator", "[ARCalcConfigurator]" )
{
	using arcsapp::ARCalcConfigurator;
	using arcsapp::OPTION;
	using arcsapp::CALC;

	SECTION ("List of supported options is sound and complete")
	{
		using std::find;
		using std::end;


		ARCalcConfigurator conf1;

		const auto supported { conf1.supported_options() };

		CHECK ( 26 == supported.size() );

		CHECK ( contains(CALC::READERID, supported) );
		CHECK ( contains(CALC::PARSERID, supported) );
		CHECK ( contains(CALC::LIST_TOC_FORMATS, supported) );
		CHECK ( contains(CALC::LIST_AUDIO_FORMATS, supported) );
		CHECK ( contains(CALC::METAFILE, supported) );
		CHECK ( contains(CALC::NOTRACKS, supported) );
		CHECK ( contains(CALC::NOFILENAMES, supported) );
		CHECK ( contains(CALC::NOOFFSETS, supported) );
		CHECK ( contains(CALC::NOLENGTHS, supported) );
		CHECK ( contains(CALC::NOLABELS, supported) );
		CHECK ( contains(CALC::COLDELIM, supported) );
		CHECK ( contains(CALC::PRINTID, supported) );
		CHECK ( contains(CALC::PRINTURL, supported) );
		CHECK ( contains(CALC::FIRST, supported) );
		CHECK ( contains(CALC::LAST, supported) );
		CHECK ( contains(CALC::ALBUM, supported) );
		CHECK ( contains(CALC::NOV1, supported) );
		CHECK ( contains(CALC::NOV2, supported) );
		CHECK ( contains(CALC::SUMSONLY, supported) );
		CHECK ( contains(CALC::TRACKSASCOLS, supported) );

		CHECK ( contains(OPTION::HELP, supported) );
		CHECK ( contains(OPTION::VERSION, supported) );
		CHECK ( contains(OPTION::VERBOSITY, supported) );
		CHECK ( contains(OPTION::QUIET, supported) );
		CHECK ( contains(OPTION::LOGFILE, supported) );
		CHECK ( contains(OPTION::OUTFILE, supported) );
	}

	SECTION ("Option --metafile triggers album mode")
	{
		const int argc = 6;
		const char* argv[] = { "arcstk-calc",
			"--metafile", "foo/foo.cue", "foo/foo.wav",
			"--list-audio-formats", "--list-toc-formats"
		};

		ARCalcConfigurator conf1;

		auto options1 = conf1.read_options(argc, argv);
		options1 = conf1.configure_options(std::move(options1));

		CHECK ( options1->is_set(CALC::ALBUM) );
		CHECK ( options1->is_set(CALC::FIRST) );
		CHECK ( options1->is_set(CALC::LAST)  );
	}

	SECTION ("Option --metafile overrides list options")
	{
		const int argc = 6;
		const char* argv[] = { "arcstk-calc",
			"--metafile", "foo/foo.cue", "foo/foo.wav",
			"--list-toc-formats", "--list-audio-formats"
		};

		ARCalcConfigurator conf1;

		auto options1 = conf1.read_options(argc, argv);
		options1 = conf1.configure_options(std::move(options1));

		CHECK (     options1->is_set(CALC::ALBUM)              );
		CHECK ( not options1->is_set(CALC::LIST_TOC_FORMATS)   );
		CHECK ( not options1->is_set(CALC::LIST_AUDIO_FORMATS) );
	}

	SECTION ("No calculation requested leads to unmodified options object")
	{
		const int argc = 3;
		const char* argv[] = { "arcstk-calc",
			"--list-toc-formats", "--list-audio-formats"
		};

		ARCalcConfigurator conf1;

		auto options1 = conf1.read_options(argc, argv);

		CHECK ( options1->is_set(CALC::LIST_TOC_FORMATS)   );
		CHECK ( options1->is_set(CALC::LIST_AUDIO_FORMATS) );

		CHECK ( not options1->is_set(CALC::METAFILE)       );
		CHECK ( not options1->is_set(CALC::NOTRACKS)       );
		CHECK ( not options1->is_set(CALC::NOFILENAMES)    );
		CHECK ( not options1->is_set(CALC::NOOFFSETS)      );
		CHECK ( not options1->is_set(CALC::NOLENGTHS)      );
		CHECK ( not options1->is_set(CALC::NOLABELS)       );
		CHECK ( not options1->is_set(CALC::COLDELIM)       );
		CHECK ( not options1->is_set(CALC::PRINTID)        );
		CHECK ( not options1->is_set(CALC::PRINTURL)       );

		CHECK ( not options1->is_set(CALC::FIRST)        );
		CHECK ( not options1->is_set(CALC::LAST)         );
		CHECK ( not options1->is_set(CALC::ALBUM)        );
		CHECK ( not options1->is_set(CALC::NOV1)         );
		CHECK ( not options1->is_set(CALC::NOV2)         );
		CHECK ( not options1->is_set(CALC::SUMSONLY)     );
		CHECK ( not options1->is_set(CALC::TRACKSASCOLS) );
	}
}

