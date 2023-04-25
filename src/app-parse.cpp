#ifndef __ARCSTOOLS_APPPARSE_HPP__
#include "app-parse.hpp"
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
#include "appregistry.hpp"         // for RegisterApplicationType
#endif
#ifndef __ARCSTOOLS_CONFIG_HPP__
#include "config.hpp"              // for DefaultConfigurator
#endif
#ifndef __ARCSTOOLS_RESULT_HPP__
#include "result.hpp"              // for Result
#endif
#ifndef __ARCSTOOLS_TOOLS_PARSE_HPP__
#include "tools-parse.hpp"        // for ARParserContentPrintHandler,
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

using arcstk::DefaultErrorHandler;


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


bool ARParseApplication::calculation_requested(const Options &options) const
{
	return false;
}


auto ARParseApplication::run_calculation(const Options &options) const
	-> std::pair<int, std::unique_ptr<Result>>
{
	return { 0, nullptr }; // never called
}


int ARParseApplication::do_run(const Options &options)
{
	auto content_handler = std::make_unique<ARParserContentPrintHandler>(
		options.value(OPTION::OUTFILE));

	const auto arguments = options.arguments();

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

