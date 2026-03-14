#include "catch2/catch_test_macros.hpp"

/**
 * \file
 *
 * \brief Fixtures for application.hpp.
 */

#ifndef ARCSTOOLS_APPLICATION_HPP__
#include "application.hpp"        // TO BE TESTED
#endif


TEST_CASE ( "Output", "[output]" )
{
	using arcsapp::Output;

	SECTION ( "Getting the instance works" )
	{
		CHECK ( not Output::instance().is_appending() );
		CHECK (     Output::instance().filename() == std::string{} );
	}
}

