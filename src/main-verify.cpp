/**
 * \file
 *
 * \brief verify, a tool to verify AccurateRip checksums.
 */

#include <cstdlib>           // for EXIT_FAILURE
#include <iostream>          // for operator<<, basic_ostream, endl, cerr

#ifndef __ARCSTOOLS_APPARVERIFY_HPP__
#include "app-verify.hpp"    // for ARVerifyApplication
#endif

/**
 * \brief Instantiates and runs an ARVerifyApplication with args from command
 * line.
 *
 * \param[in] argc Number of CLI arguments
 * \param[in] argv Array of CLI arguments
 *
 * \return Result code
 */
int main(int argc, char** argv)
{
	arcsapp::ARVerifyApplication application;

	try
	{
		return application.run(argc, argv);

	} catch (const std::exception &e)
	{
		std::cerr << "ERROR: " << e.what() << std::endl;

		return EXIT_FAILURE;
	}
}

