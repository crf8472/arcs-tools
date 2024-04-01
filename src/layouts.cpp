/**
 * \file
 *
 * \brief Formatting interfaces and classes
 */

#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#include "layouts.hpp"
#endif

#include <cstdint>             // for uint32_t, uint16_t

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
		flags_ |=  (1u << idx); // <= true
	} else
	{
		flags_ &= ~(1u << idx); // <= false
	}
}


bool InternalFlags::flag(const int idx) const
{
	return flags_ & (1u << idx);
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

