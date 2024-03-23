#ifndef __ARCSTOOLS_APPPARSE_HPP__
#include "app-parse.hpp"
#endif

#include <cstdlib>             // for EXIT_SUCCESS
#include <memory>              // for make_unique, unique_ptr
#include <string>              // for string
#include <utility>             // for move, pair

#ifndef __LIBARCSTK_DBAR_HPP__
#include <arcstk/dbar.hpp>
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
#include "tools-parse.hpp"        // for PrintParseHandler, DBARTripletLayout
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


// ARParseApplication


std::string ARParseApplication::do_name() const
{
	return "parse";
}


std::string ARParseApplication::do_call_syntax() const
{
	return "[OPTIONS] <filename>";
}


std::unique_ptr<Configurator> ARParseApplication::do_create_configurator() const
{
	return std::make_unique<DefaultConfigurator>();
}


int ARParseApplication::do_run(const Configuration& config)
{
	auto printer = PrintParseHandler {};
	const auto arguments = config.arguments();

	// read from file(s)
	if (arguments && !arguments->empty())
	{
		for (const auto& file : *arguments)
		{
			arcstk::parse_file(file, &printer, nullptr);
		}
	}
	else // read from stdin
	{
		// TODO Actual amount of bytes should come from config
		read_from_stdin(1024, &printer, nullptr);
	}

	return EXIT_SUCCESS;
}

} // namespace arcsapp

