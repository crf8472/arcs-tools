#ifndef ARCSTOOLS_ANSI_HPP_
#define ARCSTOOLS_ANSI_HPP_
/**
 * \file
 *
 * \brief ANSI output modifiers for the command line.
 */

#include <ostream>                // for ostream
#include <string>                 // for to_string
#include <vector>                 // for vector


namespace arcsapp
{
inline namespace v_1_0_0
{

/**
 * \brief ANSI color and SGR management.
 */
namespace ansi
{


/**
 * \brief Subset of ANSI SGR codes.
 *
 * Supports Reset/Normal, Bold, Faint/Dim, Underline and SlowBlink.
 *
 * According to https://en.wikipedia.org/wiki/ANSI_escape_code#Select_Graphic_Rendition_parameters
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
	NOBOLD   = 22, // normal intensity, also resets FAINT
	NOFAINT  = 22, // normal intensity, also resets BOLD
	NOUNDERL = 24,
	NOBLINK  = 25
};


/**
 * \brief Get the reset code for the highlight.
 *
 * \param[in] hl Highlight to get reset code for
 *
 * \return Reset code for \p hl
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


/**
 * \brief TRUE iff color \c c is a foreground color, otherwise FALSE.
 *
 * \param[in] c Color to test
 *
 * \return TRUE iff \c c is a foreground color, otherwise FALSE.
 */
bool is_foreground(const Color c);


class Modifier;
std::ostream& operator << (std::ostream& o, const Modifier& m);


/**
 * \brief ANSI modifier encoding a highlight SGR and a trailing set of colors.
 */
class Modifier // TODO May encode many HL-SGR codes instead of one
{
	/**
	 * \brief Internal highlight.
	 */
	Highlight hl_;

	/**
	 * \brief Internal list of colors to use.
	 */
	std::vector<Color> colors_;

	/**
	 * \brief Return SGR string of color codes.
	 *
	 * \return SGR string representation of trailing color codes for SGR
	 */
	std::string colors_str() const;

public:

	friend std::ostream& operator << (std::ostream& o, const Modifier& m);

	/**
	 * \brief Default constructor.
	 *
	 * Highlight will be NORMAL, no colors.
	 *
	 * Can be used to reset previous settings.
	 */
	Modifier();

	/**
	 * \brief Constructor.
	 *
	 * \param[in] hl     Highlight to set
	 * \param[in] colors Colors to set
	 */
	Modifier(Highlight hl, const std::vector<Color>& colors);

	/**
	 * \brief Constructor.
	 *
	 * Sets no colors.
	 *
	 * \param[in] hl Highlight to set
	 */
	explicit Modifier(Highlight hl);

	/**
	 * \brief Constructor.
	 *
	 * Sets default SGR (== 0).
	 *
	 * \param[in] colors Colors to set
	 */
	explicit Modifier(const std::vector<Color>& colors);

	/**
	 * \brief Highlight of this modifier.
	 *
	 * \return String representation of the highlight of this modifier
	 */
	Highlight highlight() const;

	/**
	 * \brief Colors of this modifier.
	 *
	 * \return List of colors of this modifier
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
} // namespace v_1_0_0
} // namespace arcsapp

#endif

