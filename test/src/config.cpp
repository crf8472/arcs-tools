#include "catch2/catch_test_macros.hpp"

/**
 * \file
 *
 * \brief Fixtures for config.hpp.
 */

#ifndef ARCSTOOLS_CONFIG_HPP_
#include "config.hpp"             // TO BE TESTED
#endif


TEST_CASE ( "DefaultConfigurator", "[DefaultConfigurator]" )
{
	using arcsapp::DefaultConfigurator;
	using arcsapp::OPTION;

	SECTION ("List of supported options is sound and complete")
	{
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

