#include "catch2/catch_test_macros.hpp"


/**
 * \file
 *
 * \brief Fixtures for tools-validate.hpp.
 */

#ifndef __ARCSTOOLS_TOOLS_VALIDATE_HPP__
#include "tools-validate.hpp"        // TO BE TESTED
#endif

#include <cstdint>                // for uint32_t
#include <string>                 // for string, stoul

#include "config.hpp"             // for Options, OptionCode

TEST_CASE ( "Validate", "[validate]" )
{
	using arcsapp::valid::Validate;
	using arcsapp::Options;
	using arcsapp::OPTION;

	SECTION ( "Options" )
	{
		auto o = Options {};

		o.set(OPTION::HELP);
		o.set(OPTION::VERBOSITY, "42");

		REQUIRE ( o.is_set(OPTION::HELP) );
		REQUIRE ( o.value(OPTION::VERBOSITY) == "42" );

		auto v1 = Validate<Options>(
				"Some valid contents",
				[](const Options& o)
				{
					return o.is_set(OPTION::HELP)
						&& o.value(OPTION::VERBOSITY) == "42";
				},
				"Either not --help or verbosity is not 42");

		auto v2 = Validate<Options>(
				"Some other valid contents",
				[](const Options& o)
				{
					return o.is_set(OPTION::QUIET)
						&& o.value(OPTION::VERBOSITY) == "24";
				},
				"Either not --quiet or verbosity is not 24");

		CHECK         ( v1.succeeds(o) );
		CHECK_NOTHROW ( v1.perform(o)  );

		CHECK         ( not v2.succeeds(o) );
		CHECK_THROWS  (     v2.perform(o)  );
	}
}

