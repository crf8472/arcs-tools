#include "catch2/catch_test_macros.hpp"

/**
 * \file
 *
 * \brief Fixtures for tools-arid.hpp.
 */

#ifndef ARCSTOOLS_TOOLS_ARID_HPP_
#include "tools-arid.hpp"         // TO BE TESTED
#endif


// TODO Testcase RichARId

TEST_CASE ( "ARIdTableLayout", "[aridtablelayout]" )
{
	using arcsapp::arid::ARID_FLAG;
	using arcsapp::arid::ARIdTableLayout;

	ARIdTableLayout lyt1(false, false, true, false, false, false, false, false);

	SECTION ( "Default construction works" )
	{
		const auto lyt = ARIdTableLayout {};

		CHECK ( lyt.fieldlabels() );
		CHECK ( lyt.has_property(ARID_FLAG::ID) );
		CHECK ( lyt.has_property(ARID_FLAG::URL) );
		CHECK ( lyt.has_property(ARID_FLAG::FILENAME) );
		CHECK ( lyt.has_property(ARID_FLAG::TRACKS) );
		CHECK ( lyt.has_property(ARID_FLAG::ID1) );
		CHECK ( lyt.has_property(ARID_FLAG::ID2) );
		CHECK ( lyt.has_property(ARID_FLAG::CDDBID) );
	}

	SECTION ( "Constructor flags are correctly set" )
	{
		CHECK ( not lyt1.fieldlabels() );
		CHECK ( not lyt1.has_property(ARID_FLAG::ID) );
		CHECK (     lyt1.has_property(ARID_FLAG::URL) );
		CHECK ( not lyt1.has_property(ARID_FLAG::FILENAME) );
		CHECK ( not lyt1.has_property(ARID_FLAG::TRACKS) );
		CHECK ( not lyt1.has_property(ARID_FLAG::ID1) );
		CHECK ( not lyt1.has_property(ARID_FLAG::ID2) );
		CHECK ( not lyt1.has_property(ARID_FLAG::CDDBID) );
	}

	// set_fieldlabels()

	// label()
	// set_label()
	// has_only()

	SECTION ( "format() has non-empty result" )
	{
		CHECK ( not lyt1.format({ 15, 0x001b9178, 0x014be24e, 0xb40d2d0f }, "")
				.empty() );
	}
}

