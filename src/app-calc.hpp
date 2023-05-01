#ifndef __ARCSTOOLS_APPCALC_HPP__
#define __ARCSTOOLS_APPCALC_HPP__

/**
 * \file
 *
 * \brief Interface for ARCSApplication.
 *
 * Options, Configurator and Application for calc application.
 */

#include <cstdint>     // for uint8_t, uint16_t
#include <memory>      // for unique_ptr
#include <string>      // for string
#include <tuple>       // for tuple
#include <utility>     // for pair
#include <vector>      // for vector

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
#include "layouts.hpp"             // for Layout, ResultFormatter
#endif


namespace arcsapp
{

class Result;

/**
 * \brief Options to configure Application instances that do ARCS calculation.
 */
struct CALCBASE : public FORMATBASE
{
private:

	static constexpr auto& BASE = FORMATBASE::SUBCLASS_BASE;

public:

	// Calculation Input Options

	static constexpr OptionCode METAFILE      = BASE +  0; // 11

	// Calculation Output Options

	static constexpr OptionCode NOTRACKS      = BASE +  1;
	static constexpr OptionCode NOFILENAMES   = BASE +  2;
	static constexpr OptionCode NOOFFSETS     = BASE +  3;
	static constexpr OptionCode NOLENGTHS     = BASE +  4;
	static constexpr OptionCode NOLABELS      = BASE +  5;
	static constexpr OptionCode COLDELIM      = BASE +  6;
	static constexpr OptionCode PRINTID       = BASE +  7;
	static constexpr OptionCode PRINTURL      = BASE +  8; // 19

protected:

	/**
	 * \brief Max constant occurring in CALC
	 */
	static constexpr OptionCode SUBCLASS_BASE = BASE +  9;
};


/**
 * \brief Base class for Application instances that support CALCBASE options.
 */
class ARCalcConfiguratorBase : public Configurator
{
public:

	using Configurator::Configurator;

	/**
	 * \brief TRUE iff a calculation request is implicit in the Options,
	 * otherwise FALSE.
	 *
	 * \param[in] options The Options to run the application
	 *
	 * \return TRUE iff a calculation is requested, otherwise FALSE
	 */
	virtual bool calculation_requested(const Options &options) const;

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

	static constexpr OptionCode FIRST        = BASE + 0; // 20
	static constexpr OptionCode LAST         = BASE + 1;
	static constexpr OptionCode ALBUM        = BASE + 2;

	// Calculation Output Options

	static constexpr OptionCode NOV1         = BASE + 3;
	static constexpr OptionCode NOV2         = BASE + 4;
	static constexpr OptionCode SUMSONLY     = BASE + 5;
	static constexpr OptionCode TRACKSASCOLS = BASE + 6; // 26
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

	void flush_local_options(OptionRegistry& r) const final;

	std::unique_ptr<Options> do_configure_options(
			std::unique_ptr<Options> options) const final;
};


/**
 * \brief Interface to format result objects of a calculation process.
 */
using C5L = Layout<std::unique_ptr<Result>, Checksums, std::vector<std::string>,
			const TOC*, ARId, std::string>;


/**
 * \brief Format the results of the ARCalcApplication.
 */
class CalcResultFormatter final : public ResultFormatter
								, public C5L
{
private:

	virtual std::vector<ATTR> do_create_attributes(
		const bool tracks, const bool offsets, const bool lengths,
		const bool filenames,
		const std::vector<arcstk::checksum::type>& types_to_print) const final;

	virtual void configure_composer(ResultComposer& composer) const final;

	virtual void do_their_checksum(const std::vector<Checksum>& checksums,
		const arcstk::checksum::type t, const int entry, ResultComposer* b)
		const final;

	virtual void do_mine_checksum(const Checksums& checksums,
		const arcstk::checksum::type t, const int entry, ResultComposer* b,
		const bool match) const final;

	void assertions(InputTuple t) const final;

	std::unique_ptr<Result> do_format(InputTuple t) const final;
};


/**
 * \brief Abstract base class for an Application to perform calculations.
 */
class ARCalcApplicationBase : public Application
{
	/**
	 * \brief Run the internal calculation.
	 *
	 * Can be used as a worker for run().
	 *
	 * \param[in] options The options to run the application
	 *
	 * \return Application return code and calculation Result
	 */
	virtual std::pair<int, std::unique_ptr<Result>> run_calculation(
			const Options &options) const
	= 0;

	virtual int do_run(const Options &options) override;

protected:

	virtual bool calculation_requested(const Options &options) const;
};


/**
 * \brief Application to calculate AccurateRip checksums.
 */
class ARCalcApplication final : public ARCalcApplicationBase
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
		const std::vector<arcstk::checksum::type> &types,
		arcsdec::FileReaderSelection *audio_selection,
		arcsdec::FileReaderSelection *toc_selection);

private:

	/**
	 * \brief Create the printing format according to the options
	 *
	 * The format object returned will not yet have formatted the checksums.
	 *
	 * \param[in] options  The options parsed from command line
	 * \param[in] types    The requested checksum types
	 */
	std::unique_ptr<CalcResultFormatter> configure_layout(
			const Options &options,
			const std::vector<arcstk::checksum::type> &types) const;

	/**
	 * \brief Worker: Determine the requested checksum types for calculation.
	 *
	 * \param[in] options The options parsed from command line
	 *
	 * \return Checksum types to calculate.
	 */
	std::vector<arcstk::checksum::type> requested_types(const Options &options)
			const;


	std::string do_name() const final;

	std::string do_call_syntax() const final;

	std::unique_ptr<Configurator> do_create_configurator() const final;

	std::pair<int, std::unique_ptr<Result>> run_calculation(
			const Options &options) const final;
};

} // namespace arcsapp

#endif

