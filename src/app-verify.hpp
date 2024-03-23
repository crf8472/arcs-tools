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

#ifndef __LIBARCSTK_VERIFY_HPP__
#include <arcstk/verify.hpp>
#endif
#ifndef __LIBARCSTK_DBAR_HPP__
#include <arcstk/dbar.hpp>
#endif

#ifndef __ARCSTOOLS_ANSI_HPP__
#include "ansi.hpp"              // for Color
#endif
#ifndef __ARCSTOOLS_APPCALC_HPP__
#include "app-calc.hpp"          // for ARCalcConfigurator, CALC
#endif
#ifndef __ARCSTOOLS_APPLICATION_HPP__
#include "application.hpp"       // for Application
#endif
#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#include "layouts.hpp"           // for Layout
#endif
#ifndef __ARCSTOOLS_TOOLS_TABLE_HPP__
#include "tools-table.hpp"
#endif


namespace arcsapp
{
inline namespace v_1_0_0
{

class Configurator;
class Options;
class Result;

// libarcstk
using arcstk::Checksum;
using arcstk::Checksums;
using arcstk::DBAR;
using arcstk::VerificationResult;
using arcstk::Verifier;

// arcsapp
using table::TableComposer;
using RefValuesType = std::vector<uint32_t>;

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
	static constexpr OptionCode COLORED      = BASE +  8;
	static constexpr OptionCode CONFIDENCE   = BASE +  9; // 29
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

	// Configurator

	void do_flush_local_options(OptionRegistry& r) const final;

	std::unique_ptr<Options> do_configure_options(
			std::unique_ptr<Options> options) const final;

	void do_validate(const Options& o) const final;

	OptionParsers do_parser_list() const final;

	void do_validate(const Configuration& c) const final;
};


using arcstk::Checksum;
using arcstk::Checksums;
using arcstk::ChecksumSource;
using arcstk::ChecksumSourceOf;


/**
 * \brief Access list of reference values by block and index.
 */
class RefvaluesSource final : public ChecksumSourceOf<std::vector<uint32_t>>
{
	ARId do_id(const ChecksumSource::size_type block_idx) const final;
	Checksum do_checksum(const ChecksumSource::size_type block_idx,
			const ChecksumSource::size_type idx) const final;
	const uint32_t& do_arcs_value(const ChecksumSource::size_type block_idx,
			const ChecksumSource::size_type track_idx) const final;
	const uint32_t& do_confidence(const ChecksumSource::size_type block_idx,
			const ChecksumSource::size_type track_idx) const final;
	const uint32_t& do_frame450_arcs_value(
			const ChecksumSource::size_type block_idx,
			const ChecksumSource::size_type track_idx) const final;
	std::size_t do_size(const ChecksumSource::size_type block_idx) const final;
	std::size_t do_size() const final;

public:

	using ChecksumSourceOf::ChecksumSourceOf;
	using ChecksumSourceOf::operator=;
};


/**
 * \brief Dummy source for providing only empty checksums.
 */
class EmptyChecksumSource final : public ChecksumSource
{
	static const auto zero = uint32_t { 0 };

	ARId do_id(const ChecksumSource::size_type block_idx) const final;
	Checksum do_checksum(const ChecksumSource::size_type block_idx,
			const ChecksumSource::size_type idx) const final;
	const uint32_t& do_arcs_value(const ChecksumSource::size_type block_idx,
			const ChecksumSource::size_type track_idx) const final;
	const uint32_t& do_confidence(const ChecksumSource::size_type block_idx,
			const ChecksumSource::size_type track_idx) const final;
	const uint32_t& do_frame450_arcs_value(
			const ChecksumSource::size_type block_idx,
			const ChecksumSource::size_type track_idx) const final;
	std::size_t do_size(const ChecksumSource::size_type block_idx) const final;
	std::size_t do_size() const final;

public:

