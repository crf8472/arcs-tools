#include "catch2/catch_test_macros.hpp"

#include <algorithm> // for find_if
#include <cstdint>   // for uint32_t
#include <iterator>  // for begin, end
#include <utility>   // for pair

#ifndef __ARCSTOOLS_CONFIG_HPP__
#include "config.hpp"
#endif


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

		CHECK ( contains(OPTION::HELP,      supported) );
		CHECK ( contains(OPTION::VERSION,   supported) );
		CHECK ( contains(OPTION::VERBOSITY, supported) );
		CHECK ( contains(OPTION::QUIET,     supported) );
		CHECK ( contains(OPTION::LOGFILE,   supported) );
		CHECK ( contains(OPTION::OUTFILE,   supported) );
	}

	SECTION ("Global option: --verbosity")
	{
		const int argc = 4;
		const char* argv[] = { "arcstk-calc", "-v", "4", "foo/foo.wav" };

		DefaultConfigurator conf1;

		auto options1 = conf1.read_options(argc, argv);

		CHECK ( options1->is_set(OPTION::VERBOSITY) );
		CHECK ( options1->value(OPTION::VERBOSITY) == "4" );
	}

	SECTION ("Global option: --quiet")
	{
		const int argc = 3;
		const char* argv[] = { "arcstk-calc", "-q", "foo/foo.wav" };

		DefaultConfigurator conf1;

		auto options1 = conf1.read_options(argc, argv);
		options1 = conf1.configure_options(std::move(options1));

		CHECK ( options1->is_set(OPTION::QUIET) );
		CHECK ( options1->is_set(OPTION::VERBOSITY) );
		CHECK ( options1->value(OPTION::VERBOSITY) == "0" );
	}

	SECTION ("Global option: --logfile")
	{
		const int argc = 4;
		const char* argv[] = { "arcstk-calc", "-l", "logfile", "foo/foo.wav" };

		DefaultConfigurator conf1;

		auto options1 = conf1.read_options(argc, argv);

		CHECK ( options1->is_set(OPTION::LOGFILE) );
		CHECK ( options1->value(OPTION::LOGFILE) == "logfile" );
	}

	SECTION ("Global option: --version")
	{
		const int argc = 3;
		const char* argv[] = { "arcstk-calc", "--version", "foo/foo.wav" };

		DefaultConfigurator conf1;

		auto options1 = conf1.read_options(argc, argv);

		CHECK ( options1->is_set(OPTION::VERSION) );
	}

	SECTION ("Global option: --help")
	{
		const int argc = 3;
		const char* argv[] = { "arcstk-calc", "--help", "foo/foo.wav" };

		DefaultConfigurator conf1;

		auto options1 = conf1.read_options(argc, argv);

		CHECK ( options1->is_set(OPTION::HELP) );
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
					return std::stoul(s, nullptr, 16);
				}) };

		CHECK ( 3 == res1.size() );
		CHECK ( 0x98B10E0F == res1[0] );
		CHECK ( 0x475F57E9 == res1[1] );
		CHECK ( 0x7304F1C4 == res1[2] );

		const auto list2 { "98B10E0F,475F57E9,7304F1C4" };

		const auto res2 { parse_list_to_objects<uint32_t>(list2, ',',
				[](const std::string& s) -> uint32_t
				{
					return std::stoul(s, nullptr, 16);
				}) };

		CHECK ( 3 == res2.size() );
		CHECK ( 0x98B10E0F == res2[0] );
		CHECK ( 0x475F57E9 == res2[1] );
		CHECK ( 0x7304F1C4 == res2[2] );
	}
}

