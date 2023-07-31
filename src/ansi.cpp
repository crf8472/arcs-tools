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


Highlight reset(const Highlight hl)
{
	static const std::unordered_map<Highlight, Highlight> highlights = {
		{ Highlight::NORMAL, Highlight::NORMAL   },
		{ Highlight::BOLD,   Highlight::NOBOLD   },
		{ Highlight::FAINT,  Highlight::NOFAINT  },
		{ Highlight::UNDERL, Highlight::NOUNDERL },
		{ Highlight::BLINK,  Highlight::NOBLINK  }
	};

	using std::end;

	if (const auto r { highlights.find(hl) }; end(highlights) != r)
	{
		return r->second;
	}

	return Highlight::NORMAL;
}


Color get_color(const std::string& name)
{
	static const std::unordered_map<std::string, Color> colors = {
		{ "FG_BLACK",   Color::FG_BLACK },
		{ "FG_RED",     Color::FG_RED },
		{ "FG_GREEN",   Color::FG_GREEN },
		{ "FG_YELLOW",  Color::FG_YELLOW },
		{ "FG_BLUE",    Color::FG_BLUE },
		{ "FG_MAGENTA", Color::FG_MAGENTA },
		{ "FG_CYAN",    Color::FG_CYAN },
		{ "FG_WHITE",   Color::FG_WHITE },
		{ "FG_DEFAULT", Color::FG_DEFAULT },
		//
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

	using std::end;
	if (const auto& c = colors.find(name); c != end(colors))
	{
		return c->second;
	}

	return Color::NONE;
}


// Modifier


Modifier::Modifier(Highlight hl, const std::vector<Color>& colors)
	: hl_     { hl }
	, colors_ { colors }
{
	/* empty */
}


Modifier::Modifier(Highlight hl)
	: hl_     { hl }
	, colors_ { /* empty */ }
{
	/* empty */
}


Highlight Modifier::highlight() const
{
	return hl_;
}


std::vector<Color> Modifier::colors() const
{
	return colors_;
}


std::string Modifier::colors_str() const
{
	if (colors_.empty())
	{
		return "";
	}

	std::ostringstream s;

	using std::to_string;
	for (const auto& c : colors_)
	{
		s << ";" << to_string(static_cast<std::underlying_type_t<Color>>(c));
	}

	return s.str();
}


std::string Modifier::str() const
{
	using std::to_string;
	return "\x1B["
		+ to_string(static_cast<std::underlying_type_t<Highlight>>(hl_))
		+ colors_str()
		+ "m";
}


std::ostream& operator << (std::ostream& o, const Modifier& m)
{
	return o << m.str();
}


// colored


std::string colored(const Highlight hl, const Color color_fg,
		const Color color_bg, const std::string& s)
{
	return Modifier{ hl, { color_fg, color_bg } }.str()
			+ s
			+ Modifier{ reset(hl),
				{ Color::FG_DEFAULT, Color::BG_DEFAULT } }.str();
}


} // namespace ansi
} // namespace arcsapp

