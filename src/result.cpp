#ifndef __ARCSTOOLS_RESULT_HPP__
#include "result.hpp"
#endif

/**
 * \file
 *
 * \brief Result interface implementation
 */


#include <memory>         // for unique_ptr
#include <ostream>        // for ostream
#include <utility>        // for move


namespace arcsapp
{
inline namespace v_1_0_0
{

// Result


void Result::print(std::ostream& o) const
{
	this->do_print(o);
}


std::ostream& operator << (std::ostream& o, const Result& result)
{
	result.print(o);
	return o;
}


// ResultList


void ResultList::append(std::unique_ptr<Result> r)
{
	results_.push_back(std::move(r));
}


void ResultList::do_print(std::ostream& o) const
{
	for (const auto& p : results_)
	{
		o << *p;
	}
}


// ResultProvider


std::unique_ptr<Result> ResultProvider::result() const
{
	return do_result();
}

} // namespace v_1_0_0
} // namespace arcsapp

