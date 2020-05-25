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

class CLIParser;

/**
 * \brief Options for configuring ARCSApplication instances.
 */
class ARCalcOptions : public Options
{
public:

	/**
	 * \brief Do not output V1 ARCS
	 */
	static constexpr uint8_t NOV1 = 1;

	/**
	 * \brief Do not output V2 ARCS
	 */
	static constexpr uint8_t NOV2 = 2;

	/**
	 * \brief Skip front and back samples of the sample stream
	 */
	static constexpr uint8_t ALBUM = 4;

	/**
	 * \brief Skip front samples of first track
	 */
	static constexpr uint8_t FIRST = 8;

	/**
	 * \brief Skip back samples of last track
	 */
	static constexpr uint8_t LAST = 16;

	/**
	 * \brief A metadata file is specified
	 */
	static constexpr uint8_t METAFILE = 32;

	/**
	 * \brief List compiled TOC formats
	 */
	static constexpr uint16_t LIST_TOC_FORMATS = 128;

	/**
	 * \brief List compiled audio formats (codec/container)
	 */
	static constexpr uint16_t LIST_AUDIO_FORMATS = 256;

	/**
	 * \brief Metadata file path
	 */
	static constexpr uint16_t METAFILEPATH = 512;
};


/**
 * \brief Configurator for ARCSApplications.
 */
class ARCalcConfigurator : public Configurator
{

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

	int do_parse_arguments(CLIParser& cli, Options &options) const override;

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
	 * \param[in] options The options to use
	 */
	static std::tuple<Checksums, ARId, std::unique_ptr<TOC>> calculate(
			const Options &options);

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

