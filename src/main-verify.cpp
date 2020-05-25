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
	using arcsapp::Application;
	using Verify = arcsapp::ARVerifyApplication ;

	Application<Verify> application;
	return application.run(argc, argv);
}