	EmptyChecksumSource();
};


/**
 * \brief Decoratable output cell categories.
 *
 * Decoratable cell categories are matches with "theirs" (MATCH), mismatches
 * with "theirs" (MISMATCH), and locally computed checksums (MINE).
 */
enum class DecorationType : int
{
	MATCH,
	MISMATCH,
	MINE
};


/**
 * \brief Get a DecorationType by its name.
 *
 * \param[in] name Name of the decoration type
 *
 * \return The DecorationType named as in \c name
 */
DecorationType get_decorationtype(const std::string& name);


/**
 * \brief Get name for a DecorationType.
 *
 * \param[in] type DecorationType to get name of
 *
 * \return Name of \c type
 */
std::string name(const DecorationType type);


/**
 * \brief Decorator to highlight matching checksums by color.
 *
 * If a cell is \c decorate_set() to TRUE, it will be printed in
 * \c color_for_match(). The default color for match is FG_GREEN (BRIGHT).
 *
 * If a cell is \c decorate_set() to FALSE, it will be printed in
 * \c color_for_mismatch(). The default color for mismatch is FG_RED (BRIGHT).
 */
class MatchDecorator final : public CellDecorator
{
	using Highlight = ansi::Highlight;
	using Color = ansi::Color;

	/**
	 * \brief Internal highlight store.
	 */
	Highlight highlights_[2];

	/**
	 * \brief Internal color store.
	 */
	Color colors_[4];

	// CellDecorator

	std::string do_decorate_set(std::string&& s) const final;

	std::string do_decorate_unset(std::string&& s) const final;

	std::unique_ptr<CellDecorator> do_clone() const final;

public:

	/**
	 * \brief Constructor.
	 *
	 * \param[in] n            Total number of decoratable entries
	 * \param[in] match_hl     Highlight for matches
	 * \param[in] fg_match     Foreground color for matches
	 * \param[in] bg_match     Background color for matches
	 * \param[in] mismatch_hl  Highlight for mismatches
	 * \param[in] fg_mismatch  Foreground color for mismatches
	 * \param[in] bg_mismatch  Background color for mismatches
	 */
	MatchDecorator(const std::size_t n, const Highlight match_hl,
		const Color fg_match, const Color bg_match,
		const Highlight mismatch_hl,
		const Color fg_mismatch, const Color bg_mismatch);

	/**
	 * \brief Constructor.
	 *
	 * \param[in] n            Total number of decoratable entries
	 * \param[in] match_hl     FG+BG highlight for matches
	 * \param[in] match        FG+BG color for matches
	 * \param[in] mismatch_hl  FG+BG highlight for mismatches
	 * \param[in] mismatch     FG+BG colors for mismatches
	 */
	MatchDecorator(const std::size_t n, const Highlight match_hl,
			const std::pair<Color, Color>& match, const Highlight mismatch_hl,
			const std::pair<Color, Color>& mismatch);

	/**
	 * \brief Default destructor.
	 */
	~MatchDecorator() noexcept final = default;

	/**
	 * \brief Return the background color for coloring matches.
	 *
	 * The first element is the foreground highlight, the second element is the
	 * background highlight.
	 *
	 * \param[in] d DecorationType to get highlights for
	 *
	 * \return Highlights for type \c d
	 */
	std::pair<ansi::Highlight, ansi::Highlight> highlights(
			const DecorationType& d) const;

	/**
	 * \brief Return highlight for specified decoration type.
	 *
	 * \param[in] d DecorationType to get highlight for
	 *
	 * \return Highlight for type \c d
	 */
	Highlight hl(const DecorationType& d) const;

	/**
	 * \brief Return the colors for the specified decoration type.
	 *
	 * The first element is the foreground color, the second element is the
	 * background color.
	 *
	 * \return Colors for type \c d
	 */
	std::pair<Color,Color> colors(const DecorationType& d) const;

	/**
	 * \brief Return foreground color for specified decoration type.
	 *
	 * \param[in] d DecorationType to get foreground color for
	 *
	 * \return Foreground color for type \c d
	 */
	Color fg(const DecorationType& d) const;

