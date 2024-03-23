#include "catch2/catch_test_macros.hpp"

#ifndef __ARCSTOOLS_TOOLS_PARSE_HPP__
#include "tools-parse.hpp"
#endif


TEST_CASE ( "DBARTripletLayout", "[artripletlayout]" )
{
	using arcsapp::DBARTripletLayout;
	using arcstk::DBARTriplet;

	auto t = DBARTriplet { 0xFFAABBCC, 24, 0x12345678 };

	REQUIRE ( t.arcs() == 0xFFAABBCCu );
	REQUIRE ( t.confidence() == 24 );
	REQUIRE ( t.frame450_arcs() == 0x12345678u );

	DBARTripletLayout lyt;

	CHECK ( not lyt.format(8, t).empty() );
}

