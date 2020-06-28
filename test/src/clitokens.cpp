#include "catch2/catch.hpp"

#ifndef __ARCSTOOLS_CLITOKENS_HPP__
#include "clitokens.hpp"
#endif
#ifndef __ARCSTOOLS_CONFIG_HPP__
#include "config.hpp"
#endif
#ifndef __ARCSTOOLS_OPTIONS_HPP__
#include "options.hpp"
#endif
#ifndef __ARCSTOOLS_APPARVERIFY_HPP__
#include "app-verify.hpp"
#endif

TEST_CASE ( "CLITokens", "[clitokens]" )
{
	using arcsapp::CLITokens;

	SECTION ("CLITokens collects arguments correct")
	{
		const int argc = 6;
		const char* input[] = { "arcstk-verify",
			"irgendwas", "-m", "foo/foo.cue", "foo/foo.wav", "-r", "foo/foo.bin"
		};

		auto tokens = CLITokens(argc, input);

		tokens.consume("-m", true);
		tokens.consume("-r", true);

		CHECK ( "irgendwas" == tokens.consume() );
		CHECK ( "foo/foo.wav" == tokens.consume() );

		CHECK ( tokens.empty() );
	}

	SECTION ("Unconsumed tokens will be arguments")
	{
		const int argc = 6;
		const char* input[] = { "arcstk-verify",
			"-m", "foo/foo.cue", "foo/foo.wav", "-r", "foo/foo.bin"
		};

		auto tokens = CLITokens(argc, input);

		tokens.consume("-m", true);
		tokens.consume("-r", true);
	}
}

TEST_CASE ( "VERIFY Configurator", "[ARVerifyConfigurator]" )
{
	using arcsapp::ARVerifyConfigurator;
	using arcsapp::VERIFY;
	using arcsapp::CALC;

	SECTION ("Parses correct input ok")
	{
		const int argc = 6;
		const char* input[] = { "arcstk-verify",
			"-m", "foo/foo.cue", "foo/foo.wav", "-r", "foo/foo.bin"
		};

		ARVerifyConfigurator conf1(argc, input);

		auto options1 = conf1.provide_options();
		auto arguments1 = options1->get_arguments();

		CHECK ( options1->is_set(VERIFY::METAFILE) );
		CHECK ( options1->get(VERIFY::METAFILE) == "foo/foo.cue" );

		CHECK ( options1->is_set(VERIFY::RESPONSEFILE) );
		CHECK ( options1->get(VERIFY::RESPONSEFILE) == "foo/foo.bin" );

		CHECK ( arguments1.size() == 1 );
		CHECK ( arguments1[0] == "foo/foo.wav" );

		CHECK ( not options1->is_set(VERIFY::NOFIRST)      );
		CHECK ( not options1->is_set(VERIFY::NOLAST)       );
		CHECK ( not options1->is_set(VERIFY::NOALBUM)      );
		//CHECK (     options1->is_set(VERIFY::RESPONSEFILE) );
		CHECK ( not options1->is_set(VERIFY::REFVALUES)    );
		CHECK ( not options1->is_set(VERIFY::PRINTALL)     );
		CHECK ( not options1->is_set(VERIFY::BOOLEAN)      );
		CHECK ( not options1->is_set(VERIFY::NOOUTPUT)     );

		CHECK ( not options1->is_set(CALC::LIST_TOC_FORMATS)   );
		CHECK ( not options1->is_set(CALC::LIST_AUDIO_FORMATS) );
		//CHECK (     options1->is_set(CALC::METAFILE)           );
		CHECK ( not options1->is_set(CALC::NOTRACKS)           );
		CHECK ( not options1->is_set(CALC::NOFILENAMES)        );
		CHECK ( not options1->is_set(CALC::NOOFFSETS)          );
		CHECK ( not options1->is_set(CALC::NOLENGTHS)          );
		CHECK ( not options1->is_set(CALC::NOLABELS)           );
		CHECK ( not options1->is_set(CALC::COLDELIM)           );
		CHECK ( not options1->is_set(CALC::PRINTID)            );
		CHECK ( not options1->is_set(CALC::PRINTURL)           );
	}
}

