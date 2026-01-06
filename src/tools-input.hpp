#ifndef __ARCSTOOLS_TOOLS_INPUT_HPP__
#define __ARCSTOOLS_TOOLS_INPUT_HPP__

/**
 * \file
 *
 * \brief Tools for input management on cli and files.
 */

#include <any>           // for any
#include <string>        // for string

#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>     // for ARCS_LOG,...
#endif

namespace arcsapp
{
inline namespace v_1_0_0
{


/**
 * \brief Abstract base class for string parsers.
 *
 * The result class is a std::any that can be put into a Configuration.
 */
class StringParser
{
	virtual std::string start_message() const
	= 0;

	virtual std::any do_parse(const std::string& s) const
	= 0;

public:

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~StringParser() = default;

	/**
	 * \brief Parse input string to object.
	 *
	 * \param[in] s Input string to be parsed
	 *
	 * \return Result object
	 */
	std::any parse(const std::string& s) const;
};


/**
 * \brief Abstract base class for for option value string parsers.
 *
 * \tparam T Result type
 */
template <typename T>
class InputStringParser : public StringParser
{
	/**
	 * \brief Parsing result for empty input.
	 *
	 * Default implementation returns an instance created by default
	 * constructor.
	 *
	 * \return Result instance for an empty input string
	 */
	virtual auto do_parse_empty() const -> T
	{
		ARCS_LOG(DEBUG1) << "Empty parser input, return default object";
		return T { /* empty */ }; // TODO Use declval?
	}

	/**
	 * \brief Parsing result for non-empty input.
	 *
	 * \return Result instance for a non-empty input string
	 *
	 * \throws std::runtime_error If parsing fails
	 */
	virtual auto do_parse_nonempty(const std::string& s) const -> T
	= 0;

	// StringParser

	std::any do_parse(const std::string& s) const final
	{
		if (s.empty())
		{
			return this->do_parse_empty();
		}

		return this->do_parse_nonempty(s);
	}
};

} // namespace v_1_0_0
} // namespace arcsapp

#endif

