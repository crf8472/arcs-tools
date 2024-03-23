#ifndef __ARCSTOOLS_APPID_HPP__
#define __ARCSTOOLS_APPID_HPP__

/**
 * \file
 *
 * \brief Interface for ARIdApplication.
 *
 * Options, Configurator and Application for id application.
 */

#include <memory>           // for unique_ptr
#include <string>           // for string
#include <utility>          // for pair

#ifndef __ARCSTOOLS_APPLICATION_HPP__
#include "application.hpp"  // for Application
#endif
#ifndef __ARCSTOOLS_CONFIG_HPP__
#include "config.hpp"       // for Configurator
#endif
#ifndef __ARCSTOOLS_APPCALC_HPP__
#include "app-calc.hpp"     // for ARCalcApplicationBase
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
	static constexpr OptionCode FILENAME  = BASE +  3;
	static constexpr OptionCode PROFILE   = BASE +  4;
	static constexpr OptionCode URLPREFIX = BASE +  5;
	static constexpr OptionCode ID        = BASE +  6;
	static constexpr OptionCode AUDIOFILE = BASE +  7;
	static constexpr OptionCode NOLABELS  = BASE +  8; // 18
};


/**
 * \brief Configurator for ARIdApplication instances.
 */
class ARIdConfigurator final : public Configurator
{
public:

	using Configurator::Configurator;

private:

	void do_flush_local_options(OptionRegistry& r) const final;

	std::unique_ptr<Options> do_configure_options(
			std::unique_ptr<Options> options) const final;

	// void do_validate(const Options& options) const;

	// OptionParsers do_parser_list() const;

	// void do_validate(const Configuration& configuration) const;
};


/**
 * \brief Application to calculate AccurateRip Identifiers.
 */
class ARIdApplication final : public ARCalcApplicationBase
{
	// ARCalcApplicationBase

	bool do_calculation_requested(const Configuration &config) const final;

	std::pair<int, std::unique_ptr<Result>> do_run_calculation(
			const Configuration& config) const final;

	// Application

	std::string do_name() const final;

	std::string do_call_syntax() const final;

	std::unique_ptr<Configurator> do_create_configurator() const final;
};

} // namespace arcsapp

#endif

