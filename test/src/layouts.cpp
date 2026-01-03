#include "catch2/catch_test_macros.hpp"

#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#include "layouts.hpp"
#endif

TEST_CASE ( "Flags", "[flags]" )
{
	using arcsapp::Flags;

	SECTION ( "Default construction works" )
	{
		const auto flags = Flags {};

		CHECK ( flags.no_flags() );
		CHECK ( not flags.only_one_flag() );

		CHECK ( not flags.flag(0) );
		CHECK ( not flags.flag(1) );
		CHECK ( not flags.flag(2) );
		CHECK ( not flags.flag(3) );
		CHECK ( not flags.flag(4) );
		CHECK ( not flags.flag(5) );
		CHECK ( not flags.flag(6) );
		CHECK ( not flags.flag(7) );

	}

	SECTION ( "Construction with single flag works" )
	{
		const auto flags = Flags { 1 << 6 };

		CHECK ( flags.only(6) );
		CHECK ( flags.only_one_flag() );

		CHECK ( not flags.flag(0) );
		CHECK ( not flags.flag(1) );
		CHECK ( not flags.flag(2) );
		CHECK ( not flags.flag(3) );
		CHECK ( not flags.flag(4) );
		CHECK ( not flags.flag(5) );
		CHECK (     flags.flag(6) );
		CHECK ( not flags.flag(7) );
	}

	SECTION ( "Reading a flag works" )
	{
		auto flags = Flags { 1 << 6 };
		flags.set_flag(0, true);
		flags.set_flag(2, true);

		CHECK (     flags.flag(0) );
		CHECK ( not flags.flag(1) );
		CHECK (     flags.flag(2) );
		CHECK ( not flags.flag(3) );
		CHECK ( not flags.flag(4) );
		CHECK ( not flags.flag(5) );
		CHECK (     flags.flag(6) );
		CHECK ( not flags.flag(7) );
	}

	SECTION ( "Setting a flag works" )
	{
		auto flags = Flags {};

		REQUIRE (     flags.no_flags() );
		REQUIRE ( not flags.only_one_flag() );

		REQUIRE ( not flags.flag(0) );
		REQUIRE ( not flags.flag(1) );
		REQUIRE ( not flags.flag(2) );
		REQUIRE ( not flags.flag(3) );
		REQUIRE ( not flags.flag(4) );
		REQUIRE ( not flags.flag(5) );
		REQUIRE ( not flags.flag(6) );
		REQUIRE ( not flags.flag(7) );

		flags.set_flag(4, true);

		CHECK ( not flags.no_flags() );
		CHECK (     flags.only(4) );
		CHECK (     flags.only_one_flag() );

		CHECK ( not flags.flag(0) );
		CHECK ( not flags.flag(1) );
		CHECK ( not flags.flag(2) );
		CHECK ( not flags.flag(3) );
		CHECK (     flags.flag(4) );
		CHECK ( not flags.flag(5) );
		CHECK ( not flags.flag(6) );
		CHECK ( not flags.flag(7) );

		flags.set_flag(7, true);

		CHECK ( not flags.no_flags() );
		CHECK ( not flags.only(4) );
		CHECK ( not flags.only_one_flag() );

		CHECK ( not flags.flag(0) );
		CHECK ( not flags.flag(1) );
		CHECK ( not flags.flag(2) );
		CHECK ( not flags.flag(3) );
		CHECK (     flags.flag(4) );
		CHECK ( not flags.flag(5) );
		CHECK ( not flags.flag(6) );
		CHECK (     flags.flag(7) );

		flags.set_flag(7, false);

		CHECK ( not flags.no_flags() );
		CHECK (     flags.only(4) );
		CHECK (     flags.only_one_flag() );

		CHECK ( not flags.flag(0) );
		CHECK ( not flags.flag(1) );
		CHECK ( not flags.flag(2) );
		CHECK ( not flags.flag(3) );
		CHECK (     flags.flag(4) );
		CHECK ( not flags.flag(5) );
		CHECK ( not flags.flag(6) );
		CHECK ( not flags.flag(7) );
	}
}


TEST_CASE ( "FlagStore", "[flagstore]" )
{

}

