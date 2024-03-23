/**
 * \file
 *
 * \brief Formatting interfaces and classes
 */

#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#include "layouts.hpp"
#endif

#include <cstdint>             // for uint32_t, uint16_t
#include <iomanip>             // for setw, setfill
#include <ios>                 // for left, showbase, hex, uppercase
#include <memory>              // for unique_ptr, make_unique
#include <sstream>             // for ostringstream
#include <string>              // for string, to_string
#include <tuple>               // for get

#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>   // for Checksum
#endif

namespace arcsapp
{
inline namespace v_1_0_0
{

// InternalFlags


InternalFlags::InternalFlags(const uint32_t flags)
	: flags_(flags)
{
	// empty
}


void InternalFlags::set_flag(const int idx, const bool value)
{
	if (value)
	{
		flags_ |= (1 << idx);  // <= true
	} else
	{
		flags_ &= ~(1 << idx); // <= false
	}
}


bool InternalFlags::flag(const int idx) const
{
	return flags_ & (1 << idx);
}


bool InternalFlags::no_flags() const
{
	return flags_ == 0;
}


bool InternalFlags::only_one_flag() const
{
	return flags_ && !(flags_ & (flags_ - 1));
}


bool InternalFlags::only(const int idx) const
{
	return flag(idx) && only_one_flag();
}

} // namespace v_1_0_0
} // namespace arcsapp

