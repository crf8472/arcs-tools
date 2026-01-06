/**
 * \internal
 *
 * \file
 *
 * \brief Implements symbols from tools-input.hpp.
 */

#ifndef __ARCSTOOLS_TOOLS_INPUT_HPP__
#include "tools-input.hpp"
#endif

#include <any>           // for any
#include <string>        // for string

#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>     // for ARCS_LOG,...
#endif

namespace arcsapp
{
inline namespace v_1_0_0
{


// StringParser


std::any StringParser::parse(const std::string& s) const
{
	ARCS_LOG(DEBUG1) << "=> " << start_message();
	return this->do_parse(s);
}

} // namespace v_1_0_0
} // namespace arcsapp

