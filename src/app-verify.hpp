#ifndef __ARCSTOOLS_APPVERIFY_HPP__
#define __ARCSTOOLS_APPVERIFY_HPP__

/**
 * \file
 *
 * \brief Interface for ARVerifyApplication.
 *
 * Options, Configurator and Application for verify.
 */

#include <cstdint>    // for uint16_t, uint32_t
#include <memory>     // for unique_ptr
#include <string>     // for string
#include <tuple>      // for tuple
#include <utility>    // for pair
#include <vector>     // for vector

#ifndef __LIBARCSTK_MATCH_HPP__
#include <arcstk/match.hpp>
#endif
#ifndef __LIBARCSTK_PARSE_HPP__
#include <arcstk/parse.hpp>
#endif

#ifndef __ARCSTOOLS_APPCALC_HPP__
#include "app-calc.hpp"          // for ARCalcConfigurator, CALC
#endif
#ifndef __ARCSTOOLS_APPLICATION_HPP__
#include "application.hpp"       // for Application
#endif
#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#include "layouts.hpp"           // for Layout, ResultFormatter
#endif


namespace arcsapp
{

class Configurator;
class Options;
class Result;

using arcstk::ARResponse;
using arcstk::Checksum;
using arcstk::Checksums;
using arcstk::Match;
using arcstk::Matcher;

/**
 * \brief Configuration options for ARVerifyApplications.
 *
 * Access options for verify exclusively by this class, not by CALCBASE.
 */
class VERIFY : public CALCBASE
{
	static constexpr auto& BASE = CALCBASE::SUBCLASS_BASE;

public:

	static constexpr OptionCode NOFIRST      = BASE +  0; // 20
	static constexpr OptionCode NOLAST       = BASE +  1;
	static constexpr OptionCode NOALBUM      = BASE +  2;
	static constexpr OptionCode RESPONSEFILE = BASE +  3;
	static constexpr OptionCode REFVALUES    = BASE +  4;
	static constexpr OptionCode PRINTALL     = BASE +  5;
	static constexpr OptionCode BOOLEAN      = BASE +  6;
	static constexpr OptionCode NOOUTPUT     = BASE +  7;
	static constexpr OptionCode COLORED      = BASE +  8; // 28
};


/**
 * \brief Configurator for ARVerifyApplication instances.
 *
 * Respects all VERIFY options.
 */
class ARVerifyConfigurator final : public ARCalcConfiguratorBase
{
public:

	using ARCalcConfiguratorBase::ARCalcConfiguratorBase;

private:

	void flush_local_options(OptionRegistry& r) const final;

	std::unique_ptr<Options> do_configure_options(
			std::unique_ptr<Options> options) const final;
};


/**
 * \brief Decorator to hilight matching checksums by color.
 */
class MatchDecorator final : public CellDecorator
{
	std::string do_decorate_set(std::string&& s) const final;

	std::string do_decorate_unset(std::string&& s) const final;

	std::unique_ptr<CellDecorator> do_clone() const final;

public:

	MatchDecorator(const std::size_t n);

	MatchDecorator(const MatchDecorator& rhs);

	~MatchDecorator() noexcept final = default;
};


/**
 * \brief Interface to format result objects of a verification process.
 */
using V9L = Layout<std::unique_ptr<Result>, Checksums, const ARResponse*,
			const std::vector<Checksum>, const Match*, const int,
			const TOC*, const ARId, std::string, std::vector<std::string>>;


/**
 * \brief Interface for formatting the results of an ARVerifyApplication.
 */
class VerifyResultFormatter : public ResultFormatter
							, public V9L
{
public:

	/**
	 * \brief Default constructor.
	 */
	VerifyResultFormatter();

	/**
	 * \brief Set the symbol to be printed on identity of two checksum values.
	 *
	 * \param[in] match_symbol The symbol to represent a match
	 */
	void set_match_symbol(const std::string& match_symbol);

	/**
	 * \brief The symbol to be printed on identity of two checksum values.
	 *
	 * \return Match symbol
	 */
	const std::string& match_symbol() const;

private:

	// V9L

	void assertions(InputTuple t) const final;

	std::unique_ptr<Result> do_format(InputTuple t) const final;

	// ResultFormatter

	virtual std::vector<ATTR> do_create_attributes(
		const bool tracks, const bool offsets, const bool lengths,
		const bool filenames,
		const std::vector<arcstk::checksum::type>& types_to_print,
		const int total_theirs) const final;

	// VerifyResultFormatter

	virtual void do_their_match(const Checksum& checksum, const int record,
			const int field, TableComposer* c) const
	= 0;

	virtual void do_their_mismatch(const Checksum& checksum, const int record,
			const int field, TableComposer* c) const
	= 0;

	std::string format_their_checksum(const Checksum& checksum,
		const bool does_match) const;

	/**
	 * \brief The symbol to be printed on identity of two checksum values.
	 */
	std::string match_symbol_;
};


/**
 * \brief Format monochrome output.
 */
class MonochromeVerifyResultFormatter : public VerifyResultFormatter
{
	void do_their_match(const Checksum& checksum, const int record,
			const int field, TableComposer* c) const final;

