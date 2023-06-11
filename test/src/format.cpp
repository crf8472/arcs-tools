#include "catch2/catch_test_macros.hpp"

#ifndef __ARCSTOOLS_FORMAT_HPP__
#include "format.hpp"
#endif
#ifndef __ARCSTOOLS_APPVERIFY_HPP__
#include "app-verify.hpp"     // for MatchDecorator
#endif


TEST_CASE ( "MatchDecorator", "[matchdecorator]" )
{
	using arcsapp::CellDecorator;
	using arcsapp::MatchDecorator;

	std::unique_ptr<CellDecorator> d { std::make_unique<MatchDecorator>(1) };

	SECTION ( "Constructor works as expected" )
	{
		CHECK ( not d->is_set(0) );
	}
}


TEST_CASE ( "DecoratedStringTable", "[DecoratedStringTable]" )
{
	using arcsapp::DecoratedStringTable;
	using arcsapp::MatchDecorator;
	using arcsapp::table::Align;

	DecoratedStringTable t{ 5, 3 };

	t(0,0) = "foo";
	t(0,1) = "quux";
	t(0,2) = "bar";

	t(1,0) = "blubb";
	t(1,1) = "ti";
	t(1,2) = "ta";

	t(2,0) = "mor";
	t(2,1) = "quark";
	t(2,2) = "sem";

	t(3,0) = "trg";
	t(3,1) = "hkpn";
	t(3,2) = "tknr";

	t(4,0) = "plf";
	t(4,1) = "grb";
	t(4,2) = "st";

	t.set_align(1, Align::RIGHT);

	// Add decorator to column 2

	auto d0 = std::make_unique<MatchDecorator>(5 /*rows*/);
	t.register_to_col(2, std::move(d0));



	SECTION ("Decorators added")
	{
		CHECK( t.col_decorator(2) != nullptr );
	}

	SECTION ("Decorator for non-existant column is null")
	{
		CHECK( t.col_decorator(3) == nullptr );
	}
}

