#include "catch2/catch_test_macros.hpp"


/**
 * \file
 *
 * \brief Fixtures for tools-input.hpp.
 */

#ifndef __ARCSTOOLS_TOOLS_INFO_HPP__
#include "tools-input.hpp"        // TO BE TESTED
#endif

#include <cstdint>                // for uint32_t
#include <string>                 // for string, stoul


TEST_CASE ( "parse_list_to_objects()", "[parse_list_to_objects]" )
{
	using arcsapp::input::parse_list_to_objects;

	SECTION ("Parse non-empty lists of hex values successfully")
	{
		const auto list1 { "0x98B10E0F,0x475F57E9,0x7304F1C4" };

		const auto res1 { parse_list_to_objects<uint32_t>(list1, ',',
				[](const std::string& s) -> uint32_t
				{
					return std::stoul(s, nullptr, 16);
				}) };

		CHECK ( 3 == res1.size() );
		CHECK ( 0x98B10E0F == res1[0] );
		CHECK ( 0x475F57E9 == res1[1] );
		CHECK ( 0x7304F1C4 == res1[2] );

		const auto list2 { "98B10E0F,475F57E9,7304F1C4" };

		const auto res2 { parse_list_to_objects<uint32_t>(list2, ',',
				[](const std::string& s) -> uint32_t
				{
					return std::stoul(s, nullptr, 16);
				}) };

		CHECK ( 3 == res2.size() );
		CHECK ( 0x98B10E0F == res2[0] );
		CHECK ( 0x475F57E9 == res2[1] );
		CHECK ( 0x7304F1C4 == res2[2] );
	}

}

