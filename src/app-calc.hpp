#ifndef __ARCSTOOLS_APPCALC_HPP__
#define __ARCSTOOLS_APPCALC_HPP__

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

#ifndef __LIBARCSDEC_SELECTION_HPP__
#include <arcsdec/selection.hpp>  // for FileReaderSelection
#endif

#ifndef __ARCSTOOLS_APPLICATION_HPP__
#include "application.hpp"
#endif
#ifndef __ARCSTOOLS_CONFIG_HPP__
#include "config.hpp"
#endif
#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#include "layouts.hpp"           // for CalcResultLayout, CalcTableResultLayout
#endif


namespace arcsapp
{

class CLITokens;


/**
 * \brief Options to configure Application instances that do ARCS calculation.
 */
struct CALCBASE : public FORMATBASE
{
private:

	static constexpr auto& BASE = FORMATBASE::SUBCLASS_BASE;

public:

	// Calculation Input Options

	static constexpr OptionCode METAFILE      = BASE +  0; // 10

	// Calculation Output Options

	static constexpr OptionCode NOTRACKS      = BASE +  1;
	static constexpr OptionCode NOFILENAMES   = BASE +  2;
	static constexpr OptionCode NOOFFSETS     = BASE +  3;
	static constexpr OptionCode NOLENGTHS     = BASE +  4;
	static constexpr OptionCode NOLABELS      = BASE +  5;
	static constexpr OptionCode COLDELIM      = BASE +  6;
	static constexpr OptionCode PRINTID       = BASE +  7;
	static constexpr OptionCode PRINTURL      = BASE +  8; // 18

protected:

	/**
	 * \brief Max constant occurring in CALC
	 */
	static constexpr OptionCode SUBCLASS_BASE = BASE +  9;
};


/**
 * \brief Base clase for Application instances that support CALCBASE options.
 */
class ARCalcConfiguratorBase : public Configurator
{
public:

	using Configurator::Configurator;

	/**
	 * \brief Service: returns TRUE iff some real calculation is requested.
	 *
	 * A calculation is requested, iff either --help or --version are absent
	 * or audiofiles are present or --metafile is present.
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
	static constexpr auto& BASE = CALCBASE::SUBCLASS_BASE;

public:

	// Calculation Input Options

	static constexpr OptionCode FIRST        = BASE + 0;  // 19
	static constexpr OptionCode LAST         = BASE + 1;
	static constexpr OptionCode ALBUM        = BASE + 2;

	// Calculation Output Options

	static constexpr OptionCode NOV1         = BASE + 3;
	static constexpr OptionCode NOV2         = BASE + 4;
	static constexpr OptionCode SUMSONLY     = BASE + 5;
	static constexpr OptionCode TRACKSASCOLS = BASE + 6; // 25
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

	const std::vector<std::pair<Option, OptionCode>>& do_supported_options()
		const override;

	std::unique_ptr<Options> do_configure_options(
			std::unique_ptr<Options> options) override;
};


/**
 * \brief Application to calculate AccurateRip checksums.
 */
class ARCalcApplication final : public Application
{
public:

	/**
	 * \brief Do calculation based on the options passed.
	 *
	 * \param[in] metafilename    Filename of the TOC file
	 * \param[in] audiofilenames  Filenames of the audio files
	 * \param[in] as_first        Flag to indicate album first track
	 * \param[in] as_last         Flag to indicate album last track
	 * \param[in] types           The checksum types requested
	 * \param[in] audio_selection The selection for audio readers
	 * \param[in] toc_selection   The selection for TOC parsers
	 *
	 * \return Calculation result
	 */
	static std::tuple<Checksums, ARId, std::unique_ptr<TOC>> calculate(
		const std::string &metafilename,
		const std::vector<std::string> &audiofilenames,
		const bool as_first,
		const bool as_last,
		const std::set<arcstk::checksum::type> &types,
		arcsdec::FileReaderSelection *audio_selection,
		arcsdec::FileReaderSelection *toc_selection);

private:

	/**
	 * \brief Create the printing format according to the options
	 *
	 * The format object returned will not yet have formatted the checksums.
	 *
	 * \param[in] options   The options parsed from command line
	 * \param[in] checksums The checksums to format
	 */
	std::unique_ptr<CalcResultLayout> configure_layout(const Options &options)
		const;

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

	std::unique_ptr<Configurator> create_configurator() const override;

	int do_run(const Options &options) override;
};

} // namespace arcsapp

#endif

