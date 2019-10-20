#ifndef __ARCSTOOLS_APPARCALC_HPP__
#define __ARCSTOOLS_APPARCALC_HPP__

/**
 * \file
 *
 * \brief Interface for ARCSApplication.
 *
 * Options, Configurator and Application for calc application.
 */

#ifndef __ARCSTOOLS_APPLICATION_HPP__
#include "application.hpp"
#endif
#ifndef __ARCSTOOLS_OUTPUTFORMATS_HPP__
#include "outputformats.hpp"
#endif
#ifndef __ARCSTOOLS_CONFIG_HPP__
#include "config.hpp"
#endif
#ifndef __ARCSTOOLS_OPTIONS_HPP__
#include "options.hpp"
#endif


/**
 * \brief Options for configuring ARCSApplication instances.
 */
class ARCalcOptions : public Options
{

public:


	/**
	 * \brief Output V1 ARCS
	 */
	static constexpr uint8_t V1 = 1;

	/**
	 * \brief Output V2 ARCS
	 */
	static constexpr uint8_t V2 = 2;

	/**
	 * \brief A metadata file is specified
	 */
	static constexpr uint8_t METAFILE = 4;

	/**
	 * \brief Output format is specified
	 */
	static constexpr uint8_t FMT = 8;

	/**
	 * \brief Output file is specified
	 */
	static constexpr uint8_t OUT = 16;

	/**
	 * \brief Skip front and back samples of the sample stream
	 */
	static constexpr uint8_t ALBUM = 32;

	/**
	 * \brief Skip front samples of first track
	 */
	static constexpr uint8_t FIRST = 64;

	/**
	 * \brief Skip back samples of last track
	 */
	static constexpr uint8_t LAST = 128;

	/**
	 * \brief Metadata file path
	 */
	static constexpr uint16_t METAFILEPATH = 256;

	/**
	 * \brief List compiled TOC formats
	 */
	static constexpr uint16_t LIST_TOC_FORMATS = 512;

	/**
	 * \brief List compiled audio formats (codec/container)
	 */
	static constexpr uint16_t LIST_AUDIO_FORMATS = 1024;
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


protected:

	std::unique_ptr<Options> parse_options(CLIParser& cli) override;
	// private in Configurator


private:

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

	/**
	 * \brief Create the printing format according to the options
	 *
	 * The format object returned will not yet have formatted the checksums.
	 *
	 * \param[in] options   The options parsed from command line
	 * \param[in] checksums The checksums to format
	 */
	std::unique_ptr<AlbumChecksumsTableFormat> create_format(
			const Options &options, const Checksums &checksums) const;

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

	std::unique_ptr<Configurator> create_configurator(int argc, char** argv)
		const override;

	std::string do_name() const override;

	int do_run(const Options &options) override;

	void do_print_usage() const override;
};

#endif

