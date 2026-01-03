#include "catch2/catch_test_macros.hpp"

#ifndef __ARCSTOOLS_TOOLS_ARID_HPP__
#include "tools-arid.hpp"
#endif

// TODO Testcase RichARId

TEST_CASE ( "ARIdTableLayout", "[aridtablelayout]" )
{
	using arcsapp::arid::ARIdTableLayout;

	ARIdTableLayout lyt1(false, false, true, false, false, false, false, false);

	SECTION ( "Constructor flags are correctly set" )
	{
		CHECK ( not lyt1.fieldlabels() );
		CHECK ( not lyt1.id() );
		CHECK (     lyt1.url() );
		CHECK ( not lyt1.filename() );
		CHECK ( not lyt1.track_count() );
		CHECK ( not lyt1.disc_id_1() );
		CHECK ( not lyt1.disc_id_2() );
		CHECK ( not lyt1.cddb_id() );
	}

	// set_fieldlabels()
	// set_id()
	// set_url()
	// set_filename()
	// set_track_count()
	// set_disc_id1()
	// set_disc_id2()
	// set_cddb_id()

	// label()
	// set_label()
	// has_only()

	SECTION ( "format() has non-empty result" )
	{
		CHECK ( not lyt1.format({ 15, 0x001b9178, 0x014be24e, 0xb40d2d0f }, "")
				.empty() );
	}
}

