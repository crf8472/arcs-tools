#include "catch2/catch_test_macros.hpp"

/**
 * \file
 *
 * \brief Fixtures for result.hpp.
 */

#ifndef ARCSTOOLS_RESULT_HPP_
#include "result.hpp"             // TO BE TESTED
#endif


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

