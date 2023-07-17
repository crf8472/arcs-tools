#include "catch2/catch_test_macros.hpp"

#include <algorithm> // for find_if
#include <cstdint>   // for uint32_t
#include <iterator>  // for begin, end
#include <utility>   // for pair

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
#ifndef __ARCSTOOLS_ANSI_HPP__
#include "ansi.hpp"
#endif


/**
 * \brief TRUE iff OptionRegistry r contains a pair with OptionCode c.
 */
bool contains(const arcsapp::OptionCode c, const arcsapp::OptionRegistry& r)
{
	using std::begin;
	using std::end;
	using std::find_if;

	const auto code_equals =
		[c](const std::pair<arcsapp::OptionCode, arcsapp::Option>& p)
		{
			return p.first == c;
		};

	return find_if(begin(r), end(r), code_equals) != end(r);
}


TEST_CASE ( "DefaultConfigurator", "[DefaultConfigurator]" )
{
	using arcsapp::DefaultConfigurator;
	using arcsapp::OPTION;

	SECTION ("List of supported options is sound and complete")
	{
		using std::find;
		using std::end;

		DefaultConfigurator conf1;

		const auto supported { conf1.supported_options() };

		CHECK ( 6 == supported.size() );

		CHECK ( contains(OPTION::HELP, supported) );
		CHECK ( contains(OPTION::VERSION, supported) );
		CHECK ( contains(OPTION::VERBOSITY, supported) );
		CHECK ( contains(OPTION::QUIET, supported) );
		CHECK ( contains(OPTION::LOGFILE, supported) );
		CHECK ( contains(OPTION::OUTFILE, supported) );
	}

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

		CHECK ( 28 == supported.size() );

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

	SECTION ("Configuration is loaded with correct color string")
	{
		using arcstk::Checksum;

		const int argc = 4;
		const char* argv[] = { "arcstk-verify",
			"--colors=match:fg_magenta,mismatch:fg_blue", "--refvalues=1,2,3",
			"foo/foo.wav",
		};

		const ARVerifyConfigurator conf1;
		auto o = conf1.provide_options(argc, argv);
		const auto c = conf1.create(std::move(o));

		CHECK ( Color::FG_MAGENTA ==
				c->object<ColorRegistry>(VERIFY::COLORED)->get(
					DecorationType::MATCH) );

		CHECK ( Color::FG_BLUE ==
				c->object<ColorRegistry>(VERIFY::COLORED)->get(
					DecorationType::MISMATCH) );

		CHECK ( std::vector<Checksum>{ Checksum(1), Checksum(2), Checksum(3) }
				== *c->object<std::vector<Checksum>>(VERIFY::REFVALUES) );
	}
}

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


TEST_CASE ( "parse_list_to_objects()", "[parse_list_to_objects]" )
{
	using arcsapp::parse_list_to_objects;

	SECTION ("Parse non-empty lists of hex values successfully")
	{
		const auto list1 { "0x98B10E0F,0x475F57E9,0x7304F1C4" };

		const auto res1 { parse_list_to_objects<uint32_t>(list1, ',',
				[](const std::string& s) -> uint32_t
				{
					return std::stoul(s, 0, 16);
				}) };

		CHECK ( 3 == res1.size() );
		CHECK ( 0x98B10E0F == res1[0] );
		CHECK ( 0x475F57E9 == res1[1] );
		CHECK ( 0x7304F1C4 == res1[2] );

		const auto list2 { "98B10E0F,475F57E9,7304F1C4" };

		const auto res2 { parse_list_to_objects<uint32_t>(list2, ',',
				[](const std::string& s) -> uint32_t
				{
					return std::stoul(s, 0, 16);
				}) };

		CHECK ( 3 == res2.size() );
		CHECK ( 0x98B10E0F == res2[0] );
		CHECK ( 0x475F57E9 == res2[1] );
		CHECK ( 0x7304F1C4 == res2[2] );
	}
}

