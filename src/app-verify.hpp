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
 */
class VERIFY : public CALCBASE
{
	static constexpr auto& MY = CALCBASE::MAX_CONSTANT;

public:
	static constexpr OptionValue RESPONSEFILE = MY + 1;
	static constexpr OptionValue REFVALUES    = MY + 2;
	static constexpr OptionValue PRINTALL     = MY + 3;
	static constexpr OptionValue BOOLEAN      = MY + 4;
	static constexpr OptionValue NOOUTPUT     = MY + 5;
};


/**
 * \brief Configurator for ARVerifyApplication instances.
 *
 * The ARVerifyConfigurator understands all options understood by the
 * ARCalcConfigurator.
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
 * \brief AccurateRip Checksum Verifying Application.
 */
class ARVerifyApplication final : public ARApplication
{
	std::string do_name() const override;

	std::string do_call_syntax() const override;

	std::unique_ptr<Configurator> create_configurator(
			int argc, char** argv) const override;

	int do_run(const Options &options) override;

	/**
	 * \brief Parse the input for an ARResponse
	 */
	ARResponse parse_response(const Options &options) const;

	/**
	 * \brief Configure an output format for the result
	 */
	std::unique_ptr<MatchResultPrinter> configure_format(const Options &options,
		const bool with_filenames) const;

	/**
	 * \brief Log matching files from a file list.
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

