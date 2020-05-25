/**
 * \file
 *
 * \brief parse, a tool for parsing AccurateRip responses.
 */

#include <cstdlib>           // for EXIT_FAILURE
#include <iostream>          // for operator<<, basic_ostream, endl, cerr

#ifndef __ARCSTOOLS_APPARPARSE_HPP__
#include "app-parse.hpp"     // for ARParseApplication
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
	using arcsapp::Application;
	using Parse = arcsapp::ARParseApplication;

	Application<Parse> application;
	return application.run(argc, argv);
}

