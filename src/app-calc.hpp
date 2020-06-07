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
	// Info Output Options (no calculation)

	static constexpr OptionValue LIST_TOC_FORMATS   = 1;
	static constexpr OptionValue LIST_AUDIO_FORMATS = 2;

	// Calculation Input Options

	static constexpr OptionValue METAFILE     = 6;
	static constexpr OptionValue METAFILEPATH = 7; // has no cli token

	// Calculation Output Options

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

	/**
	 * \brief Worker: returns TRUE iff some real calculation is requested.
	 *
	 * This is used to determine whether the optional info options are to be
	 * ignored in the presence of a calculation request.
	 *
	 * \param[in] options The input options (may or may not be configured)
	 *
	 * \return TRUE iff a real calculation is requested.
	 */
	static bool calculation_requested(const Options &options);

protected:

	/**
	 * \brief Worker: implement configuration of CALCBASE options for reuse in
	 * subclasses.
	 *
	 * Calls calculation_requested() on unconfigured input options.
	 *
	 * \param[in] options The options to configure
	 *
	 * \return Configured options
	 */
	std::unique_ptr<Options> configure_calcbase_options(
			std::unique_ptr<Options> options) const;
};


/**
 * \brief Options exclusive to ARCalcApplication.
 *
 * Access options for verify exclusively by this class, not by CALCBASE.
 */
class CALC : public CALCBASE
{
	static constexpr auto& BASE = CALCBASE::MAX_CONSTANT;

public:

	// Calculation Input Options

	static constexpr OptionValue FIRST        = BASE + 1;
	static constexpr OptionValue LAST         = BASE + 2;
	static constexpr OptionValue ALBUM        = BASE + 3;

	// Calculation Output Options

	static constexpr OptionValue NOV1         = BASE + 4;
	static constexpr OptionValue NOV2         = BASE + 5;
	static constexpr OptionValue SUMSONLY     = BASE + 6;
	static constexpr OptionValue TRACKSASCOLS = BASE + 7;
};


/**
 * \brief Configurator for ARCalcApplication instances.
 *
 * Respects all CALC options.
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
 * \brief Application to calculate AccurateRip checksums.
 */
class ARCalcApplication final : public ARApplication
{
public:

	/**
	 * \brief Do calculation based on the options passed.
	 *
	 * \param[in] metafilename   Filename of the TOC file
	 * \param[in] audiofilenames Filenames of the audio files
	 * \param[in] as_first       Flag to indicate album first track
	 * \param[in] as_last        Flag to indicate album last track
	 * \param[in] types          The checksum types requested
	 *
	 * \return Calculation result
	 */
	static std::tuple<Checksums, ARId, std::unique_ptr<TOC>> calculate(
		const std::string &metafilename,
		const std::vector<std::string> &audiofilenames,
		const bool as_first,
		const bool as_last,
		const std::set<arcstk::checksum::type> &types);

private:

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
	 * \brief Worker: Determine the requested checksum types for calculation.
	 *
	 * \param[in] options The options parsed from command line
	 *
	 * \return Checksum types to calculate.
	 */
	std::set<arcstk::checksum::type> requested_types(const Options &options)
		const;

	/**
	 * \brief Worker for run(): handles calculation requests.
	 *
	 * \param[in] options The options to run the application
	 *
	 * \return Application return code
	 */
	int run_calculation(const Options &options);


	std::string do_name() const override;

	std::string do_call_syntax() const override;

	std::unique_ptr<Configurator> create_configurator(int argc, char** argv)
		const override;

	int do_run(const Options &options) override;
};

} // namespace arcsapp

#endif

