#ifndef __ARCSTOOLS_CONFIG_HPP__
#define __ARCSTOOLS_CONFIG_HPP__

/**
 * \file
 *
 * \brief Process command line arguments to a configuration object.
 *
 * Provides class Configurator, the abstract base class for configurators. A
 * Configurator pull-parses the command line input to an Options instance if
 * (and only if) the input is syntactically wellformed and semantically valid.
 */

#include <stdint.h>      // for uint8_t
#include <memory>        // for unique_ptr
#include <string>        // for string
#include <type_traits>   // for underlying_type

#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#ifndef __ARCSTOOLS_CLITOKENS_HPP__
#include "clitokens.hpp"           // for CLITokens
#endif
#ifndef __ARCSTOOLS_OPTIONS_HPP__
#include "options.hpp"             // for Options, __ARCSTOOLS_OPTIONS_HPP__
#endif

namespace arcsapp
{

using arcstk::Logging;
using arcstk::Log;
using arcstk::LOGLEVEL;


/**
 * \brief Transform the cli tokens concerning logging to options.
 *
 * Delegate for the implementation of Configurator::configure_loglevel().
 */
class LogManager
{
public:

	/**
	 * \brief Constructor.
	 *
	 * Specify the level to choose by default and the level to choose for
	 * quietness.
	 *
	 * \param[in] default_level The default level to use
	 * \param[in] quiet_level   The level for quietness to use
	 */
	LogManager(const LOGLEVEL default_level, const LOGLEVEL quiet_level);

	/**
	 * \brief Return the default log level.
	 *
	 * \return The default log level for the application
	 */
	LOGLEVEL default_loglevel() const;

	/**
	 * \brief Return the quiet log level.
	 *
	 * \return The quiet log level for the application
	 */
	LOGLEVEL quiet_loglevel() const;

	/**
	 * \brief Deduce the log level from the cli input.
	 *
	 * \param[in] loglevel The loglevel value parsed
	 *
	 * \return The deduced log level
	 */
	static LOGLEVEL get_loglevel(const std::string& loglevel);

private:

	/**
	 * \brief Default log level.
	 */
	const LOGLEVEL default_loglevel_;

	/**
	 * \brief Quiet log level.
	 */
	const LOGLEVEL quiet_loglevel_;
};


/**
 * \brief Abstract base class for Configurators.
 *
 * A Configurator carries out every step that is necessary to provide the
 * configuration object.
 *
 * The following is the responsibility of the Configurator:
 * - Consume the command line tokens completely
 * - Decide about syntactic wellformedness or signal an error
 * - Verfiy that mandatory input is present
 * - Apply default values
 * - Manage side effects between options
 * - Decide whether input is to be ignored
 * - Compose an Options object for configuration
 * .
 *
 * Any subclass is responsible to report the options it supports specifically,
 * to parse the expected arguments (zero, one or many) and to configure the
 * parsed options to configuration settings.
 *
 * The following properties are considered equal for any of the applications
 * and are therefore implemented in the base class: version info, logging,
 * result output.
 *
 * To have logging fully configured at hand while parsing the command line
 * input, the global Logging is configured in the base class to have a common
 * implementation for all Configurator instances. The logging setup properties
 * as there are verbosity level and logging output stream is therefore not part
 * of the resulting Options.
 *
 * A subclass DefaultConfigurator is provided that consumes a single command
 * line argument and adds no application specific options.
 */
class Configurator
{
public:

	/**
	 * \brief Constructor.
	 */
	Configurator();

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~Configurator() noexcept;

	/**
	 * \brief Configure options applying configuration logic.
	 *
	 * CLI input is checked for semantic validity. It is checked that all valued
	 * options have legal values and that no illegal combination of options is
	 * present. Default values to options are applied, if defined. The input
	 * arguments are validated.
	 *
	 * \param[in] argc Number of CLI arguments
	 * \param[in] argv Array of CLI arguments
	 *
	 * \return The options object derived from the CLI arguments
	 */
	std::unique_ptr<Options> provide_options(const int argc,
		const char* const * const argv);

	/**
	 * \brief Return the list of supported options.
	 *
	 * \return List of supported options.
	 */
	static const std::vector<Option>& global_options();

	/**
	 * \brief Return the list of supported options.
	 *
	 * \return List of supported options.
	 */
	const std::vector<std::pair<Option, OptionCode>>& supported_options()
		const;

protected:

	/**
	 * \brief Globally managed options.
	 *
	 * The order of symbols MUST match the order in global_options_.
	 */
	enum class CONFIG : OptionCode
	{
		HELP      = 1,
		VERSION   = 2,
		VERBOSITY = 3,
		QUIET     = 4,
		LOGFILE   = 5,
		OUTFILE   = 6
	};

	/**
	 *  \brief Enumerable representation of global config options.
	 */
	static constexpr std::array<CONFIG, 6> global_id_ =
	{
		CONFIG::HELP,
		CONFIG::VERSION,
		CONFIG::VERBOSITY,
		CONFIG::QUIET,
		CONFIG::LOGFILE,
		CONFIG::OUTFILE
	};

public:

	/**
	 * \brief Returns the minimal code constant to be used by subclasses.
	 *
	 * Subclasses my declare their code range starting with this code + 1.
	 *
	 * \see ARIdOptions
	 * \see CALCBASE
	 */
	static constexpr OptionCode BASE_CODE() { return global_id_.size(); };

protected:

	/**
	 * \brief Access a global option by its index.
	 *
	 * Equivalent to global_options()[index].
	 *
	 * \param[in] conf Configuration item to get the option for
	 *
	 * \return The Option for \c conf
	 */
	static const Option& global(const CONFIG conf);

	/**
	 * \brief Create a list of all application-specific supported options.
	 *
	 * \return List of all application-specific supported options
	 */
	std::vector<std::pair<Option, OptionCode>> all_supported() const;

	/**
	 * \brief Process the global options in the parsed input.
	 *
	 * \return The Options after processing the global options
	 */
	std::unique_ptr<Options> process_global_options(const CLIInput &input)
		const;

private:

	/**
	 * \brief List of options supported by any ARApplication.
	 */
	static const std::vector<Option> global_options_;

	/**
	 * \brief Implements supported_options().
	 *
	 * The returned options will NOT include the global options.
	 *
	 * \return The options supported by this instance specifically
	 *
	 * \see global_options()
	 */
	virtual const std::vector<std::pair<Option, OptionCode>>&
		do_supported_options() const
	= 0;

	/**
	 * \brief Called by provide_options() after all options are parsed.
	 *
	 * The default implementation just returns the input.
	 *
	 * \param[in] options The Options to configure
	 *
	 * \return The Options instance derived from the command line input
	 */
	virtual std::unique_ptr<Options> do_configure_options(
			std::unique_ptr<Options> options);

	/**
	 * \brief The loglevel manager.
	 *
	 * Acts as a delegate to assisst the implementation of configure_loglevel().
	 */
	LogManager logman_;
};


/**
 * \brief Default Configurator to parse a single argument without any options.
 */
class DefaultConfigurator : public Configurator
{
public:

	using Configurator::Configurator;

private:

	const std::vector<std::pair<Option, OptionCode>>&
		do_supported_options() const override;
};

} // namespace arcsapp

#endif

