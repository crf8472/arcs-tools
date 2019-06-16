/**
 * \file
 *
 * \brief id, a tool to calculate AccurateRip ids.
 */

#include <iostream>

#ifndef __ARCSTOOLS_APPARID_HPP__
#include "app-id.hpp"
#endif

/**
 * \brief Instantiates and runs an ARIdApplication with args from command line.
 *
 * \param[in] argc Number of CLI arguments
 * \param[in] argv Array of CLI arguments
 *
 * \return Result code
 */
int main(int argc, char** argv)
{
	ARIdApplication application;

	try
	{
		return application.run(argc, argv);

	} catch (const std::exception &e)
	{
		std::cerr << "ERROR: " << e.what() << std::endl;

		return EXIT_FAILURE;
	}
}