	/**
	 * \brief Return background color for specified decoration type.
	 *
	 * \param[in] d DecorationType to get background color for
	 *
	 * \return Background color for type \c d
	 */
	Color bg(const DecorationType& d) const;
};


/**
 * \brief Interface to format result objects of a verification process.
 *
 * Formatter for VERIFY results can use this interface for inheriting the
 * appropriate Layout interface.
 */
using Verify10Layout = Layout<std::unique_ptr<Result>
	,const std::vector<arcstk::checksum::type>& /* mandatory: types to print */
	,const VerificationResult*        /* mandatory: verification results   */
	,const int                        /* optional:  best block             */
	,const Checksums&                 /* mandatory: "mine" checksums       */
	,const ARId&                      /* optional:  "mine" ARId            */
	,const TOC*                       /* optional:  "mine" TOC             */
	,const DBAR&                      /* optional:  ref sums in DBAR       */
	,const std::vector<uint32_t>&     /* optional:  ref sums passed        */
	,const std::vector<std::string>&  /* optional:  input audio filenames  */
	,const std::string&               /* optional:  AccurateRip URL prefix */
>;


/**
 * \brief Interface for formatting the results of an ARVerifyApplication.
 */
class VerifyResultFormatter : public TableFormatter
							, public Verify10Layout
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

protected:

	/**
	 * \brief Worker: hook to change default labels in result table.
	 *
	 * This changes the labels of the local checksums to "Mine...".
	 *
	 * Called by init_composer().
	 *
	 * \param[in] c Composer for result table
	 */
	void update_field_labels(TableComposer& c) const;

	/**
	 * \brief Add result-specific fields to an existing field list.
	 *
	 * \param[in,out] field_list The field list to populate
	 * \param[in] print_flags    Fields requested for print
	 * \param[in] types_to_print Checksum types requested for print
	 * \param[in] total_theirs_per_block Total number of THEIRS fields per block
	 */
	void add_result_fields(std::vector<ATTR>& field_list,
		const print_flag_t print_flags,
		const std::vector<arcstk::checksum::type>& types_to_print,
		const std::size_t total_theirs_per_block) const;

	/**
	 * \brief Determine whether the DBAR object or the Refvalues are used.
	 *
	 * Use this function whenever to decide which source to choose or which
	 * source was actually chosen.
	 *
	 * \param[in] dBAR      DBAR object
	 * \param[in] refvalues Reference value list
	 *
	 * \return TRUE iff DBAR is the actual reference, otherwise FALSE
	 */
	bool reference_is_dbar(
			const DBAR& dBAR, const std::vector<uint32_t>& refvalues) const;

	/**
	 * \brief Determine the total number of blocks in reference.
	 *
	 * Value 0 is a legal result.
	 *
	 * Uses reference_is_dbar() to determine the actual reference source.
	 *
	 * \param[in] dBAR      DBAR object
	 * \param[in] refvalues Reference value list
	 *
	 * \return Actual number of blocks in reference
	 */
	std::size_t total_blocks_in_reference(
			const DBAR& dbar, const std::vector<uint32_t>& refvalues) const;

	/**
	 * \brief Create reference source
	 *
	 * Uses reference_is_dbar() to determine the actual reference object.
	 *
	 * \param[in] dBAR      DBAR object
	 * \param[in] refvalues Reference value list
	 *
	 * \return Type-normalized reference source
	 */
	std::unique_ptr<const ChecksumSource> create_reference_source(
			const DBAR& dbar, const std::vector<uint32_t>& refvalues) const;

	/**
	 * \brief Add result-specific data creators to an existing list of creators.
	 *
	 * \param[in,out] creators  List to be added to
	 * \param[in] print_flags   Fields requested for print
	 * \param[in] field_list    Ordered list of field types
	 * \param[in] types         Checksum types requested for print
	 * \param[in] vresult       VerificationResult (Matches/Mismatches)
	 * \param[in] block         Optional best block
	 * \param[in] checksums     Actual checksums
	 * \param[in] ref_source    Reference checksums
	 * \param[in] total_theirs_per_block Total number of THEIRS fields per block
	 */
	void populate_result_creators(
		std::vector<std::unique_ptr<FieldCreator>>& creators,
		const print_flag_t print_flags,
		const std::vector<ATTR>& field_list,
		const std::vector<arcstk::checksum::type>& types,
		const VerificationResult& vresult,
		const int block,
		const Checksums& checksums,
		const ChecksumSource& ref_source,
		const int total_theirs_per_block) const;

private:

