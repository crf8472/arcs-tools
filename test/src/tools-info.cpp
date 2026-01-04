#include "catch2/catch_test_macros.hpp"

#ifndef __ARCSTOOLS_TOOLSINFO_HPP__
#include "tools-info.hpp"
#endif

/**
 * \file
 *
 * \brief Fixtures for tools-info.cpp
 */

// TODO InfoResultComposer
// TODO AudioFormatComposer
// TODO ToCFormatComposer

TEST_CASE ( "AvailableFileReaders", "[availablefilereaders]" )
{
	using arcsapp::AvailableFileReaders;

	SECTION ( "At least one reader for audio and one reader for meta" )
	{
		CHECK ( not AvailableFileReaders::audio().empty()  );
		CHECK (     AvailableFileReaders::audio().rows()  >= 1 );

		CHECK ( not AvailableFileReaders::toc().empty()    );
		CHECK (     AvailableFileReaders::toc().rows()    >= 1 );
	}
}

