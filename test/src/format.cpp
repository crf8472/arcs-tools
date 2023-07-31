#include "catch2/catch_test_macros.hpp"

#ifndef __ARCSTOOLS_FORMAT_HPP__
#include "format.hpp"
#endif
#ifndef __ARCSTOOLS_APPVERIFY_HPP__
#include "app-verify.hpp"     // for MatchDecorator
#endif

// TODO Testcase RichARId
// TODO Testcase RowTableComposer
// TODO Testcase ColTableComposer
// TODO Testcase RowTableComposerBuilder
// TODO Testcase ColTableComposerBuilder
// TODO ResultFormatter


TEST_CASE ( "MatchDecorator", "[matchdecorator]" )
{
	using arcsapp::CellDecorator;
	using arcsapp::MatchDecorator;
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

