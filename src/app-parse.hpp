#ifndef __ARCSTOOLS_APPARPARSE_HPP__
#define __ARCSTOOLS_APPARPARSE_HPP__

/**
 * \file
 *
 * \brief Interface for ARParseApplication.
 *
 * Options, Configurator and Application for parse.
 */

#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>
#endif

#ifndef __ARCSTOOLS_APPLICATION_HPP__
#include "application.hpp"
#endif
#ifndef __ARCSTOOLS_OUTPUTFORMATS_HPP__
#include "outputformats.hpp"
#endif
#ifndef __ARCSTOOLS_CONFIG_HPP__
#include "config.hpp"
#endif
#ifndef __ARCSTOOLS_OPTIONS_HPP__
#include "options.hpp"
#endif


/**
 * \brief Configuration options for ARParseApplications.
 */
class ARParseOptions : public Options
{

public:

	/**
	 * \brief Output to file was specified.
	 */
	static constexpr uint8_t OUTFILE = 1;

	/**
	 * \brief Constructor.
	 */
	ARParseOptions();
};


/**
 * \brief Configurator for ARParseApplication instances.
 */
class ARParseConfigurator final : public Configurator
{

public:

	/**
	 * \brief Constructor.
	 *
	 * \param[in] argc Number of command line arguments
	 * \param[in] argv Command line arguments
	 */
	ARParseConfigurator(int argc, char** argv);


private:

	std::unique_ptr<Options> parse_options(CLIParser& cli) override;

	std::unique_ptr<Options> do_configure_options(
			std::unique_ptr<Options> options) override;
};


/**
 * \brief AccurateRip Response Parsing Application.
 */
class ARParseApplication final : public ARApplication
{
	std::unique_ptr<Configurator> create_configurator(int argc, char** argv)
		const override;

	std::string do_name() const override;

	int do_run(const Options &options) override;

	void do_print_usage() const override;
};

#endif

