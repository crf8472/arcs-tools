#include "catch2/catch_test_macros.hpp"

#ifndef __ARCSTOOLS_CLITOKENS_HPP__
#include "clitokens.hpp"
#endif

#ifndef __ARCSTOOLS_MOCKS_PARSE_HPP__
#include "mocks_parse.hpp"
#endif


TEST_CASE ( "Option", "[option]" )
{
	using arcsapp::OptionCode;
	using arcsapp::Option;

	SECTION ( "Is copy-constructible" )
	{
		CHECK( std::is_copy_constructible<Option>::value );
	}

	SECTION ( "Is move-constructible" )
	{
		CHECK( std::is_move_constructible<Option>::value );
	}
}


TEST_CASE ( "parse()", "[parse]" )
{
	using arcsapp::OptionCode;
	using arcsapp::Option;
	using arcsapp::OptionRegistry;

	using arcsapp::input::ARGUMENT;
	using arcsapp::input::DASH;
	using arcsapp::input::DDASH;
	using arcsapp::input::CallSyntaxException;
	//using arcsapp::input::get_tokens;

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
		const OptionCode OTHERSUBSET  = 9;
	};

	auto my_test = TEST {};

	const OptionRegistry supported_options =
	{
		{ my_test.NOALBUM,
		{  'n', "no-album", false, "~",
			"Abbreviates --no-first --no-last" }},
		{ my_test.METAFILE,
		{ 'm', "metafile", true, "none",
			"Specify metadata file (TOC) to use" }},
		{ my_test.RESPONSEFILE,
		{ 'r', "response", true, "none",
			"Specify AccurateRip response file" }},
		{ my_test.REFVALUES,
		{      "refvalues", true, "none",
			"Specify AccurateRip reference values (as hex value list)" }},
		{ my_test.SUBSET,
		{  's', "subset-of", false, "none",
			"Subset of subset-of-full-option" }},
		{ my_test.FULLOPTION,
		{  'f', "subset-of-full-option", false, "none",
			"Full option with another option as a subset " }},
		{ my_test.SOMEOPTION,
		{      "some-option-with-value", true, "none",
			"Some option with a value" }},
		{ my_test.SOMEOTHER,
		{  'q', "some-option-without-value", false, "none",
			"Some option without a value" }},
		{ my_test.OTHERSUBSET,
		{  't', "subset-of-another", false, "none",
			"Alias for SOMEOTHER" }},
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
		CHECK ( (tokens.begin() + 1)->code()  == ARGUMENT             );
		CHECK ( (tokens.begin() + 1)->value() == "foo/foo.wav"        );
		CHECK ( (tokens.begin() + 2)->code()  == my_test.RESPONSEFILE );
		CHECK ( (tokens.begin() + 2)->value() == "foo/foo.bin"        );
	}

	SECTION ( "Input with an option that is a subset of another" )
	{
		const char * const argv[] = { "arcstk-ignored",
			"--subset-of-full-option", "--subset-of",
			"--subset-of-another", "foo/foo.wav"
		};
		const int argc = 5;

		auto tokens = get_tokens(argc, argv, supported_options);

		CHECK ( tokens.size() == 4 );

		CHECK ( (tokens.begin() + 0)->code()  == my_test.FULLOPTION );
		CHECK ( (tokens.begin() + 1)->code()  == my_test.SUBSET     );
		CHECK ( (tokens.begin() + 2)->code()  == my_test.OTHERSUBSET);
		CHECK ( (tokens.begin() + 3)->code()  == ARGUMENT           );
		CHECK ( (tokens.begin() + 3)->value() == "foo/foo.wav"      );
	}

	SECTION ( "Input with twice the same unvalued option" )
	{
		const char * const argv[] = { "arcstk-ignored",
			"--subset-of-another", "-t", "foo/foo.wav"
		};
		const int argc = 4;

		auto tokens = get_tokens(argc, argv, supported_options);

		CHECK ( tokens.size() == 3 );

		CHECK ( (tokens.begin() + 0)->code()  == my_test.OTHERSUBSET);
		CHECK ( (tokens.begin() + 1)->code()  == my_test.OTHERSUBSET);
		CHECK ( (tokens.begin() + 2)->code()  == ARGUMENT           );
		CHECK ( (tokens.begin() + 2)->value() == "foo/foo.wav"      );
	}

	SECTION ( "Input with shorthand valued option without delimiter" )
	{
		const char * const argv[] = { "arcstk-ignored",
			"--subset-of-another", "-r0", "foo/foo.wav"
		};
		const int argc = 4;

		auto tokens = get_tokens(argc, argv, supported_options);

		CHECK ( tokens.size() == 3 );

		CHECK ( (tokens.begin() + 0)->code()  == my_test.OTHERSUBSET );
		CHECK ( (tokens.begin() + 1)->code()  == my_test.RESPONSEFILE);
		CHECK ( (tokens.begin() + 1)->value() == "0"                 );
		CHECK ( (tokens.begin() + 2)->code()  == ARGUMENT            );
		CHECK ( (tokens.begin() + 2)->value() == "foo/foo.wav"       );
	}

	SECTION ( "Valued option with blank as delimiter" )
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

	SECTION ( "Valued option with '=' as delimiter" )
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
		CHECK ( (tokens.begin() + 4)->code()  == ARGUMENT           );
		CHECK ( (tokens.begin() + 4)->value() == "foo/foo.wav"      );
	}

	SECTION ( "Command line without any options and arguments" )
	{
		const char * const argv[] = { "arcstk-whatever" };
		const int argc = 1;

		auto tokens = get_tokens(argc, argv, supported_options);

		CHECK ( tokens.size() == 0 );
		CHECK ( tokens.empty() );
	}

	SECTION ( "Command line with only --" )
	{
		const char * const argv[] = { "arcstk-whatever", "--" };
		const int argc = 2;

		auto tokens = get_tokens(argc, argv, supported_options);

		CHECK ( tokens.size() == 1 );
		CHECK ( tokens.begin()->code() == DDASH );
	}

	SECTION ( "Command line with only -" )
	{
		const char * const argv[] = { "arcstk-whatever", "-" };
		const int argc = 2;

		auto tokens = get_tokens(argc, argv, supported_options);

		CHECK ( tokens.size() == 1 );
		CHECK ( tokens.begin()->code() == DASH );
	}

	SECTION ( "Throws CallSyntaxException on invalid option" )
	{
		const char * const argv[] = { "arcstk-whatever", "--invalid-option" };
		const int argc = 2;

		CHECK_THROWS ( get_tokens(argc, argv, supported_options) );
	}

	// TODO Test path is_alias
	// TODO Test path !exact && !is_alias

	SECTION ( "Throws CallSyntaxException on missing value after '='" )
	{
		const char * const argv[] = { "arcstk-whatever", "--response=" };
		const int argc = 2;

		CHECK_THROWS ( get_tokens(argc, argv, supported_options) );
	}

	SECTION ( "Throws CallSyntaxException on trailing characters" )
	{
		const char * const argv[] = { "arcstk-whatever", "--no-album0" };
		const int argc = 2;

		CHECK_THROWS ( get_tokens(argc, argv, supported_options) );
	}

	SECTION ( "Throws CallSyntaxException on unexpected option value" )
	{
		const char * const argv[] = { "arcstk-whatever", "--no-album=42" };
		const int argc = 2;

		CHECK_THROWS ( get_tokens(argc, argv, supported_options) );
	}

	SECTION ( "Throws CallSyntaxException on missing value after symbol" )
	{
		const char * const argv[] = { "arcstk-whatever", "--response" };
		const int argc = 2;

		CHECK_THROWS ( get_tokens(argc, argv, supported_options) );
	}
}