	void do_their_mismatch(const Checksum& checksum, const int record,
			const int field, TableComposer* c) const final;
};


/**
 * \brief Format colorized output.
 *
 * All cells containing matches are green.
 */
class ColorizingVerifyResultFormatter : public VerifyResultFormatter
{
	void do_their_match(const Checksum& checksum, const int record,
			const int field, TableComposer* c) const final;

	void do_their_mismatch(const Checksum& checksum, const int record,
			const int field, TableComposer* c) const final;
};


/**
 * \brief Application to verify AccurateRip checksums.
 */
class ARVerifyApplication final : public ARCalcApplicationBase
{
	/**
	 * \brief Configure an output format for the result.
	 *
	 * \param[in] options        The options to run the application
	 * \param[in] with_filenames Flag to indicate whether to print filenames
	 */
	std::unique_ptr<VerifyResultFormatter> configure_layout(
			const Options &options,
			const std::vector<arcstk::checksum::type> &types,
			const Match &match) const;

	/**
	 * \brief Provide reference checksums from options.
	 *
	 * Reference checksums are either represented as binary data provided by
	 * AccurateRip or as a list of numeric values provided the caller. Hence,
	 * the are either an ARResponse instance or a vector of Checksum instances.
	 *
	 * The caller is responsible to interpreting the resulting tuple.
	 *
	 * \return Tuple of possible reference checksum representations
	 */
	std::tuple<ARResponse, std::vector<Checksum>> get_reference_checksums(
			const Options &options) const;

	/**
	 * \brief Worker: parse the input for an ARResponse.
	 *
	 * \param[in] options The options to run the application
	 *
	 * \return ARResponse object
	 */
	ARResponse parse_response(const Options &options) const;

	/**
	 * \brief Worker: parse the input reference values.
	 *
	 * \param[in] values The reference values as passed from the cli
	 *
	 * \return Parsed checksums
	 */
	std::vector<Checksum> parse_refvalues(const Options &options) const;

	/**
	 * \brief Worker: parse the input reference values.
	 *
	 * \param[in] values The reference values as passed from the cli
	 *
	 * \return Parsed checksums
	 */
	std::vector<Checksum> parse_refvalues_sequence(const std::string &values)
		const;

	/**
	 * \brief Worker: Log matching files from a file list.
	 *
	 * \param[in] checksums Checksums to get matching tracks
	 * \param[in] match     Matcher to show match
	 * \param[in] block     The block to match tracks from
	 * \param[in] version   The ARCS version to match tracks for
	 */
	void log_matching_files(const Checksums &checksums,
		const Match &match, const uint32_t block,
		const bool version = true) const;

	/**
	 * \brief Print the result of the matching operation.
	 *
	 * \param[in] options         Call options
	 * \param[in] actual_sums     Actual checksums
	 * \param[in] reference_sums  Reference checksums
	 * \param[in] diff            Matcher result
	 * \param[in] toc             TOC
	 * \param[in] id              Album ARId
	 * \param[in] print_filenames Flag to print filenames
	 *
	 * \return A string representation of the result
	 */
	std::unique_ptr<Result> format_result(const Options &options,
			const Checksums &actual_sums,
			const std::tuple<ARResponse, std::vector<Checksum>> &reference_sums,
			const Matcher &diff, const TOC *toc, const ARId &id,
			const std::vector<std::string>& print_filenames) const;


	std::string do_name() const final;

	std::string do_call_syntax() const final;

	std::unique_ptr<Configurator> do_create_configurator() const final;

	std::pair<int, std::unique_ptr<Result>> run_calculation(
			const Options &options) const final;
};

} // namespace arcsapp

#endif

