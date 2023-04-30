#include "catch2/catch_test_macros.hpp"

#ifndef __ARCSTOOLS_CONFIG_HPP__
#include "config.hpp"
#endif
#ifndef __ARCSTOOLS_APPARID_HPP__
#include "app-id.hpp"
#endif
#ifndef __ARCSTOOLS_APPARCALC_HPP__
#include "app-calc.hpp"
#endif
#ifndef __ARCSTOOLS_APPARVERIFY_HPP__
#include "app-verify.hpp"
#endif


TEST_CASE ( "DefaultConfigurator", "[DefaultConfigurator]" )
{
	using arcsapp::DefaultConfigurator;
	using arcsapp::OPTION;

	// TODO

	SECTION ("Global option: --verbosity")
	{
		const int argc = 4;
		const char* input[] = { "arcstk-calc", "-v", "4", "foo/foo.wav" };

		DefaultConfigurator conf1;

		auto options1 = conf1.provide_options(argc, input);

		CHECK ( options1->is_set(OPTION::VERBOSITY) );
		CHECK ( options1->value(OPTION::VERBOSITY) == "4" );
	}

	SECTION ("Global option: --quiet")
	{
		const int argc = 3;
		const char* input[] = { "arcstk-calc", "-q", "foo/foo.wav" };

		DefaultConfigurator conf1;

		auto options1 = conf1.provide_options(argc, input);

		CHECK ( options1->is_set(OPTION::QUIET) );
		CHECK ( options1->is_set(OPTION::VERBOSITY) );
		CHECK ( options1->value(OPTION::VERBOSITY) == "0" );
	}

	SECTION ("Global option: --logfile")
	{
		const int argc = 4;
		const char* input[] = { "arcstk-calc", "-l", "logfile", "foo/foo.wav" };

		DefaultConfigurator conf1;

		auto options1 = conf1.provide_options(argc, input);

		CHECK ( options1->is_set(OPTION::LOGFILE) );
		CHECK ( options1->value(OPTION::LOGFILE) == "logfile" );
	}

	SECTION ("Global option: --version")
	{
		const int argc = 3;
		const char* input[] = { "arcstk-calc", "--version", "foo/foo.wav" };

		DefaultConfigurator conf1;

		auto options1 = conf1.provide_options(argc, input);

		CHECK ( options1->is_set(OPTION::VERSION) );
	}

	SECTION ("Global option: --help")
	{
		const int argc = 3;
		const char* input[] = { "arcstk-calc", "--help", "foo/foo.wav" };

		DefaultConfigurator conf1;

		auto options1 = conf1.provide_options(argc, input);

		CHECK ( options1->is_set(OPTION::HELP) );
	}
}


