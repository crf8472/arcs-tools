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

namespace arcsapp
{

class CLITokens;

/**
 * \brief Configuration options for ARIdApplications.
 */
struct ARIdOptions
{
	static constexpr OptionValue CDDBID    = 1;
	static constexpr OptionValue URL       = 2;
	static constexpr OptionValue DBID      = 3;
	static constexpr OptionValue PROFILE   = 4;
	static constexpr OptionValue AUDIOFILE = 5;
	static constexpr OptionValue URLPREFIX = 6;
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

	const std::vector<std::pair<Option, OptionValue>>&
		do_supported_options() const override;
};


/**
 * \brief AccurateRip Identifier Calculation Application
 */
class ARIdApplication final : public ARApplication
{
	std::string do_name() const override;

	std::string do_call_syntax() const override;

	std::unique_ptr<Configurator> create_configurator(int argc, char** argv)
		const override;

	int do_run(const Options &options) override;
};

} // namespace arcsapp

#endif

