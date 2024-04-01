#include "catch2/catch_test_macros.hpp"

#ifndef __ARCSTOOLS_APPARVERIFY_HPP__
#include "app-verify.hpp"
#endif

TEST_CASE ( "ARVerifyConfigurator", "[ARVerifyConfigurator]" )
{
	using arcsapp::ARVerifyConfigurator;
	using arcsapp::OPTION;
	using arcsapp::VERIFY;
	using arcsapp::Configuration;
	using arcsapp::DecorationType;
	using arcsapp::ColorRegistry;
	using arcsapp::ansi::Color;

	SECTION ("List of supported options is sound and complete")
	{
		ARVerifyConfigurator conf1;
		using std::end;

		const auto supported { conf1.supported_options() };

		CHECK ( 29 == supported.size() );

		CHECK ( contains(VERIFY::READERID, supported) );
		CHECK ( contains(VERIFY::PARSERID, supported) );
		CHECK ( contains(VERIFY::LIST_TOC_FORMATS, supported) );
		CHECK ( contains(VERIFY::LIST_AUDIO_FORMATS, supported) );
		CHECK ( contains(VERIFY::METAFILE, supported) );
		CHECK ( contains(VERIFY::NOTRACKS, supported) );
		CHECK ( contains(VERIFY::NOFILENAMES, supported) );
		CHECK ( contains(VERIFY::NOOFFSETS, supported) );
		CHECK ( contains(VERIFY::NOLENGTHS, supported) );
		CHECK ( contains(VERIFY::NOLABELS, supported) );
		CHECK ( contains(VERIFY::COLDELIM, supported) );
		CHECK ( contains(VERIFY::PRINTID, supported) );
		CHECK ( contains(VERIFY::PRINTURL, supported) );
		CHECK ( contains(VERIFY::NOFIRST, supported) );
		CHECK ( contains(VERIFY::NOLAST, supported) );
		CHECK ( contains(VERIFY::NOALBUM, supported) );
		CHECK ( contains(VERIFY::RESPONSEFILE, supported) );
		CHECK ( contains(VERIFY::REFVALUES, supported) );
		CHECK ( contains(VERIFY::PRINTALL, supported) );
		CHECK ( contains(VERIFY::BOOLEAN, supported) );
		CHECK ( contains(VERIFY::NOOUTPUT, supported) );
		CHECK ( contains(VERIFY::CONFIDENCE, supported) );

		CHECK ( contains(OPTION::HELP, supported) );
		CHECK ( contains(OPTION::VERSION, supported) );
		CHECK ( contains(OPTION::VERBOSITY, supported) );
		CHECK ( contains(OPTION::QUIET, supported) );
		CHECK ( contains(OPTION::LOGFILE, supported) );
		CHECK ( contains(OPTION::OUTFILE, supported) );
	}

	SECTION ("Input with -m and -r is ok")
	{
		const int argc = 6;
		const char* argv[] = { "arcstk-verify",
			"-m", "foo/foo.cue", "foo/foo.wav", "-r", "foo/foo.bin"
		};

		ARVerifyConfigurator conf1;

		auto options1 = conf1.read_options(argc, argv);
		auto arguments1 = *options1->arguments();

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
		const char* argv[] = { "arcstk-verify",
			"--metafile", "foo/foo.cue", "foo/foo.wav", "-r", "foo/foo.bin",
			"--list-toc-formats", "--list-audio-formats"
		};

		ARVerifyConfigurator conf1;

		auto options1 = conf1.read_options(argc, argv);
		options1 = conf1.configure_options(std::move(options1));

		CHECK ( not options1->is_set(VERIFY::NOALBUM)          );
		CHECK ( not options1->is_set(VERIFY::LIST_TOC_FORMATS)   );
		CHECK ( not options1->is_set(VERIFY::LIST_AUDIO_FORMATS) );
	}

	SECTION ("Options --no-last and --no-first trigger --no-album")
	{
		const int argc = 5;
		const char* argv[] = { "arcstk-verify", "--no-first", "--no-last",
			"--refvalues=1,2,3", "foo/foo.wav"
		};

		ARVerifyConfigurator conf1;

		auto options1 = conf1.read_options(argc, argv);
		options1 = conf1.configure_options(std::move(options1));

		CHECK ( options1->is_set(VERIFY::NOFIRST) );
		CHECK ( options1->is_set(VERIFY::NOLAST)  );
		CHECK ( options1->is_set(VERIFY::NOALBUM) );
	}

	SECTION ("Option --no-album triggers --no-last and --no-first")
	{
		const int argc = 4;
		const char* argv[] = { "arcstk-verify", "--no-album",
			"--refvalues=1,2,3", "foo/foo.wav" };

		ARVerifyConfigurator conf1;

		auto options1 = conf1.read_options(argc, argv);
		options1 = conf1.configure_options(std::move(options1));

		CHECK ( options1->is_set(VERIFY::NOFIRST) );
		CHECK ( options1->is_set(VERIFY::NOLAST)  );
		CHECK ( options1->is_set(VERIFY::NOALBUM) );
	}

	SECTION ("Option --no-output triggers --boolean")
	{
		const int argc = 7;
		const char* argv[] = { "arcstk-verify",
			"-m", "foo/foo.cue", "foo/foo.wav", "-r", "foo/foo.bin",
			"--no-output"
		};

		ARVerifyConfigurator conf1;

		auto options1 = conf1.read_options(argc, argv);
		options1 = conf1.configure_options(std::move(options1));

		CHECK ( options1->is_set(VERIFY::BOOLEAN)  );
		CHECK ( options1->is_set(VERIFY::NOOUTPUT) );
	}

	SECTION ("Option --refvalues deactivates --print-url and --print-id")
	{
		const int argc = 6;
		const char* argv[] = { "arcstk-verify",
			"--refvalues=1,2,3", "foo/foo.cue", "foo/foo.wav",
			"--print-url", "--print-id"
		};

		ARVerifyConfigurator conf1;

		auto options1 = conf1.read_options(argc, argv);
		options1 = conf1.configure_options(std::move(options1));

		CHECK (     options1->is_set(VERIFY::REFVALUES) );
		CHECK ( not options1->is_set(VERIFY::PRINTID)     );
		CHECK ( not options1->is_set(VERIFY::PRINTURL)    );
	}

	SECTION ("Incompatible options --refvalues and --response are refused")
	{
		const int argc = 6;
		const char* argv[] = { "arcstk-verify",
			"--refvalues=1,2,3", "foo/foo.cue", "foo/foo.wav",
			"-r", "foo/foo.bin",
		};

		ARVerifyConfigurator conf1;
		auto options1 = conf1.read_options(argc, argv);

		CHECK_THROWS( conf1.configure_options(std::move(options1)) );
	}

	SECTION ("Configuration is loaded with correct color string")
	{
		using arcstk::Checksum;

		const int argc = 4;
		const char* argv[] = { "arcstk-verify",
			"--colors=match:fg_magenta,mismatch:fg_blue", "--refvalues=1,2,3",
			"foo/foo.wav",
		};

		const ARVerifyConfigurator vconf;
		auto options = vconf.read_options(argc, argv);

		REQUIRE ( options->value(VERIFY::COLORED) ==
				"match:fg_magenta,mismatch:fg_blue" );

		REQUIRE ( options->value(VERIFY::REFVALUES) == "1,2,3" );

		REQUIRE ( options->argument(0) == "foo/foo.wav");

		const auto config = vconf.create(std::move(options));

		auto p = config->object_ptr<arcstk::DBAR>(VERIFY::RESPONSEFILE);
		REQUIRE ( p == nullptr );

		CHECK ( Color::FG_MAGENTA ==
				config->object<ColorRegistry>(VERIFY::COLORED).get(
					DecorationType::MATCH).first );

		CHECK ( Color::FG_BLUE ==
				config->object<ColorRegistry>(VERIFY::COLORED).get(
					DecorationType::MISMATCH).first );

		CHECK ( std::vector<uint32_t>{ 1, 2, 3 } ==
				config->object<std::vector<uint32_t>>(VERIFY::REFVALUES) );
	}
}

