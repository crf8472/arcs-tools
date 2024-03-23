#include "catch2/catch_test_macros.hpp"

#ifndef __ARCSTOOLS_TOOLS_ARID_HPP__
#include "tools-arid.hpp"
#endif

// TODO Testcase RichARId

TEST_CASE ( "ARIdTableLayout", "[aridtablelayout]" )
{
	using arcsapp::arid::ARIdTableLayout;

	ARIdTableLayout lyt(false, false, true, false, false, false, false, false);

	CHECK ( not lyt.fieldlabels() );
	CHECK ( not lyt.id() );
	CHECK (     lyt.url() );
	CHECK ( not lyt.filename() );
	CHECK ( not lyt.track_count() );
	CHECK ( not lyt.disc_id_1() );
	CHECK ( not lyt.disc_id_2() );
	CHECK ( not lyt.cddb_id() );

	CHECK ( not lyt.format({ 15, 0x001b9178, 0x014be24e, 0xb40d2d0f }, "")
			.empty() );
}

