#include "catch2/catch_test_macros.hpp"

#ifndef __ARCSTOOLS_APPPARSE_HPP__
#include "app-parse.hpp"
#endif

/**
 * \file
 *
 * \brief Fixtures for app-parse.cpp
 */

TEST_CASE ( "ARParseApplication", "[arparseapplication]" )
{
	using arcsapp::ARParseApplication;

	auto a1 = ARParseApplication{};

	SECTION ( "Default construction works" )
	{
		CHECK ( a1.name() == "parse" );
	}
}

