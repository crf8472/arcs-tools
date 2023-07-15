/**
 * \file
 *
 * \brief Implements symbols from ansi.hpp.
 */

#ifndef __ARCSTOOLS_ANSI_HPP__
#include "ansi.hpp"
#endif

#include <unordered_map>               // for unordered_map
#include <type_traits>                 // for underlying_type_t

namespace arcsapp
{
namespace ansi
{


Color get_color(const std::string& name)
{
	static const std::unordered_map<std::string, Color> values = {
		{ "FG_BLACK",   Color::FG_BLACK },
		{ "FG_RED",     Color::FG_RED },
		{ "FG_GREEN",   Color::FG_GREEN },
		{ "FG_YELLOW",  Color::FG_YELLOW },
		{ "FG_BLUE",    Color::FG_BLUE },
		{ "FG_MAGENTA", Color::FG_MAGENTA },
		{ "FG_CYAN",    Color::FG_CYAN },
		{ "FG_WHITE",   Color::FG_WHITE },
		{ "FG_DEFAULT", Color::FG_DEFAULT },
		{ "BG_BLACK",   Color::BG_BLACK },
		{ "BG_RED",     Color::BG_RED },
		{ "BG_GREEN",   Color::BG_GREEN },
		{ "BG_YELLOW",  Color::BG_YELLOW },
		{ "BG_BLUE",    Color::BG_BLUE },
		{ "BG_MAGENTA", Color::BG_MAGENTA },
		{ "BG_CYAN",    Color::BG_CYAN },
		{ "BG_WHITE",   Color::BG_WHITE },
		{ "BG_DEFAULT", Color::BG_DEFAULT }
	};

	return values.find(name)->second;
}


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