TEST_CASE ( "ARCalcConfigurator", "[ARCalcConfigurator]" )
{
	using arcsapp::ARCalcConfigurator;
	using arcsapp::OPTION;
	using arcsapp::CALC;

	SECTION ("List of supported options is sound and complete")
	{
		ARCalcConfigurator conf1;
		using std::end;

		const auto supported { conf1.supported_options() };

		CHECK ( 26 == supported.size() );

		CHECK ( supported.find(CALC::READERID) != end(supported) );
		CHECK ( supported.find(CALC::PARSERID) != end(supported) );
		CHECK ( supported.find(CALC::LIST_TOC_FORMATS) != end(supported) );
		CHECK ( supported.find(CALC::LIST_AUDIO_FORMATS) != end(supported) );
		CHECK ( supported.find(CALC::METAFILE) != end(supported) );
		CHECK ( supported.find(CALC::NOTRACKS) != end(supported) );
		CHECK ( supported.find(CALC::NOFILENAMES) != end(supported) );
		CHECK ( supported.find(CALC::NOOFFSETS) != end(supported) );
		CHECK ( supported.find(CALC::NOLENGTHS) != end(supported) );
		CHECK ( supported.find(CALC::NOLABELS) != end(supported) );
		CHECK ( supported.find(CALC::COLDELIM) != end(supported) );
		CHECK ( supported.find(CALC::PRINTID) != end(supported) );
		CHECK ( supported.find(CALC::PRINTURL) != end(supported) );
		CHECK ( supported.find(CALC::FIRST) != end(supported) );
		CHECK ( supported.find(CALC::LAST) != end(supported) );
		CHECK ( supported.find(CALC::ALBUM) != end(supported) );
		CHECK ( supported.find(CALC::NOV1) != end(supported) );
		CHECK ( supported.find(CALC::NOV2) != end(supported) );
		CHECK ( supported.find(CALC::SUMSONLY) != end(supported) );
		CHECK ( supported.find(CALC::TRACKSASCOLS) != end(supported) );

		CHECK ( supported.find(OPTION::HELP) != end(supported) );
		CHECK ( supported.find(OPTION::VERSION) != end(supported) );
		CHECK ( supported.find(OPTION::VERBOSITY) != end(supported) );
		CHECK ( supported.find(OPTION::QUIET) != end(supported) );
		CHECK ( supported.find(OPTION::LOGFILE) != end(supported) );
		CHECK ( supported.find(OPTION::OUTFILE) != end(supported) );
	}

	SECTION ("Option --metafile triggers album mode")
	{
		const int argc = 6;
		const char* input[] = { "arcstk-calc",
			"--metafile", "foo/foo.cue", "foo/foo.wav",
			"--list-audio-formats", "--list-toc-formats"
		};

		ARCalcConfigurator conf1;

		auto options1 = conf1.provide_options(argc, input);

		CHECK ( options1->is_set(CALC::ALBUM) );
		CHECK ( options1->is_set(CALC::FIRST) );
		CHECK ( options1->is_set(CALC::LAST)  );
	}

	SECTION ("Option --metafile overrides list options")
	{
		const int argc = 6;
		const char* input[] = { "arcstk-calc",
			"--metafile", "foo/foo.cue", "foo/foo.wav",
			"--list-toc-formats", "--list-audio-formats"
		};

		ARCalcConfigurator conf1;

		auto options1 = conf1.provide_options(argc, input);

		CHECK (     options1->is_set(CALC::ALBUM)              );
		CHECK ( not options1->is_set(CALC::LIST_TOC_FORMATS)   );
		CHECK ( not options1->is_set(CALC::LIST_AUDIO_FORMATS) );
	}

	SECTION ("No calculation requested leads to unmodified options object")
	{
		const int argc = 3;
		const char* input[] = { "arcstk-calc",
			"--list-toc-formats", "--list-audio-formats"
		};

		ARCalcConfigurator conf1;

		auto options1 = conf1.provide_options(argc, input);

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


TEST_CASE ( "ARVerifyConfigurator", "[ARVerifyConfigurator]" )
{
	using arcsapp::ARVerifyConfigurator;
	using arcsapp::VERIFY;

	SECTION ("Input with -m and -r is ok")
	{
		const int argc = 6;
		const char* input[] = { "arcstk-verify",
			"-m", "foo/foo.cue", "foo/foo.wav", "-r", "foo/foo.bin"
		};

		ARVerifyConfigurator conf1;

		auto options1 = conf1.provide_options(argc, input);
		auto arguments1 = options1->arguments();

		CHECK ( options1->is_set(VERIFY::METAFILE) );
		CHECK ( options1->value(VERIFY::METAFILE) == "foo/foo.cue" );

		CHECK ( options1->is_set(VERIFY::RESPONSEFILE) );
		CHECK ( options1->value(VERIFY::RESPONSEFILE) == "foo/foo.bin" );

		CHECK ( arguments1.size() == 1 );
		CHECK ( arguments1[0] == "foo/foo.wav" );

		CHECK ( not options1->is_set(VERIFY::NOFIRST)      );
		CHECK ( not options1->is_set(VERIFY::NOLAST)       );
		CHECK ( not options1->is_set(VERIFY::NOALBUM)      );
		CHECK (     options1->is_set(VERIFY::RESPONSEFILE) );
		CHECK ( not options1->is_set(VERIFY::REFVALUES)    );
		CHECK ( not options1->is_set(VERIFY::PRINTALL)     );
		CHECK ( not options1->is_set(VERIFY::BOOLEAN)      );
		CHECK ( not options1->is_set(VERIFY::NOOUTPUT)     );

		CHECK ( not options1->is_set(VERIFY::LIST_TOC_FORMATS)   );
		CHECK ( not options1->is_set(VERIFY::LIST_AUDIO_FORMATS) );
		CHECK (     options1->is_set(VERIFY::METAFILE)           );
		CHECK ( not options1->is_set(VERIFY::NOTRACKS)           );
		CHECK ( not options1->is_set(VERIFY::NOFILENAMES)        );
		CHECK ( not options1->is_set(VERIFY::NOOFFSETS)          );
		CHECK ( not options1->is_set(VERIFY::NOLENGTHS)          );
		CHECK ( not options1->is_set(VERIFY::NOLABELS)           );
		CHECK ( not options1->is_set(VERIFY::COLDELIM)           );
		CHECK ( not options1->is_set(VERIFY::PRINTID)            );
		CHECK ( not options1->is_set(VERIFY::PRINTURL)           );
	}

	SECTION ("Option --metafile overrides list options")
	{
		const int argc = 8;
		const char* input[] = { "arcstk-verify",
			"--metafile", "foo/foo.cue", "foo/foo.wav", "-r", "foo/foo.bin",
			"--list-toc-formats", "--list-audio-formats"
		};

		ARVerifyConfigurator conf1;

		auto options1 = conf1.provide_options(argc, input);

		CHECK ( not options1->is_set(VERIFY::NOALBUM)          );
		CHECK ( not options1->is_set(VERIFY::LIST_TOC_FORMATS)   );
		CHECK ( not options1->is_set(VERIFY::LIST_AUDIO_FORMATS) );
	}

	SECTION ("Options --no-last and --no-first trigger --no-album")
	{
		const int argc = 4;
		const char* input[] = { "arcstk-verify", "--no-first", "--no-last",
			"foo/foo.wav"
		};

		ARVerifyConfigurator conf1;

		auto options1 = conf1.provide_options(argc, input);

		CHECK ( options1->is_set(VERIFY::NOFIRST) );
		CHECK ( options1->is_set(VERIFY::NOLAST)  );
		CHECK ( options1->is_set(VERIFY::NOALBUM) );
	}

	SECTION ("Option --no-album triggers --no-last and --no-first")
	{
		const int argc = 3;
		const char* input[] = { "arcstk-verify", "--no-album", "foo/foo.wav" };

		ARVerifyConfigurator conf1;

		auto options1 = conf1.provide_options(argc, input);

		CHECK ( options1->is_set(VERIFY::NOFIRST) );
		CHECK ( options1->is_set(VERIFY::NOLAST)  );
		CHECK ( options1->is_set(VERIFY::NOALBUM) );
	}

	SECTION ("Option --no-output triggers --boolean")
	{
		const int argc = 7;
		const char* input[] = { "arcstk-verify",
			"-m", "foo/foo.cue", "foo/foo.wav", "-r", "foo/foo.bin",
			"--no-output"
		};

		ARVerifyConfigurator conf1;

		auto options1 = conf1.provide_options(argc, input);

		CHECK ( options1->is_set(VERIFY::BOOLEAN)  );
		CHECK ( options1->is_set(VERIFY::NOOUTPUT) );
	}

	SECTION ("Option --refvalues deactivates --print-url and --print-id")
	{
		const int argc = 6;
		const char* input[] = { "arcstk-verify",
			"--refvalues=1,2,3", "foo/foo.cue", "foo/foo.wav",
			"--print-url", "--print-id"
		};

		ARVerifyConfigurator conf1;

		auto options1 = conf1.provide_options(argc, input);

		CHECK (     options1->is_set(VERIFY::REFVALUES) );
		CHECK ( not options1->is_set(VERIFY::PRINTID)     );
		CHECK ( not options1->is_set(VERIFY::PRINTURL)    );
	}

	SECTION ("Incompatible options --refvalues and --response are refused")
	{
		const int argc = 6;
		const char* input[] = { "arcstk-verify",
			"--refvalues=1,2,3", "foo/foo.cue", "foo/foo.wav",
			"-r", "foo/foo.bin",
		};

		ARVerifyConfigurator conf1;

		CHECK_THROWS( conf1.provide_options(argc, input) );
	}
}

