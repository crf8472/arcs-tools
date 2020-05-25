/**
 * \file
 *
 * \brief id, a tool to calculate AccurateRip ids.
 */

#include <cstdlib>           // for EXIT_FAILURE
#include <iostream>          // for operator<<, basic_ostream, endl, cerr

#ifndef __ARCSTOOLS_APPARID_HPP__
#include "app-id.hpp"        // for ARIdApplication
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
	using arcsapp::Application;
	using Id = arcsapp::ARIdApplication;

	Application<Id> application;
	return application.run(argc, argv);
}

