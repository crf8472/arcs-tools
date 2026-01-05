#include "catch2/catch_test_macros.hpp"

/**
 * \file
 *
 * \brief Fixtures for ansi.hpp.
 */

#ifndef __ARCSTOOLS_ANSI_HPP__
#include "ansi.hpp"               // TO BE TESTED
#endif

#include <vector>                 // for vector


// TODO reset()
// TODO get_color()
// TODO colored()

TEST_CASE ( "Modifier", "[modifier]" )
{
	using arcsapp::ansi::Color;
	using arcsapp::ansi::Highlight;
	using arcsapp::ansi::Modifier;

	auto m1 = Modifier { Highlight::BOLD, { Color::FG_BLACK, Color::BG_RED } };
	auto m2 = Modifier { Highlight::UNDERL };

	SECTION ( "Default construction works" )
	{
		CHECK ( m1.highlight() == Highlight::BOLD );
		CHECK ( m1.colors()    ==
				std::vector<Color>{ Color::FG_BLACK, Color::BG_RED } );
		CHECK ( m1.str()       == "\x1B[1;30;41m" );

		CHECK ( m2.highlight() == Highlight::UNDERL );
		CHECK ( m2.colors()    == std::vector<Color>{/*empty*/} );
		CHECK ( m2.str()       == "\x1B[4m" );
	}
}

