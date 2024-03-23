#include "catch2/catch_test_macros.hpp"

#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#include "layouts.hpp"
#endif

TEST_CASE ( "InternalFlags", "[internalflags]" )
{
	using arcsapp::InternalFlags;

	InternalFlags flags(0);
	flags.set_flag(4, true);

	REQUIRE ( not flags.flag(0) );
	REQUIRE ( not flags.flag(1) );
	REQUIRE ( not flags.flag(2) );
	REQUIRE ( not flags.flag(3) );
	REQUIRE ( flags.flag(4) );
	REQUIRE ( not flags.flag(5) );
	REQUIRE ( not flags.flag(6) );
	REQUIRE ( not flags.flag(7) );

	REQUIRE ( flags.only(4) );
	//REQUIRE ( flags.only_one_flag() );

	flags.set_flag(7, true);

	REQUIRE ( flags.flag(7) );
	REQUIRE ( not flags.only(4) );
}

