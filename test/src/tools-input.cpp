#include "catch2/catch_test_macros.hpp"

/**
 * \file
 *
 * \brief Fixtures for tools-input.hpp.
 */

#ifndef __ARCSTOOLS_TOOLS_INPUT_HPP__
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


TEST_CASE ( "ChecksumValuesParser", "[checksumvaluesparser]" )
{
	using arcsapp::input::ChecksumValuesParser;
	using arcsapp::input::ChecksumValuesType;

	const auto parser = ChecksumValuesParser {};

	SECTION ( "Parsing non-empty correct list works" )
	{
		const auto input = std::string {
			"0xB89992E5,0x4F77EB03,0x56582282,0x9E2187F9,0x6BE71E50,"
			"0x01E7235F,0xD8F7763C,0x8480223E,0x42C5061C,0x47A70F02,"
			"0xBABF08CC,0x563EDCCB,0xAB123C7C,0xC65C20E4,0x58FC3C3E"
		};

		REQUIRE ( 164 == input.size() );

		const auto ref_output = ChecksumValuesType {
			0xB89992E5, 0x4F77EB03, 0x56582282, 0x9E2187F9, 0x6BE71E50,
			0x01E7235F, 0xD8F7763C, 0x8480223E, 0x42C5061C, 0x47A70F02,
			0xBABF08CC, 0x563EDCCB, 0xAB123C7C, 0xC65C20E4, 0x58FC3C3E
		};

		const auto output = parser.parse(input); // std::any

		//CHECK ( output.size() == 1 );
		//CHECK ( output.at( 1) == 0xB89992E5 );
		//CHECK ( output.at(15) == 0x58FC3C3E );
	}
}

