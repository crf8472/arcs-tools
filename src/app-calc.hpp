#ifndef __ARCSTOOLS_APPCALC_HPP__
#define __ARCSTOOLS_APPCALC_HPP__

/**
 * \file
 *
 * \brief Interface for ARCSApplication.
 *
 * Options, Configurator and Application for calc application.
 */

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
#include "application.hpp"         // for Application
#endif
#ifndef __ARCSTOOLS_CONFIG_HPP__
#include "config.hpp"              // for Configurator
#endif
#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#include "layouts.hpp"             // for Layout
#endif
#ifndef __ARCSTOOLS_TOOLS_TABLE_HPP__
#include "tools-table.hpp"         // for TableFormatter
#endif


namespace arcsapp
{

class Result;

// libarcstk
using arcstk::ARId;
using arcstk::Checksums;
using arcstk::TOC;

// arcsapp
using table::ATTR;
using table::FieldCreator;
using table::TableFormatter;

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

	// Configurator

	void do_flush_local_options(OptionRegistry& r) const final;

	std::unique_ptr<Options> do_configure_options(
			std::unique_ptr<Options> options) const final;

	// void do_validate(const Options& options) const;

	// OptionParsers do_parser_list() const;

	// void do_validate(const Configuration& configuration) const;
};


/**
 * \brief Interface to format result objects of a calculation process.
 */
using Calc6Layout = Layout<std::unique_ptr<Result>
	,const std::vector<arcstk::checksum::type>& /* mandatory: types to print */
	,const Checksums&                /* mandatory: locally computed checksums */
	,const ARId&                     /* optional: ARId */
	,const TOC*                      /* optional: TOC */
	,const std::vector<std::string>& /* optional: input audio filenames */
	,const std::string&              /* optional: AccurateRip URL prefix */
>;


/**
 * \brief Format the results of the ARCalcApplication.
 */
class CalcResultFormatter final : public TableFormatter
								, public Calc6Layout
{
protected:

	/**
	 * \brief Add result-specific fields to an existing field list.
	 *
	 * \param[in,out] field_list The field list to populate
	 * \param[in] print_flags    Fields requested for print
	 * \param[in] types_to_print Checksum types requested for print
	 */
	void add_result_fields(std::vector<ATTR>& field_list,
		const print_flag_t print_flags,
		const std::vector<arcstk::checksum::type>& types_to_print) const;

	/**
	 * \brief Add result-specific data creators to an existing list of creators.
	 *
	 * \param[in,out] creators  List to be added to
	 * \param[in] print_flags   Fields requested for print
	 * \param[in] field_list    Ordered list of field types
	 * \param[in] types         Checksum types requested for print
	 * \param[in] checksums     Actual checksums
	 */
	void populate_result_creators(
		std::vector<std::unique_ptr<FieldCreator>>& creators,
		const print_flag_t print_flags,
		const std::vector<ATTR>& field_list,
		const std::vector<arcstk::checksum::type>& types,
		const Checksums& checksums) const;

private:

	/*
	std::vector<ATTR> do_create_field_types(
		const print_flag_t print_flags,
		const std::vector<arcstk::checksum::type>& types_to_print,
		const int total_theirs) const final;
		*/

	void assertions(InputTuple t) const final;

	std::unique_ptr<Result> do_format(InputTuple t) const final;
};


/**
 * \brief Abstract base class for an Application to perform calculations.
 */
class ARCalcApplicationBase : public Application
{
	virtual bool do_calculation_requested(const Configuration& config) const;

	virtual std::vector<arcstk::checksum::type> do_requested_types(
			const Configuration &options) const;

	virtual std::pair<int, std::unique_ptr<Result>> do_run_calculation(
			const Configuration &config) const
	= 0;

	// Application

	virtual int do_run(const Configuration &config) override;

protected:

	/**
	 * \brief Return TRUE iff the configuration requires a calculation.
	 *
	 * The result of this function is used to decide whether
	 * \c run_calculation() will actually be called.
	 *
	 * \param[in] config Application configuration
	 *
	 * \return TRUE iff a calculation is actually requested
	 */
	bool calculation_requested(const Configuration& config) const;

	/**
	 * \brief Worker: Determine the requested checksum types for calculation.
	 *
	 * \param[in] options The options parsed from command line
	 *
	 * \return Checksum types to calculate.
	 */
	std::vector<arcstk::checksum::type> requested_types(
			const Configuration &options) const;

	/**
	 * \brief Run the actual calculation.
	 *
	 * Can be used as a worker for run().
	 *
	 * \param[in] config Application configuration
	 *
	 * \return Exit code and calculation result
	 */
	std::pair<int, std::unique_ptr<Result>> run_calculation(
			const Configuration &config) const;
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
	 */
	std::unique_ptr<CalcResultFormatter> create_formatter(
			const Configuration &config) const;


	// ARCalcApplicationBase

	std::pair<int, std::unique_ptr<Result>> do_run_calculation(
			const Configuration &config) const final;

	// Application

	std::string do_name() const final;

	std::string do_call_syntax() const final;

	std::unique_ptr<Configurator> do_create_configurator() const final;
};

} // namespace arcsapp

#endif

