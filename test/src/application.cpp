#include "catch2/catch_test_macros.hpp"

#ifndef __ARCSTOOLS_APPLICATION_HPP__
#include "application.hpp"
#endif

/**
 * \file
 *
 * \brief Fixtures for application.cpp
 */

TEST_CASE ( "Output", "[output]" )
{
	using arcsapp::Output;

	SECTION ( "Getting the instance works" )
	{
		CHECK ( not Output::instance().is_appending() );
		CHECK (     Output::instance().filename() == std::string{} );
	}
}

