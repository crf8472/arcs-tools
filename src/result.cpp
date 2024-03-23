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
	results_.emplace_back(std::move(r));
}


void ResultList::do_print(std::ostream& o) const
{
	for (const auto& p : results_)
	{
		o << *p;
	}
}


// ResultBuffer


ResultBuffer::ResultBuffer()
	: list_ { std::make_unique<ResultList>() }
{
	// empty
}


std::unique_ptr<Result> ResultBuffer::flush()
{
	std::unique_ptr<Result> r { std::move(list_) };
	list_ = std::make_unique<ResultList>();
	return r;
}

} // namespace v_1_0_0
} // namespace arcsapp