	// Verify10Layout

	void assertions(InputTuple t) const final;

	std::unique_ptr<Result> do_format(InputTuple t) const final;

	// TableFormatter

	/*
	std::vector<ATTR> do_create_field_types(
		const print_flag_t print_flags,
		const std::vector<arcstk::checksum::type>& types_to_print,
		const int total_theirs) const final;
		*/

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
class MonochromeVerifyResultFormatter final : public VerifyResultFormatter
{
	// VerifyResultFormatter

	void do_init_composer(TableComposer& c) const override;

	void do_their_match(const Checksum& checksum, const int record,
			const int field, TableComposer* c) const final;

	void do_their_mismatch(const Checksum& checksum, const int record,
			const int field, TableComposer* c) const final;
};


/**
 * \brief Registry assigning output colors for DecorationTypes.
 */
class ColorRegistry final
{
	/**
	 * \brief Internal store for colors.
	 */
	std::unordered_map<DecorationType, std::pair<ansi::Color,ansi::Color>>
		colors_;

public:

	/**
	 * \brief Constructor.
	 */
	ColorRegistry();

	/**
	 * \brief Return TRUE iff registry has a color associated to specified type.
	 *
	 * \param[in] d  DecorationType to check
	 *
	 * \return Color for type \c d
	 */
	bool has(DecorationType d) const;

	/**
	 * \brief Return colors for coloring output of type \c d.
	 *
	 * \param[in] d  DecorationType to get actual color for
	 *
	 * \return Colors for coloring output of type \c d.
	 */
	std::pair<ansi::Color,ansi::Color> get(DecorationType d) const;

	/**
	 * \brief Return foreground color for coloring output of type \c d.
	 *
	 * \param[in] d  DecorationType to get actual color for
	 *
	 * \return Foreground color for coloring output of type \c d.
	 */
	ansi::Color get_fg(DecorationType d) const;

	/**
	 * \brief Return background color for coloring output of type \c d.
	 *
	 * \param[in] d  DecorationType to get actual color for
	 *
	 * \return Background color for coloring output of type \c d.
	 */
	ansi::Color get_bg(DecorationType d) const;

	/**
	 * \brief Set foreground color for coloring output of type \c d.
	 *
	 * Background color is set to BG_DEFAULT.
	 *
	 * \param[in] d  DecorationType to assign an actual color to
	 * \param[in] c  Color for coloring output of type \c d.
	 */
	void set_fg(DecorationType d, ansi::Color c);

	/**
	 * \brief Set background color for coloring output of type \c d.
	 *
	 * Background color is set to BG_DEFAULT.
	 *
	 * \param[in] d  DecorationType to assign an actual color to
	 * \param[in] c  Color for coloring output of type \c d.
	 */
	void set_bg(DecorationType d, ansi::Color c);

	/**
	 * \brief Set foreground and background color for coloring output of
	 * type \c d.
	 *
	 * \param[in] d  DecorationType to assign an actual color to
	 * \param[in] fg Color for coloring output of type \c d.
	 * \param[in] bg Color for coloring output of type \c d.
	 */
	void set(DecorationType d, ansi::Color fg, ansi::Color bg);

	/**
	 * \brief Delete all colors.
	 */
	void clear();
};


/**
 * \brief Format colorized output.
 *
 * All cells containing matches are green. All cells containing mismatches are
 * red.
 */
class ColorizingVerifyResultFormatter final : public VerifyResultFormatter
{
	using Color = ansi::Color;

	/**
	 * \brief Internal color registry.
	 */
	ColorRegistry colors_;

	/**
	 * \brief Worker: hook to register color decorators.
	 *
	 * Called by init_composer().
	 *
	 * \param[in] c TableComposer to modify
	 */
	void register_decorators(TableComposer& c) const;


	// TableFormatter

	void do_init_composer(TableComposer& c) const final;

	// VerifyResultFormatter

	void do_their_match(const Checksum& checksum, const int record,
			const int field, TableComposer* c) const final;

	void do_their_mismatch(const Checksum& checksum, const int record,
			const int field, TableComposer* c) const final;

public:

