#ifndef __ARCSTOOLS_TOOLS_INFO_HPP__
#define __ARCSTOOLS_TOOLS_INFO_HPP__

/**
 * \file
 *
 * \brief Tools for informational output.
 */

#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#include "layouts.hpp"          // for StringTable
#endif

namespace arcsapp
{

/**
 * \brief Tool class to get supported formats.
 */
class SupportedFormats final
{
public:

	/**
	 * \brief List of supported audio formats.
	 *
	 * \return List of supported audio formats.
	 */
	static const StringTable& audio();

	/**
	 * \brief List of supported toc formats.
	 *
	 * \return List of supported toc formats.
	 */
	static const StringTable& toc();
};

} // namespace arcsapp

#endif

