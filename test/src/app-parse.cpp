#include "catch2/catch_test_macros.hpp"

/**
 * \file
 *
 * \brief Fixtures for app-parse.hpp.
 */

#ifndef __ARCSTOOLS_APPPARSE_HPP__
#include "app-parse.hpp"          // TO BE TESTED
#endif


TEST_CASE ( "ARParseApplication", "[arparseapplication]" )
{
	using arcsapp::ARParseApplication;

	auto a1 = ARParseApplication{};

	SECTION ( "Default construction works" )
	{
		CHECK ( a1.name() == "parse" );
	}
}

