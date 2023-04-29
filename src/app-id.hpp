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


namespace arcsapp
{

class Result;

/**
 * \brief Configuration options for ARIdApplications.
 */
class ARIdOptions : public FORMATBASE
{
	static constexpr auto& BASE = FORMATBASE::SUBCLASS_BASE;

public:

	static constexpr OptionCode CDDBID    = BASE +  0; // 10
	static constexpr OptionCode URL       = BASE +  1;
	static constexpr OptionCode DBID      = BASE +  2;
	static constexpr OptionCode PROFILE   = BASE +  3;
	static constexpr OptionCode URLPREFIX = BASE +  4;
	static constexpr OptionCode ID        = BASE +  5;
	static constexpr OptionCode AUDIOFILE = BASE +  6;
	static constexpr OptionCode NOLABELS  = BASE +  7; // 17
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
 * \brief Application to calculate AccurateRip Identifiers.
 */
class ARIdApplication final : public Application
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

