#include "catch2/catch_test_macros.hpp"

#ifndef __ARCSTOOLS_APPREGISTRY_HPP__
#include "appregistry.hpp"
#endif

/**
 * \file
 *
 * \brief Fixtures for appregistry.cpp
 */

// TODO instantiate_application()
// TODO RegisterApplicationType

TEST_CASE ( "matches_name()", "[matches_name]" )
{
	using arcsapp::matches_name;

	SECTION ( "matches only equal strings" )
	{
		CHECK ( not matches_name("foo", "fo")     );
		CHECK ( not matches_name("foo", "fooo")   );
		CHECK ( not matches_name("foo", "fo0")    );
		CHECK ( not matches_name("foo", "7oo")    );

		CHECK (     matches_name("foo", "foo")    );
	}

}

