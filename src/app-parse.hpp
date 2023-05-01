#ifndef __ARCSTOOLS_APPPARSE_HPP__
#define __ARCSTOOLS_APPPARSE_HPP__

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
#include <utility>          // for pair

#ifndef __ARCSTOOLS_APPLICATION_HPP__
#include "application.hpp"     // for Application
#endif

namespace arcsapp
{

class Application;
class Configurator;
class Options;
class Result;

/**
 * \brief Application to parse AccurateRip responses.
 */
class ARParseApplication final : public Application
{
	std::string do_name() const final;

	std::string do_call_syntax() const final;

	std::unique_ptr<Configurator> create_configurator() const final;

	bool calculation_requested(const Options &options) const final;

	std::pair<int, std::unique_ptr<Result>> run_calculation(
			const Options &options) const final;

	int do_run(const Options &options) final;
};

} // namespace arcsapp

#endif

