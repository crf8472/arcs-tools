#ifndef __ARCSTOOLS_APPARCALC_HPP__
#define __ARCSTOOLS_APPARCALC_HPP__

/**
 * \file
 *
 * \brief Interface for ARCSApplication.
 *
 * Options, Configurator and Application for calc application.
 */

#include <cstdint>                // for uint8_t, uint16_t
#include <memory>                 // for unique_ptr
#include <set>                    // for set
#include <string>                 // for string
#include <tuple>                  // for tuple

#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>   // for Checksums, ARId, TOC
#endif

#ifndef __ARCSTOOLS_APPLICATION_HPP__
#include "application.hpp"
#endif
#ifndef __ARCSTOOLS_CONFIG_HPP__
#include "config.hpp"
#endif
#ifndef __ARCSTOOLS_OPTIONS_HPP__
#include "options.hpp"            // for Options, __ARCSTOOLS_OPTIONS_HPP__
#endif
#ifndef __ARCSTOOLS_PRINTERS_HPP__
#include "printers.hpp"
#endif

namespace arcsapp
{

class CLITokens;


/**
 * \brief Options to configure ARApplication instances that do ARCS calculation.
 */
struct CALCBASE
{
	// No-calculation output (info) options

	static constexpr OptionValue LIST_TOC_FORMATS   = 1;
	static constexpr OptionValue LIST_AUDIO_FORMATS = 2;

	// Calculation input options

	static constexpr OptionValue FIRST        = 3;
	static constexpr OptionValue LAST         = 4;
	static constexpr OptionValue ALBUM        = 5;
	static constexpr OptionValue METAFILE     = 6;
	static constexpr OptionValue METAFILEPATH = 7; // has no cli token

	// Calculation output options

	static constexpr OptionValue NOTRACKS     =  8;
	static constexpr OptionValue NOFILENAMES  =  9;
	static constexpr OptionValue NOOFFSETS    = 10;
	static constexpr OptionValue NOLENGTHS    = 11;
	static constexpr OptionValue NOLABELS     = 12;
	static constexpr OptionValue COLDELIM     = 13;
	static constexpr OptionValue PRINTID      = 14;
	static constexpr OptionValue PRINTURL     = 15;

protected:

	/**
	 * \brief Max constant occurring in CALC
	 */
	static constexpr OptionValue MAX_CONSTANT = 15;
};


/**
 * \brief Base clase for ARApplication instances that support CALCBASE options.
 */
class ARCalcConfiguratorBase : public Configurator
{
public:

	using Configurator::Configurator;

protected:

	/**
	 * \brief Worker: returns TRUE iff some real calculation is requested.
	 *
	 * This is used to determine whether the optional info options are to be
	 * ignored in the presence of a calculation request.
	 *
	 * \param[in] options The options to configure
	 *
	 * \return TRUE iff a real calculation is requested.
	 */
	bool calculation_requested(const Options &options) const;

	/**
	 * \brief Worker: implement configuration of CALCBASE options for reuse in
	 * subclasses.
	 *
	 * \param[in] options The options to configure
	 *
	 * \return Configured options
	 */
	std::unique_ptr<Options> configure_calcbase_options(
			std::unique_ptr<Options> options) const;
};


/**
 * \brief Options exclusive to ARCalcApplication
 */
class CALC : public CALCBASE
{
	static constexpr auto& START = CALCBASE::MAX_CONSTANT;

public:

	static constexpr OptionValue NOV1         = START + 1;
	static constexpr OptionValue NOV2         = START + 2;
	static constexpr OptionValue SUMSONLY     = START + 3;
	static constexpr OptionValue TRACKSASCOLS = START + 4;
};


/**
 * \brief Configurator for ARCalcApplication instances.
 */
class ARCalcConfigurator final : public ARCalcConfiguratorBase
{
public:

	using ARCalcConfiguratorBase::ARCalcConfiguratorBase;

private:

	const std::vector<std::pair<Option, OptionValue>>& do_supported_options()
		const override;

	int do_parse_arguments(CLITokens& cli, Options &options) const override;

	std::unique_ptr<Options> do_configure_options(
			std::unique_ptr<Options> options) override;
};


/**
 * \brief Checksum Calculation Application
 */
class ARCalcApplication final : public ARApplication
{
public:

	/**
	 * \brief Do calculation based on the options passed.
	 *
	 * Implementation can use any of the \c options, but should not alter
	 * the set of requested types.
	 *
	 * \param[in] options The options to use
	 * \param[in] types   The checksum types requested
	 *
	 * \return Calculation result
	 */
	static std::tuple<Checksums, ARId, std::unique_ptr<TOC>> calculate(
			const Options &options,
			const std::set<arcstk::checksum::type> &types);

private:

	std::string do_name() const override;

	std::string do_call_syntax() const override;

	std::unique_ptr<Configurator> create_configurator(int argc, char** argv)
		const override;

	int do_run(const Options &options) override;

	/**
	 * \brief Create the printing format according to the options
	 *
	 * The format object returned will not yet have formatted the checksums.
	 *
	 * \param[in] options   The options parsed from command line
	 * \param[in] checksums The checksums to format
	 */
	std::unique_ptr<ChecksumsResultPrinter> configure_format(
			const Options &options) const;

	/**
	 * \brief Determine the requested checksum types for calculation.
	 *
	 * \param[in] options   The options parsed from command line
	 *
	 * \return Checksum types to calculate.
	 */
	std::set<arcstk::checksum::type> requested_types(const Options &options)
		const;

	/**
	 * \brief Worker method for run(): handles info requests.
	 *
	 * \param[in] options The options to run the application
	 *
	 * \return Application return code
	 */
	int run_info(const Options &options);

	/**
	 * \brief Worker method for run(): handles calculation requests.
	 *
	 * \param[in] options The options to run the application
	 *
	 * \return Application return code
	 */
	int run_calculation(const Options &options);
};

} // namespace arcsapp

#endif

