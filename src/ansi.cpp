/**
 * \file
 *
 * \brief Implements symbols from ansi.hpp.
 */

#ifndef __ARCSTOOLS_ANSI_HPP__
#include "ansi.hpp"
#endif

#include <type_traits>                 // for underlying_type_t

namespace arcsapp
{
namespace ansi
{

// Modifier


Modifier::Modifier(Color c, Highlight h)
	: color_ { c }
	, hl_    { h }
{
	// empty
}


Modifier::Modifier(Color c)
	: Modifier{c, Highlight::BRIGHT} { /* empty */ }


std::string Modifier::color() const
{
	using std::to_string;
	return to_string(static_cast<std::underlying_type_t<Color>>(color_));
}


std::string Modifier::highlight() const
{
	using std::to_string;
	return to_string(static_cast<std::underlying_type_t<Highlight>>(hl_));
}


std::string Modifier::to_string() const
{
	return "\x1B[" + highlight() + ";" + color() + "m";
}


std::ostream& operator << (std::ostream& o, const Modifier& m)
{
	return o << m.to_string();
}


// colored


std::string colored(Color c, Highlight h, const std::string& s)
{
	return Modifier{c, h}.to_string()
			+ s + Modifier{Color::FG_DEFAULT, Highlight::NORMAL}.to_string();
}


} // namespace ansi
} // namespace arcsapp

