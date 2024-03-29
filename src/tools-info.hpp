#ifndef __ARCSTOOLS_TOOLS_INFO_HPP__
#define __ARCSTOOLS_TOOLS_INFO_HPP__

/**
 * \file
 *
 * \brief Tools for informational output.
 */

#ifndef __ARCSTOOLS_TABLE_HPP__
#include "table.hpp"               // for StringTable
#endif

namespace arcsapp
{
inline namespace v_1_0_0
{

using table::StringTable;

/**
 * \brief Lists of available FileReaders.
 */
class AvailableFileReaders final
{
public:

	/**
	 * \brief List of supported audio readers.
	 *
	 * \return List of supported audio readers.
	 */
	static const StringTable& audio();

	/**
	 * \brief List of supported toc readers.
	 *
	 * \return List of supported toc readers.
	 */
	static const StringTable& toc();
};

} // namespace v_1_0_0
} // namespace arcsapp

#endif

