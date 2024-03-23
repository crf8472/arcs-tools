#ifndef __ARCSTOOLS_ANSI_HPP__
#define __ARCSTOOLS_ANSI_HPP__

/**
 * \file
 *
 * \brief ANSI output modifiers for the command line.
 */

#include <ostream>                // for ostream
#include <string>                 // for to_string
#include <sstream>                // for ostringstream
#include <vector>

namespace arcsapp
{

/**
 * \brief ANSI color management.
 */
namespace ansi
{


/**
 * \brief ANSI highlight codes.
 *
 * According to https://en.wikipedia.org/wiki/ANSI_escape_code#Colors
 */
enum class Highlight : int
{
	NORMAL =  0,
	//
	BOLD   =  1,
	FAINT  =  2,
	UNDERL =  4,
	BLINK  =  5,
	//
	NOBOLD   = 22,
	NOFAINT  = 22,
	NOUNDERL = 24,
	NOBLINK  = 25
};


/**
 * \brief Get the reset code for the highlight.
 */
Highlight reset(const Highlight hl);


/**
 * \brief ANSI color codes.
 *
 * According to https://en.wikipedia.org/wiki/ANSI_escape_code#Colors
 */
enum class Color : int
{
	NONE        =  0,
	//
	FG_BLACK    = 30,
	FG_RED      = 31,
	FG_GREEN    = 32,
	FG_YELLOW   = 33,
	FG_BLUE     = 34,
	FG_MAGENTA  = 35,
	FG_CYAN     = 36,
	FG_WHITE    = 37,
	FG_DEFAULT  = 39,
	//
	BG_BLACK    = 40,
	BG_RED      = 41,
	BG_GREEN    = 42,
	BG_YELLOW   = 43,
	BG_BLUE     = 44,
	BG_MAGENTA  = 45,
	BG_CYAN     = 46,
	BG_WHITE    = 47,
	BG_DEFAULT  = 49
};


/**
 * \brief Return color for name.
 *
 * \param[in] name Name of the requested color
 *
 * \return Color for the passed name
 */
Color get_color(const std::string& name);


class Modifier;

std::ostream& operator << (std::ostream& o, const Modifier& m);


/**
 * \brief Color modifier that encodes the color and the highlight.
 */
class Modifier
{
	/**
	 * \brief Internal highlight.
	 */
	Highlight hl_;

	/**
	 * \brief List of codes to use.
	 */
	std::vector<Color> colors_;

	/**
	 * \brief Return SGR string of color codes.
	 *
	 * \return String representation of color codes for SGR
	 */
	std::string colors_str() const;

public:

	friend std::ostream& operator << (std::ostream& o, const Modifier& m);

	/**
	 * \brief Constructor.
	 *
	 * \param[in] hl     Highlight to set
	 * \param[in] colors Color to set
	 */
	Modifier(Highlight hl, const std::vector<Color>& colors);

	/**
	 * \brief Constructor.
	 *
	 * Sets no colors.
	 *
	 * \param[in] hl Highlight to set
	 */
	Modifier(Highlight hl);

	/**
	 * \brief Highlight of this modifier.
	 *
	 * \return String representation of the highlight of this modifier
	 */
	Highlight highlight() const;

	/**
	 * \brief Color of this modifier.
	 *
	 * \return String representation of the color of this modifier
	 */
	std::vector<Color> colors() const;

	/**
	 * \brief Modifier string
	 *
	 * \return String representation of this modifier
	 */
	std::string str() const;
};


/**
 * \brief Colorize a string.
 *
 * The colorized string will have trailing modifier for resetting the highlight.
 *
 * \param[in] hl       Highlight to use
 * \param[in] color_fg Foreground color to use
 * \param[in] color_bg Background color to use
 * \param[in] s        The string to be colorized
 *
 * \return Colorized string
 */
std::string colored(Highlight hl, const Color color_fg, const Color color_bg,
		const std::string& s);

} // namespace ansi
} // namespace arcsapp

#endif

