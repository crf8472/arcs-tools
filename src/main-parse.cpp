/**
 * \file
 *
 * \brief parse, a tool for parsing AccurateRip responses.
 */

#include <iostream>

#ifndef __ARCSTOOLS_APPARPARSE_HPP__
#include "app-parse.hpp"
#endif

/**
 * \brief Instantiates and runs an ARParseApplication with args from command
 * line.
 *
 * \param[in] argc Number of CLI arguments
 * \param[in] argv Array of CLI arguments
 *
 * \return Result code
 */
int main(int argc, char** argv)
{
	ARParseApplication application;

	try
	{
		return application.run(argc, argv);

	} catch (const std::exception &e)
	{
		std::cerr << "ERROR: " << e.what() << std::endl;

		return EXIT_FAILURE;
	}
}

