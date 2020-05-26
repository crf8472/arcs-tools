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
#include "app-calc.hpp"          // for ARCalcConfigurator, ARCalcOptions
#endif
#ifndef __ARCSTOOLS_APPLICATION_HPP__
#include "application.hpp"       // for ARApplication
#endif
#ifndef __ARCSTOOLS_PRINTERS_HPP__
#include "printers.hpp"     // for ARResponse, Checksums, Match
#endif

namespace arcsapp
{

class CLIParser;
class Configurator;
class Options;


using arcstk::Checksums;
using arcstk::Match;

/**
 * \brief Configuration options for ARVerifyApplications.
 */
class ARVerifyOptions : public ARCalcOptions
{
public:

	// ... 1 - 256 are from ARCalcOptions

	/**
	 * \brief Response data to parse for verification
	 */
	static constexpr uint16_t RESPONSEFILE = 1024;
};


/**
 * \brief Configurator for ARVerifyApplication instances.
 *
 * The ARVerifyConfigurator understands all options understood by the
 * ARCalcConfigurator.
 */
class ARVerifyConfigurator final : public ARCalcConfigurator
{ // XXX Non-leaf class ARCalcConfigurator is not abstract
public:

	/**
	 * \brief Constructor.
	 *
	 * \param[in] argc Number of command line arguments
	 * \param[in] argv Command line arguments
	 */
	ARVerifyConfigurator(int argc, char** argv);

private:

	const std::vector<std::pair<Option, uint32_t>>& do_supported_options() const
		override;
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
};

} // namespace arcsapp

#endif

