#ifndef __ARCSTOOLS_APPARPARSE_HPP__
#include "app-parse.hpp"
#endif

#ifdef _WIN32 // XXX This is completely untested

#include <io.h>     // for stdin
#include <fcntl.h>  // for _setmode, 0_BINARY

#endif

#include <cstdlib>             // for EXIT_SUCCESS
#include <iostream>            // for operator<<, cout, ostream, basic_ostream
#include <memory>              // for make_unique, unique_ptr, allocator
#include <string>              // for string, operator<<, char_traits
#include <utility>             // for move
#include <vector>              // for vector

#ifndef __LIBARCSTK_PARSE_HPP__
#include <arcstk/parse.hpp>
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#ifndef __ARCSTOOLS_APPREGISTRY_HPP__
#include "appregistry.hpp"
#endif
#ifndef __ARCSTOOLS_CONFIG_HPP__
#include "config.hpp"
#endif
#ifndef __ARCSTOOLS_TOOLS_PARSE_HPP__
#include "tools-parse.hpp"
#endif
#ifndef __ARCSTOOLS_TOOLS_FS_HPP__
#include "tools-fs.hpp"
#endif

namespace arcsapp
{

namespace registered
{
const auto parse = RegisterApplicationType<ARParseApplication>("parse");
}

class Options;

using arcstk::ARFileParser;
using arcstk::ARStdinParser;
using arcstk::DefaultErrorHandler;


namespace {

/**
 * \brief Wrap a vector in an istream.
 */
template <typename CharT, typename TraitsT = std::char_traits<CharT>>
class istreamwrapper : public std::basic_streambuf<CharT, TraitsT>
{
public:

	/**
	 * Constructor
	 *
	 * \param[in] vec The vector to wrap
	 */
	explicit istreamwrapper(std::vector<CharT> &vec)
	{
		this->setg(vec.data(), vec.data(), vec.data() + vec.size());
	}
};

} // namespace


// ARParseApplication


std::string ARParseApplication::do_name() const
{
	return "parse";
}


std::string ARParseApplication::do_call_syntax() const
{
	return "[OPTIONS] <filename>";
}


std::unique_ptr<Configurator> ARParseApplication::create_configurator() const
{
	return std::make_unique<DefaultConfigurator>();
}


int ARParseApplication::do_run(const Options &options)
{
	auto content_handler = std::make_unique<ARParserContentPrintHandler>(
		options.get(OPTION::OUTFILE));

	auto arguments = options.get_arguments();

	if (not arguments.empty()) // read from file(s)
	{
		ARFileParser parser;
		parser.set_content_handler(std::move(content_handler));
		parser.set_error_handler(std::make_unique<DefaultErrorHandler>());

		for (const auto& file : arguments)
		{
			parser.set_file(file);
			parser.parse();
		}
	}
	else // read from stdin
	{
		ARStdinParser parser;
		parser.set_content_handler(std::move(content_handler));
		parser.set_error_handler(std::make_unique<DefaultErrorHandler>());

		parser.parse();
	}

	return EXIT_SUCCESS;
}

} // namespace arcsapp

