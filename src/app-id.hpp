#ifndef __ARCSTOOLS_APPID_HPP__
#define __ARCSTOOLS_APPID_HPP__

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
#include "application.hpp"  // for Application
#endif
#ifndef __ARCSTOOLS_CONFIG_HPP__
#include "config.hpp"       // for Configurator
#endif
#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#include "layouts.hpp"      // for ARIdLayout, ...
#endif

namespace arcsapp
{

class CLITokens;

/**
 * \brief Configuration options for ARIdApplications.
 */
struct ARIdOptions
{
	static constexpr OptionCode BASE = Configurator::BASE_CODE();

	static constexpr OptionCode CDDBID    = BASE +  1; // 7
	static constexpr OptionCode URL       = BASE +  2;
	static constexpr OptionCode DBID      = BASE +  3;
	static constexpr OptionCode PROFILE   = BASE +  4;
	static constexpr OptionCode URLPREFIX = BASE +  5;
	static constexpr OptionCode ID        = BASE +  6;
	static constexpr OptionCode AUDIOFILE = BASE +  7; // ...
	static constexpr OptionCode NOLABELS  = BASE +  8; // 14
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
class ARIdApplication final : public Application
{
	std::string do_name() const override;

	std::string do_call_syntax() const override;

	std::unique_ptr<Configurator> create_configurator() const override;

	int do_run(const Options &options) override;
};

} // namespace arcsapp

#endif

