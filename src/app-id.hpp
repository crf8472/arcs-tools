#ifndef __ARCSTOOLS_APPARID_HPP__
#define __ARCSTOOLS_APPARID_HPP__

/**
 * \file
 *
 * \brief Interface for ARIdApplication.
 *
 * Options, Configurator and Application for id application.
 */

#include <cstdint>          // for uint8_t
#include <memory>           // for unique_ptr
#include <string>           // for string

#ifndef __ARCSTOOLS_APPLICATION_HPP__
#include "application.hpp"  // for ARApplication
#endif
#ifndef __ARCSTOOLS_CONFIG_HPP__
#include "config.hpp"       // for Configurator
#endif
#ifndef __ARCSTOOLS_OPTIONS_HPP__
#include "options.hpp"      // for Options, __ARCSTOOLS_OPTIONS_HPP__
#endif

class CLIParser;

/**
 * \brief Configuration options for ARIdApplications.
 */
class ARIdOptions : public Options
{

public:

	/**
	 * \brief Requested cddb id
	 */
	static constexpr uint8_t CDDBID = 1;

	/**
	 * \brief Requested url
	 */
	static constexpr uint8_t URL = 2;

	/**
	 * \brief Requested ar db id
	 */
	static constexpr uint8_t DBID = 4;

	/**
	 * \brief Requested entire disc profile
	 */
	static constexpr uint8_t PROFILE = 8;

	/**
	 * \brief Specified AUDIOFILE file
	 */
	static constexpr uint8_t AUDIOFILE = 16;

	/**
	 * \brief Specified alternative URL prefix
	 */
	static constexpr uint8_t PRE = 32;

	/**
	 * \brief Name of optional outfile
	 */
	static constexpr uint8_t OUT = 64;

	/**
	 * \brief Default constructor.
	 */
	ARIdOptions();
};


/**
 * \brief Configurator for ARIdApplication instances.
 */
class ARIdConfigurator final : public Configurator
{

public:

	/**
	 * \brief Constructor.
	 *
	 * \param[in] argc Number of command line arguments
	 * \param[in] argv Command line arguments
	 */
	ARIdConfigurator(int argc, char** argv);


private:

	std::unique_ptr<Options> parse_options(CLIParser& cli) override;

	std::unique_ptr<Options> do_configure_options(
			std::unique_ptr<Options> options) override;
};


/**
 * \brief AccurateRip Identifier Calculation Application
 */
class ARIdApplication final : public ARApplication
{
	std::unique_ptr<Configurator> create_configurator(int argc, char** argv)
		const override;

	std::string do_name() const override;

	int do_run(const Options &options) override;

	void do_print_usage() const override;
};

#endif

