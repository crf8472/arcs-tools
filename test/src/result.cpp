#include "catch2/catch_test_macros.hpp"

#ifndef __ARCSTOOLS_RESULT_HPP__
#include "result.hpp"
#endif

/**
 * \file
 *
 * \brief Fixtures for result.cpp
 */

// TODO Result
// TODO ResultList
// TODO ResultObject

TEST_CASE ( "ResultBuffer", "[resultbuffer]" )
{
	using arcsapp::ResultBuffer;

	auto buf = ResultBuffer{};

	SECTION ( "Default construction works" )
	{
		CHECK ( buf.flush() != nullptr );
	}
}

