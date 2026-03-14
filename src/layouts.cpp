/**
 * \internal
 *
 * \file
 *
 * \brief Implements symbols from layouts.hpp.
 */

#ifndef ARCSTOOLS_LAYOUTS_HPP_
#include "layouts.hpp"
#endif


namespace arcsapp
{
inline namespace v_1_0_0
{

// Flags


Flags::Flags(const flags_t flags)
	: flags_ { flags }
{
	// empty
}


Flags::Flags()
	: Flags { 0 }
{
	// empty
};


void Flags::set(const int idx)
{
	flags_ |= (1u << idx);
}


void Flags::unset(const int idx)
{
	flags_ &= ~(1u << idx);
}


void Flags::set_flag(const int idx, const bool value)
{
	if (value) { set(idx); } else { unset(idx); }
}


bool Flags::flag(const int idx) const
{
	return flags_ & (1u << idx);
}


bool Flags::no_flags() const
{
	return flags_ == 0;
}


bool Flags::only_one_flag() const
{
	return flags_ && !(flags_ & (flags_ - 1));
}


bool Flags::only(const int idx) const
{
	return flag(idx) && only_one_flag();
}

} // namespace v_1_0_0
} // namespace arcsapp

