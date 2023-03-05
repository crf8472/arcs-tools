#ifndef __ARCSTOOLS_TOOLS_INFO_HPP__
#define __ARCSTOOLS_TOOLS_INFO_HPP__

#include <algorithm>   // for transform
#include <functional>  // for function
#include <iterator>    // for ostream_iterator
#include <sstream>     // for ostringstream

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

namespace details
{

/**
 * \brief Transform container content to a delimiter separated list.
 */
template <typename Container> // TODO SFINAE stuff: empty(), size(), b+e, rbegin
std::string to_sep_list(const Container c, const std::string delim,
		const std::function<std::string(const typename Container::value_type &)>
		&f)
{
	if (c.empty())
	{
		return std::string{};
	}

	std::ostringstream list_stream;

	if (c.size() > 1)
	{
		std::transform(c.begin(), --c.rbegin().base(),
			std::ostream_iterator<std::string>(list_stream, delim.c_str()), f);

		list_stream << f(*c.rbegin());
	} else
	{
		list_stream << f(*c.begin());
	}

	return list_stream.str();
}

} // namespace details


/**
 * \brief Tool class to get lists of available readers.
 */
class AvailableFileReaders final
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

