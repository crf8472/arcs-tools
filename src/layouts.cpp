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

#ifndef __LIBARCSTK_IDENTIFIER_HPP__
#include <arcstk/identifier.hpp>  // for ARId
#endif
#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>   // for Checksum
#endif
#ifndef __LIBARCSTK_DBAR_HPP__
#include <arcstk/dbar.hpp>        // for DBARTriplet
#endif

#ifndef __ARCSTOOLS_RESULT_HPP__
#include "result.hpp"             // for ResultObject
#endif

namespace arcsapp
{

using arcstk::Checksum;


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


// HexLayout


HexLayout::HexLayout()
	: WithInternalFlags()
{
	set_show_base(false);
	set_uppercase(true);
}


void HexLayout::set_show_base(const bool base)
{
	flags().set_flag(0, base);
}


bool HexLayout::shows_base() const
{
	return flags().flag(0);
}


void HexLayout::set_uppercase(const bool uppercase)
{
	flags().set_flag(1, uppercase);
}


bool HexLayout::is_uppercase() const
{
	return flags().flag(1);
}


std::string HexLayout::do_format(InputTuple t) const
{
	auto checksum = std::get<0>(t);
	auto width    = std::get<1>(t);

	std::ostringstream ss;

	if (shows_base())
	{
		ss << std::showbase;
	}

	if (is_uppercase())
	{
		ss << std::uppercase;
	}

	ss << std::hex << std::setw(width) << std::setfill('0') << checksum.value();

	return ss.str();
}


// DBARTripletLayout


std::string DBARTripletLayout::do_format(InputTuple t) const
{
	const auto track   = std::get<0>(t);
	const auto triplet = std::get<1>(t);

	HexLayout hex; // TODO Make this configurable, inherit from WithChecksums...
	hex.set_show_base(false);
	hex.set_uppercase(true);

	const int width_arcs = 8;
	const int width_conf = 2;

	const auto unparsed_value = std::string { "????????" };

	std::ostringstream out;

	// TODO Make label configurable
	out << "Track " << std::setw(2) << std::setfill('0') << track << ": ";

	out << std::setw(width_arcs)
			<< hex.format(Checksum { triplet.arcs() }, width_arcs);

	out << " ";

	out << "(";
	out << std::setw(width_conf) << std::setfill('0')
			<< static_cast<unsigned int>(triplet.confidence());
	out << ") ";

	out << std::setw(width_arcs)
			<< hex.format(Checksum { triplet.frame450_arcs() }, width_arcs);

	out << '\n';

	return out.str();
}

} // namespace arcsapp

