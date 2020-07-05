/**
 * \file
 *
 * \brief arcstk, a toolbox for AccurateRip checksums
 */

#include <algorithm>         // for find_if
#include <cstdlib>           // for EXIT_SUCCESS, EXIT_FAILURE
#include <iostream>          // for operator<<, basic_ostream, endl, cerr
#include <memory>            // for unique_ptr
#include <exception>         // for exception
#include <utility>           // for pair
#include <string>            // for string

#ifndef __ARCSTOOLS_APPLICATION_HPP__
#include "application.hpp"
#endif
#ifndef __ARCSTOOLS_APPREGISTRY_HPP__
#include "appregistry.hpp"
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
	using arcsapp::ARCSTOOLS_BINARY_NAME;
	using arcsapp::ARCSTOOLS_VERSION_INFO_STRING;
	using arcsapp::ApplicationFactory;
	using arcsapp::CallSyntaxException;

	const auto& BINARY = std::string { ARCSTOOLS_BINARY_NAME };
	const auto  ARGV_0 = std::string { argv[0] };

	// Was binary called by some alias?
	const bool is_call_by_alias =
		ARGV_0.length() >= BINARY.length()
		and
		ARGV_0.substr(ARGV_0.length() - BINARY.length()) != BINARY;

	if (argc > 1 or is_call_by_alias)
	{
		const auto& requested_name = argv[!is_call_by_alias];

		auto application = ApplicationFactory::lookup(requested_name);

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
			} catch (const arcsapp::CallSyntaxException &cse)
			{
				std::cerr << "ERROR: " << cse.what() << std::endl;

				application->print_usage();

				return EXIT_FAILURE;

			} catch (const std::exception &e)
			{
				std::cerr << "ERROR: " << e.what() << std::endl;

				return EXIT_FAILURE;
			}
		} else
		{
			std::cout << "No application selected." << std::endl;
		}
	}

	// No input? Print usage.

	std::cout << ARCSTOOLS_BINARY_NAME << " "
		<< ARCSTOOLS_VERSION_INFO_STRING
		<< std::endl;

	std::cout << "Usage: " << ARCSTOOLS_BINARY_NAME
		<< " [id|calc|parse|verify] " // TODO Load names from Registry
		<< "[OPTIONS] <filenames>"
		<< std::endl;

	return EXIT_SUCCESS;
}

