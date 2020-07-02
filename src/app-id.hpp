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
	static constexpr OptionCode CDDBID    = 1;
	static constexpr OptionCode URL       = 2;
	static constexpr OptionCode DBID      = 3;
	static constexpr OptionCode PROFILE   = 4;
	static constexpr OptionCode AUDIOFILE = 5;
	static constexpr OptionCode URLPREFIX = 6;
	static constexpr OptionCode ID        = 7;
};


/**
 * \brief Configurator for ARIdApplication instances.
 */
class ARIdConfigurator final : public Configurator
{
public:

	using Configurator::Configurator;

private:

	const std::vector<std::pair<Option, OptionCode>>&
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

