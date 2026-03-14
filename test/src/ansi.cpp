#include "catch2/catch_test_macros.hpp"

/**
 * \file
 *
 * \brief Fixtures for ansi.hpp.
 */

#ifndef ARCSTOOLS_ANSI_HPP__
#include "ansi.hpp"               // TO BE TESTED
#endif

#include <vector>                 // for vector


// TODO reset()
// TODO get_color()
// TODO colored()

TEST_CASE ( "is_foreground", "" )
{
	using arcsapp::ansi::Color;

	SECTION ( "is correct" )
	{
		CHECK (		is_foreground(Color::FG_BLACK)   );
		CHECK (		is_foreground(Color::FG_RED)     );
		CHECK (		is_foreground(Color::FG_GREEN)   );
		CHECK (		is_foreground(Color::FG_YELLOW)  );
		CHECK (		is_foreground(Color::FG_BLUE)    );
		CHECK (		is_foreground(Color::FG_MAGENTA) );
		CHECK (		is_foreground(Color::FG_CYAN)    );
		CHECK (		is_foreground(Color::FG_WHITE)   );
		CHECK (		is_foreground(Color::FG_DEFAULT) );

		CHECK ( not is_foreground(Color::BG_BLACK)   );
		CHECK ( not is_foreground(Color::BG_RED)     );
		CHECK ( not is_foreground(Color::BG_GREEN)   );
		CHECK ( not is_foreground(Color::BG_YELLOW)  );
		CHECK ( not is_foreground(Color::BG_BLUE)    );
		CHECK ( not is_foreground(Color::BG_MAGENTA) );
		CHECK ( not is_foreground(Color::BG_CYAN)    );
		CHECK ( not is_foreground(Color::BG_WHITE)   );
		CHECK ( not is_foreground(Color::BG_DEFAULT) );

		CHECK ( not is_foreground(Color::NONE) );
	}
}

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

