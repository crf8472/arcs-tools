#ifndef __ARCSTOOLS_APPARVERIFY_HPP__
#define __ARCSTOOLS_APPARVERIFY_HPP__

/**
 * \file
 *
 * \brief Interface for ARVerifyApplication.
 *
 * Options, Configurator and Application for verify.
 */

#include <cstdint>               // for uint16_t, uint32_t
#include <memory>                // for unique_ptr
#include <string>                // for string

#ifndef __LIBARCSTK_MATCH_HPP__
#include <arcstk/match.hpp>
#endif
#ifndef __LIBARCSTK_PARSE_HPP__
#include <arcstk/parse.hpp>
#endif

#ifndef __ARCSTOOLS_ARCALC_HPP__
#include "app-calc.hpp"          // for ARCalcConfigurator, CALC
#endif
#ifndef __ARCSTOOLS_APPLICATION_HPP__
#include "application.hpp"       // for ARApplication
#endif
#ifndef __ARCSTOOLS_PRINTERS_HPP__
#include "printers.hpp"          // for ARResponse, Checksums, Match
#endif

namespace arcsapp
{

class CLITokens;
class Configurator;
class Options;


using arcstk::Checksums;
using arcstk::Match;

/**
 * \brief Configuration options for ARVerifyApplications.
 *
 * Access options for verify exclusively by this class, not by CALCBASE.
 */
class VERIFY : public CALCBASE
{
	static constexpr auto& BASE = CALCBASE::MAX_CONSTANT;

public:
	static constexpr OptionValue NOFIRST      = BASE + 1;
	static constexpr OptionValue NOLAST       = BASE + 2;
	static constexpr OptionValue NOALBUM      = BASE + 3;
	static constexpr OptionValue RESPONSEFILE = BASE + 4;
	static constexpr OptionValue REFVALUES    = BASE + 5;
	static constexpr OptionValue PRINTALL     = BASE + 6;
	static constexpr OptionValue BOOLEAN      = BASE + 7;
	static constexpr OptionValue NOOUTPUT     = BASE + 8;
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

	const std::vector<std::pair<Option, OptionValue>>& do_supported_options()
		const override;

	std::unique_ptr<Options> do_configure_options(
			std::unique_ptr<Options> options) override;
};


/**
 * \brief Application to verify AccurateRip checksums.
 */
class ARVerifyApplication final : public ARApplication
{
	/**
	 * \brief Configure an output format for the result.
	 *
	 * \param[in] options        The options to run the application
	 * \param[in] with_filenames Flag to indicate whether to print filenames
	 */
	std::unique_ptr<MatchResultPrinter> configure_format(const Options &options,
		const bool with_filenames) const;

	/**
	 * \brief Worker: parse the input for an ARResponse.
	 *
	 * \param[in] options The options to run the application
	 */
	ARResponse parse_response(const Options &options) const;

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
	 * \brief Worker for run(): handles calculation requests.
	 *
	 * \param[in] options The options to run the application
	 *
	 * \return Application return code
	 */
	int run_calculation(const Options &options);


	std::string do_name() const override;

	std::string do_call_syntax() const override;

	std::unique_ptr<Configurator> create_configurator(
			int argc, char** argv) const override;

	int do_run(const Options &options) override;
};

} // namespace arcsapp

#endif

