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
 * \brief Tool class to print supported formats
 */
class SupportedFormats final
{

public:

	static const StringTable& audio();

	static const StringTable& toc();
};

} // namespace arcsapp

#endif

