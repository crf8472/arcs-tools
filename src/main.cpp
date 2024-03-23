/**
 * \file
 *
 * \brief arcstk, a toolbox for AccurateRip checksums
 */

#include <cstdlib>           // for EXIT_SUCCESS, EXIT_FAILURE
#include <iostream>          // for operator<<, endl, cout, cerr
#include <exception>         // for exception
#include <string>            // for string

#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>  // for Logging
#endif

#ifndef __ARCSTOOLS_APPLICATION_HPP__
#include "application.hpp"
#endif
#ifndef __ARCSTOOLS_APPREGISTRY_HPP__
#include "appregistry.hpp"     // for ApplicationFactory
#endif
#ifndef __ARCSTOOLS_CLITOKENS_HPP__
#include "clitokens.hpp"       // for CallSyntaxException
#endif
#ifndef __ARCSTOOLS_VERSION_HPP__
#include "version.hpp"
#endif


/*
 * Instantiates and runs the application requested from command line.
 *
 * Either the binary can be called directly with the name of the application
 * as its first parameter.
 *
 * Or the binary can be called by an OS defined alias in which case the alias
 * must end with a known application name.
 */
int main(int argc, char** argv)
{
	using arcstk::Logging;
	using arcstk::LOGLEVEL;

	// Perform default setup for the logging of all applications.

	// We do not know whether the application is required to run quiet,
	// so initial level is NOT default level but quiet level.
	// Application decides about setting the level.
	Logging::instance().set_level(LOGLEVEL::NONE);

	// TODO Make configurable
	Logging::instance().set_timestamps(false);

	using arcsapp::ARCSTOOLS_BINARY_NAME;
	using arcsapp::ARCSTOOLS_VERSION;
	using arcsapp::ApplicationFactory;
	using arcsapp::input::CallSyntaxException;

	// Was binary called by some alias?

	const auto CALL_NAME = std::string { argv[0] ? argv[0] : "" };
	const auto is_call_by_alias = bool {
		CALL_NAME.length() >= ARCSTOOLS_BINARY_NAME.length()
		and
		CALL_NAME.substr(CALL_NAME.length() - ARCSTOOLS_BINARY_NAME.length())
			!= ARCSTOOLS_BINARY_NAME
	};

	// Select application by name requested in call and run it

	if (argc > 1 or is_call_by_alias)
	{
		const auto* requested_name = argc ? argv[!is_call_by_alias] : argv[0];
		auto application { ApplicationFactory::lookup(requested_name) };

		if (application)
		{
			try
			{
				if (is_call_by_alias)
				{
					return application->run(argc, argv);
				} else
				{
					// skip first token
					return application->run(argc - 1, argv + 1);
				}
			} catch (const CallSyntaxException &cse)
			{
				std::cerr << "Syntax error: " << cse.what() << '\n';
				application->print_usage();
				return EXIT_FAILURE;

			} catch (const std::exception &e)
			{
				std::cerr << "Error: " << e.what() << '\n';
				return EXIT_FAILURE;
			}
		} else
		{
			std::cerr << "No application selected." << '\n';
		}
	}

	// No input? Print usage.

	std::cout << ARCSTOOLS_BINARY_NAME << " " << ARCSTOOLS_VERSION << '\n';
	std::cout << "Usage: " << ARCSTOOLS_BINARY_NAME << " [";
	{
		const auto apps { ApplicationFactory::registered_names() };
		auto i = apps.size();
		for (const auto& name : apps)
		{
			std::cout << name;
			if (0 <-- i) { std::cout << '|'; }
		}
	}
	std::cout << "] " << "[OPTIONS] <filenames>" << '\n';

	return EXIT_SUCCESS;
}

