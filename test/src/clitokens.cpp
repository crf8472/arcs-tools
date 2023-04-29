#include "catch2/catch_test_macros.hpp"

#ifndef __ARCSTOOLS_CLITOKENS_HPP__
#include "clitokens.hpp"
#endif

TEST_CASE ( "parse", "[parse]" )
{
	using arcsapp::OptionCode;
	using arcsapp::Option;
	using arcsapp::input::get_tokens;

	struct TEST
	{
		const OptionCode METAFILE     = 1;
		const OptionCode REFVALUES    = 2;
		const OptionCode RESPONSEFILE = 3;
		const OptionCode NOALBUM      = 4;
		const OptionCode FULLOPTION   = 5;
		const OptionCode SUBSET       = 6;
		const OptionCode SOMEOPTION   = 7;
		const OptionCode SOMEOTHER    = 8;
	};

	auto my_test = TEST {};

	const std::vector<std::pair<Option, OptionCode>> supported_options =
	{
		{{  'n', "no-album", false, "~",
			"Abbreviates --no-first --no-last" },
			my_test.NOALBUM },
		{{ 'm', "metafile", true, "none",
			"Specify metadata file (TOC) to use" },
			my_test.METAFILE },
		{{ 'r', "response", true, "none",
			"Specify AccurateRip response file" },
			my_test.RESPONSEFILE },
		{{      "refvalues", true, "none",
			"Specify AccurateRip reference values (as hex value list)" },
			my_test.REFVALUES},
		{{  'f', "subset-of-full-option", false, "none",
			"Full option with another option as a subset " },
			my_test.FULLOPTION},
		{{  's', "subset-of", false, "none",
			"Subset of subset-of-full-option" },
			my_test.SUBSET},
		{{      "some-option-with-value", true, "none",
			"Some option with a value" },
			my_test.SOMEOPTION },
		{{  'q', "some-option-without-value", false, "none",
			"Some option without a value" },
			my_test.SOMEOTHER }
	};

	SECTION ( "Input with distinct options" )
	{
		const char * const argv[] = { "arcstk-ignored",
			"--metafile", "foo/foo.cue", "foo/foo.wav", "-r", "foo/foo.bin"
		};
		const int argc = 6;

		auto tokens = get_tokens(argc, argv, supported_options);

		CHECK ( tokens.size() == 3 );

		CHECK ( (tokens.begin() + 0)->code()  == my_test.METAFILE     );
		CHECK ( (tokens.begin() + 0)->value() == "foo/foo.cue"        );
		CHECK ( (tokens.begin() + 1)->code()  == Option::NONE         );
		CHECK ( (tokens.begin() + 1)->value() == "foo/foo.wav"        );
		CHECK ( (tokens.begin() + 2)->code()  == my_test.RESPONSEFILE );
		CHECK ( (tokens.begin() + 2)->value() == "foo/foo.bin"        );
	}

	SECTION ( "Input with non-distinct options" )
	{
		const char * const argv[] = { "arcstk-ignored",
			"--subset-of-full-option", "--subset-of", "foo/foo.wav"
		};
		const int argc = 4;

		auto tokens = get_tokens(argc, argv, supported_options);

		CHECK ( tokens.size() == 3 );

		CHECK ( (tokens.begin() + 0)->code()  == my_test.FULLOPTION );
		CHECK ( (tokens.begin() + 1)->code()  == my_test.SUBSET     );
		CHECK ( (tokens.begin() + 2)->code()  == Option::NONE       );
		CHECK ( (tokens.begin() + 2)->value() == "foo/foo.wav"      );
	}

	SECTION ( "Symbol with value (blank)" )
	{
		const char * const argv[] = { "arcstk-whatever",
			"--some-option-with-value", "foo/foo.wav"
		};
		const int argc = 3;

		auto tokens = get_tokens(argc, argv, supported_options);

		CHECK ( tokens.size() == 1 );

		CHECK ( tokens.begin()->code()  == my_test.SOMEOPTION );
		CHECK ( tokens.begin()->value() == "foo/foo.wav"      );
	}

	SECTION ( "Symbol with value (=)" )
	{
		const char * const argv[] = { "arcstk-whatever",
			"--some-option-with-value=foo/foo.wav"
		};
		const int argc = 2;

		auto tokens = get_tokens(argc, argv, supported_options);

		CHECK ( tokens.size() == 1 );

		CHECK ( tokens.begin()->code()  == my_test.SOMEOPTION );
		CHECK ( tokens.begin()->value() == "foo/foo.wav"      );
	}

	SECTION ( "Grouped shorthands" )
	{
		const char * const argv[] = { "arcstk-whatever", "-snf",
			"--refvalues=1,2,3", "foo/foo.wav"
		};
		const int argc = 4;

		auto tokens = get_tokens(argc, argv, supported_options);

		CHECK ( tokens.size() == 5 );

		CHECK ( (tokens.begin() + 0)->code()  == my_test.SUBSET     );
		CHECK ( (tokens.begin() + 1)->code()  == my_test.NOALBUM    );
		CHECK ( (tokens.begin() + 1)->value() == ""                 );
		CHECK ( (tokens.begin() + 2)->code()  == my_test.FULLOPTION );
		CHECK ( (tokens.begin() + 2)->value() == ""                 );
		CHECK ( (tokens.begin() + 3)->code()  == my_test.REFVALUES  );
		CHECK ( (tokens.begin() + 3)->value() == "1,2,3"            );
		CHECK ( (tokens.begin() + 4)->code()  == Option::NONE       );
		CHECK ( (tokens.begin() + 4)->value() == "foo/foo.wav"      );
	}

	SECTION ( "No options" )
	{
		const char * const argv[] = { "arcstk-whatever" };
		const int argc = 1;

		auto tokens = get_tokens(argc, argv, supported_options);

		CHECK ( tokens.size() == 0 );
		CHECK ( tokens.empty() );
	}
}

