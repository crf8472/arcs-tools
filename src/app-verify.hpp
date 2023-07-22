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


class ARVerifyConfiguration final : public Configuration
{
	void do_load();

public:

	using Configuration::Configuration;
};


class ColorRegistry;


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

	/**
	 * \brief Worker: parse the input for an ARResponse.
	 *
	 * \param[in] responsefile  The request string as passed from the cli
	 *
	 * \return ARResponse object
	 */
	ARResponse parse_response(const std::string &responsefile) const;

	/**
	 * \brief Worker: parse the input reference values.
	 *
	 * \param[in] value_list  The request string as passed from the cli
	 *
	 * \return Parsed checksums
	 */
	std::vector<Checksum> parse_refvalues(const std::string &value_list) const;

	/**
	 * \brief Worker: parse the colors requested by cli.
	 *
	 * \param[in] colors  The request string as passed from the cli
	 *
	 * \return Colors as requested.
	 */
	ColorRegistry parse_color_request(const std::string colors) const;


	// Configurator

	void do_flush_local_options(OptionRegistry& r) const final;

	std::unique_ptr<Options> do_configure_options(
			std::unique_ptr<Options> options) const final;

	std::unique_ptr<Configuration>do_create(
			std::unique_ptr<Options> options) const final;
};


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
	/**
	 * \brief Internal color for coloring matches.
	 */
	ansi::Color match_color_;

	/**
	 * \brief Internal color for coloring mismatches.
	 */
	ansi::Color mismatch_color_;


	// CellDecorator

	std::string do_decorate_set(std::string&& s) const final;

	std::string do_decorate_unset(std::string&& s) const final;

	std::unique_ptr<CellDecorator> do_clone() const final;

public:

	/**
	 * \brief Constructor.
	 *
	 * \param[in] n        Total number of decoratable entries
	 * \param[in] match    Color for coloring matches
	 * \param[in] mismatch Color for coloring mismatches
	 */
	MatchDecorator(const std::size_t n, const ansi::Color match,
			const ansi::Color mismatch);

	/**
	 * \brief Constructor.
	 *
	 * This constructor initializes the instance with the following defaults:
	 * FG_GREEN (BRIGHT) for matches, FG_RED (BRIGHT) for mismatches.
	 *
	 * \param[in] n        Total number of decoratable entries
	 */
	MatchDecorator(const std::size_t n);

	/**
	 * \brief Copy constructor.
	 *
	 * \param[in] rhs Instance to copy
	 */
	MatchDecorator(const MatchDecorator& rhs);

	/**
	 * \brief Default destructor.
	 */
	~MatchDecorator() noexcept final = default;

	/**
	 * \brief Return the color for coloring matches.
	 *
	 * \return Color for matches
	 */
	ansi::Color color_for_match() const;

	/**
	 * \brief Return the color for coloring mismatches.
	 *
	 * \return Color for mismatches
	 */
	ansi::Color color_for_mismatch() const;
};


/**
 * \brief Interface to format result objects of a verification process.
 *
 * Formatter for VERIFY results can use this interface for inheriting the
 * appropriate Layout interface.
 */
using Verify9Layout = Layout<std::unique_ptr<Result>
	,const Match*                     /* mandatory: match results          */
	,const int                        /* optional:  best block             */
	,const Checksums&                 /* mandatory: "mine" checksums       */
	,const ARId&                      /* optional:  "mine" ARId            */
	,const TOC*                       /* optional:  "mine" TOC             */
	,const ARResponse&                /* optional:  ref sums in ARResponse */
	,const std::vector<Checksum>&     /* optional:  ref sums passed        */
	,const std::vector<std::string>&  /* optional:  input audio filenames  */
	,const std::string&               /* optional:  AccurateRip URL prefix */
>;


/**
 * \brief Interface for formatting the results of an ARVerifyApplication.
 */
class VerifyResultFormatter : public ResultFormatter
							, public Verify9Layout
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

	// Verify9Layout

	void assertions(InputTuple t) const final;

	std::unique_ptr<Result> do_format(InputTuple t) const final;

	// ResultFormatter

	virtual std::vector<ATTR> do_create_attributes(
		const print_flag_t print_flags,
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
	// VerifyResultFormatter

	void do_their_match(const Checksum& checksum, const int record,
			const int field, TableComposer* c) const final;

	void do_their_mismatch(const Checksum& checksum, const int record,
			const int field, TableComposer* c) const final;
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
 * \param[in] type Name of the decoration type
 *
 * \return The DecorationType named as in \c type
 */
DecorationType get_decorationtype(const std::string& type);


/**
 * \brief Registry assigning output colors for DecorationTypes.
 */
class ColorRegistry final
{
	/**
	 * \brief Internal store for colors.
	 */
	std::map<DecorationType, ansi::Color> colors_;

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
	 * \brief Return color for coloring output of type \c d.
	 *
	 * \param[in] d  DecorationType to get actual color for
	 *
	 * \return Color for coloring output of type \c d.
	 */
	ansi::Color get(DecorationType d) const;

	/**
	 * \brief Set color for coloring output of type \c d.
	 *
	 * \param[in] d  DecorationType to assign an actual color to
	 * \param[in] c  Color for coloring output of type \c d.
	 */
	void set(DecorationType d, ansi::Color c);

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
class ColorizingVerifyResultFormatter : public VerifyResultFormatter
{
	/**
	 * \brief Internal color registry.
	 */
	ColorRegistry colors_;

	// ResultFormatter

	void init_composer(TableComposer* c) const final;

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
	 * \brief Return color for coloring output of type \c d.
	 *
	 * \param[in] d  DecorationType to get actual color for
	 *
	 * \return Color for coloring output of type \c d.
	 */
	ansi::Color color(DecorationType d) const;

	/**
	 * \brief Set color for coloring output of type \c d.
	 *
	 * \param[in] d  DecorationType to assign an actual color to
	 * \param[in] c  Color for coloring output of type \c d.
	 */
	void set_color(DecorationType d, ansi::Color c);
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
	 * \param[in] types    Checksum types to print
	 * \param[in] match    Match object to print
	 */
	std::unique_ptr<VerifyResultFormatter> create_formatter(
			const Configuration& config,
			const std::vector<arcstk::checksum::type> &types,
			const Match &match) const;

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


	// ARCalcApplicationBase

	std::pair<int, std::unique_ptr<Result>> do_run_calculation(
			const Configuration& config) const final;

	// Application

	std::string do_name() const final;

	std::string do_call_syntax() const final;

	std::unique_ptr<Configurator> do_create_configurator() const final;
};

} // namespace arcsapp

#endif

