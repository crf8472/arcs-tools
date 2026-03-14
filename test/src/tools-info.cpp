#include "catch2/catch_test_macros.hpp"

/**
 * \file
 *
 * \brief Fixtures for tools-info.hpp.
 */

#ifndef ARCSTOOLS_TOOLSINFO_HPP_
#include "tools-info.hpp"         // TO BE TESTED
#endif


// TODO InfoResultComposer
// TODO AudioFormatComposer
// TODO ToCFormatComposer

TEST_CASE ( "AvailableFileReaders", "[availablefilereaders]" )
{
	using arcsapp::info::AvailableFileReaders;

	SECTION ( "At least one reader for audio and one reader for meta" )
	{
		CHECK ( not AvailableFileReaders::audio().empty()  );
		CHECK (     AvailableFileReaders::audio().rows()  >= 1 );

		CHECK ( not AvailableFileReaders::toc().empty()    );
		CHECK (     AvailableFileReaders::toc().rows()    >= 1 );
	}
}

