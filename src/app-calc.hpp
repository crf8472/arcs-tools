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
 * \brief Options for configuring ARCSApplication instances.
 */
class ARCalcOptions : public Options
{
public:

	/**
	 * \brief Do not output V1 ARCS
	 */
	static constexpr OptionValue NOV1 = 1;

	/**
	 * \brief Do not output V2 ARCS
	 */
	static constexpr OptionValue NOV2 = 2;

	/**
	 * \brief Skip front and back samples of the sample stream
	 */
	static constexpr OptionValue ALBUM = 4;

	/**
	 * \brief Skip front samples of first track
	 */
	static constexpr OptionValue FIRST = 8;

	/**
	 * \brief Skip back samples of last track
	 */
	static constexpr OptionValue LAST = 16;

	/**
	 * \brief A metadata file is specified
	 */
	static constexpr OptionValue METAFILE = 32;

	/**
	 * \brief List compiled TOC formats
	 */
	static constexpr OptionValue LIST_TOC_FORMATS = 128;

	/**
	 * \brief List compiled audio formats (codec/container)
	 */
	static constexpr OptionValue LIST_AUDIO_FORMATS = 256;

	/**
	 * \brief Metadata file path
	 */
	static constexpr OptionValue METAFILEPATH = 512;


protected:

	static constexpr OptionValue MAX_VALUE = 512;
};


/**
 * \brief Configurator for ARCSApplications.
 */
class ARCalcConfigurator : public Configurator
{ // XXX Should be 'final' but ARVerifyConfigurator is a subclass!
public:

	/**
	 * \brief Empty constructor
	 *
	 * \param[in] argc Number of command line arguments
	 * \param[in] argv Command line arguments
	 */
	ARCalcConfigurator(int argc, char** argv);

	/**
	 * \brief Virtual default destructor
	 */
	~ARCalcConfigurator() noexcept override;

private:

	const std::vector<std::pair<Option, OptionValue>>& do_supported_options()
		const override;

	int do_parse_arguments(CLITokens& cli, Options &options) const override;

	std::unique_ptr<Options> do_configure_options(
			std::unique_ptr<Options> options) override;

};


/**
 * \brief Checksum Calculation Application
 */
class ARCalcApplication final : public ARApplication
{
public:

	/**
	 * \brief Do calculation based on the options passed.
	 *
	 * Implementation can use any of the \c options, but should not alter
	 * the set of requested types.
	 *
	 * \param[in] options The options to use
	 * \param[in] types   The checksum types requested
	 *
	 * \return Calculation result
	 */
	static std::tuple<Checksums, ARId, std::unique_ptr<TOC>> calculate(
			const Options &options,
			const std::set<arcstk::checksum::type> &types);

private:

	std::string do_name() const override;

	std::string do_call_syntax() const override;

	std::unique_ptr<Configurator> create_configurator(int argc, char** argv)
		const override;

	int do_run(const Options &options) override;

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
	 * \brief Determine the requested checksum types for calculation.
	 *
	 * \param[in] options   The options parsed from command line
	 *
	 * \return Checksum types to calculate.
	 */
	std::set<arcstk::checksum::type> requested_types(const Options &options)
		const;

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