	/**
	 * \brief Default constructor.
	 */
	ColorizingVerifyResultFormatter();

	/**
	 * \brief Constructor.
	 *
	 * \param[in] colors Colorset to use
	 */
	ColorizingVerifyResultFormatter(const ColorRegistry& colors);

	/**
	 * \brief Return colors for coloring output of type \c d.
	 *
	 * \param[in] d  DecorationType to get actual color for
	 *
	 * \return Colors for coloring output of type \c d.
	 */
	std::pair<Color,Color> colors(DecorationType d) const;

	/**
	 * \brief Return foreground color for coloring output of type \c d.
	 *
	 * \param[in] d  DecorationType to get actual color for
	 *
	 * \return Foreground color for coloring output of type \c d.
	 */
	Color color_fg(DecorationType d) const;

	/**
	 * \brief Return background color for coloring output of type \c d.
	 *
	 * \param[in] d  DecorationType to get actual color for
	 *
	 * \return Background color for coloring output of type \c d.
	 */
	Color color_bg(DecorationType d) const;

	/**
	 * \brief Set foreground color for coloring output of type \c d.
	 *
	 * \param[in] d  DecorationType to assign an actual color to
	 * \param[in] c  Color for coloring output of type \c d.
	 */
	void set_color_fg(DecorationType d, Color c);

	/**
	 * \brief Set background color for coloring output of type \c d.
	 *
	 * \param[in] d  DecorationType to assign an actual color to
	 * \param[in] c  Color for coloring output of type \c d.
	 */
	void set_color_bg(DecorationType d, Color c);
};


/**
 * \brief Parser for a dBAR response, either from a file or from stdin.
 *
 * Accepts binary input for option VERIFY::RESPONSEFILE.
 */
class DBARParser final : public InputStringParser<DBAR>
{
	/**
	 * \brief Load DBAR from file or from stdin.
	 *
	 * In case the filename is empty, input is expected from stdin.
	 *
	 * \param[in] file The name of the response file
	 */
	DBAR load_data(const std::string& file) const;

	// InputStringParser

	std::string start_message() const final;

	DBAR do_parse_empty() const final;

	DBAR do_parse_nonempty(const std::string& s) const final;
};


/**
 * \brief Parser for a checksum list.
 *
 * Accepts a comma-separated list of hexadecimal values as input for option
 * VERIFY::REFVALUES.
 */
class ChecksumListParser final : public InputStringParser<std::vector<uint32_t>>
{
	std::string start_message() const final;

	std::vector<uint32_t> do_parse_nonempty(const std::string& s) const final;
};


/**
 * \brief Parser for a color specification.
 *
 * Accepts a comma-separated list of colon-separated name-value pairs. The name
 * is the name of a DecorationType. The value is a color specification, which is
 * either a single color name or a pair of color names separated by '+' (plus).
 */
class ColorSpecParser final : public InputStringParser<ColorRegistry>
{
	std::string start_message() const final;

	ColorRegistry do_parse_nonempty(const std::string& s) const final;
};


/**
 * \brief Application to verify AccurateRip checksums.
 */
class ARVerifyApplication final : public ARCalcApplicationBase
{
	/**
	 * \brief Configure an output format for the result.
	 *
	 * \param[in] config   The Application configuration
	 */
	std::unique_ptr<VerifyResultFormatter> create_formatter(
			const Configuration& config) const;

	/**
	 * \brief Worker: Log matching files from a file list.
	 *
	 * \param[in] checksums Checksums to get matching tracks
	 * \param[in] vresult   Result of the verification
	 * \param[in] block     The block to match tracks from
	 * \param[in] version   The ARCS version to match tracks for
	 */
	void log_matching_files(const Checksums &checksums,
		const VerificationResult& vresult, const uint32_t block,
		const bool version = true) const;


	// ARCalcApplicationBase

	std::pair<int, std::unique_ptr<Result>> do_run_calculation(
			const Configuration& config) const final;

	// Application

	std::string do_name() const final;

	std::string do_call_syntax() const final;

	std::unique_ptr<Configurator> do_create_configurator() const final;
};

} // namespace v_1_0_0
} // namespace arcsapp

#endif

