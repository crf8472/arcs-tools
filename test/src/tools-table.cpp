#include "catch2/catch_test_macros.hpp"

#ifndef __ARCSTOOLS_TOOLS_TABLE_HPP__
#include "tools-table.hpp"
#endif
#ifndef __ARCSTOOLS_APPVERIFY_HPP__
#include "app-verify.hpp"     // for MatchDecorator
#endif

// TODO Testcase RowTableComposer
// TODO Testcase ColTableComposer
// TODO Testcase RowTableComposerBuilder
// TODO Testcase ColTableComposerBuilder
// TODO Testcase TableFormatter


// FIXME Should go to unit 'app-verify' since MatchDecorator is verify specific
TEST_CASE ( "MatchDecorator", "[matchdecorator]" )
{
	using arcsapp::MatchDecorator;
	using arcsapp::table::CellDecorator;

	using arcsapp::ansi::Color;
	using arcsapp::ansi::Highlight;

	std::unique_ptr<CellDecorator> d {
		std::make_unique<MatchDecorator>(1,
				Highlight::BOLD, Color::FG_GREEN, Color::BG_DEFAULT,
				Highlight::BOLD, Color::FG_RED, Color::BG_DEFAULT)
	};

	SECTION ( "Constructor works as expected" )
	{
		CHECK ( not d->is_set(0) );
	}
}

