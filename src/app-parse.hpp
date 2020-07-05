#ifndef __ARCSTOOLS_APPARPARSE_HPP__
#define __ARCSTOOLS_APPARPARSE_HPP__

/**
 * \file
 *
 * \brief Interface for ARParseApplication.
 *
 * Options, Configurator and Application for parse.
 */


#include <cstdint>          // for uint8_t
#include <memory>           // for unique_ptr
#include <string>           // for string

#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>  // for ARCS_LOG_WARNING
#endif
#ifndef __LIBARCSTK_PARSE_HPP__
#include <arcstk/parse.hpp>    // for DefaultErrorHandler, ARFileParser, ARS...
#endif

#ifndef __ARCSTOOLS_APPLICATION_HPP__
#include "application.hpp"     // for ARApplication
#endif

namespace arcsapp
{

class CLITokens;
class Configurator;
class Options;

/**
 * \brief AccurateRip Response Parsing Application.
 */
class ARParseApplication final : public ARApplication
{
	std::string do_name() const override;

	std::string do_call_syntax() const override;

	std::unique_ptr<Configurator> create_configurator() const override;

	std::unique_ptr<Configurator> create_configurator(int argc, char** argv)
		const override;

	int do_run(const Options &options) override;
};

} // namespace arcsapp

#endif

