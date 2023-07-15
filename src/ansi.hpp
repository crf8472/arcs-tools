#ifndef __ARCSTOOLS_ANSI_HPP__
#define __ARCSTOOLS_ANSI_HPP__

/**
 * \file
 *
 * \brief ANSI output modifiers for the command line.
 */

#include <ostream>                // for ostream
#include <string>                 // for to_string

namespace arcsapp
{
namespace ansi
{

/**
 * \brief ANSI color codes.
 *
 * According to https://en.wikipedia.org/wiki/ANSI_escape_code#Colors
 */
enum class Color : int
{
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


/**
 * \brief ANSI highlight codes.
 *
 * According to https://en.wikipedia.org/wiki/ANSI_escape_code#Colors
 */
enum class Highlight : int
{
	BRIGHT =  0,
	NORMAL =  1
};


class Modifier;
std::ostream& operator << (std::ostream& o, const Modifier& m);


/**
 * \brief Color modifier that encodes the color and the highlight.
 */
class Modifier
{
	Color     color_;
	Highlight hl_;

public:

	explicit Modifier(Color c, Highlight hl);

	Modifier(Color c);

	std::string color() const;

	std::string highlight() const;

	std::string to_string() const;

	friend std::ostream& operator << (std::ostream& o, const Modifier& m);
};


/**
 * \brief Colorize a string.
 */
std::string colored(Color c, Highlight h, const std::string& s);


/**
 * \brief Colorize a string.
 */
template <Color C, Highlight H = Highlight::BRIGHT>
struct Colorize
{
	inline std::string operator() (const std::string& s) const
	{
		return colored(C, H, s);
	}
};


} // namespace ansi
} // namespace arcsapp

#endif

