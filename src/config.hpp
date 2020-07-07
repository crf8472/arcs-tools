#ifndef __ARCSTOOLS_CONFIG_HPP__
#define __ARCSTOOLS_CONFIG_HPP__

/**
 * \file
 *
 * \brief Process command line arguments to a configuration object.
 *
 * Provides class Configurator, the abstract base class for configurators. A
 * Configurator push-parses the command line input to an Options instance if
 * (and only if) the input is syntactically wellformed and semantically valid.
 * It also assigns the default values to options that are not part of the input
 * and can apply configuring logic on the resulting object.
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
 * \brief Service class for configuring the loglevel.
 *
 * Implements the levels for 'default' and 'quiet' and can convert a string
 * to a loglevel.
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
	 * \brief Deduce the log level from a string representation.
	 *
	 * The string is expected to consist of digit symbols.
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
 *   - Parse the command line tokens
 *   - Ensure syntactic wellformedness or signal an error
 *   - Verify that mandatory input is present
 *   - Prevent illegal combination of options
 *   - Decide whether input is to be ignored
 *   - Apply default values
 *   - Manage side effects between options, i.e. adjust defaults
 *   - Compose an Options object for configuration
 *
 * Any subclass is responsible to report the options it supports specifically,
 * to parse the expected arguments (zero, one or many) and to configure the
 * parsed options to configuration settings.
 *
 * The following properties are considered equal for any of the applications
 * and are therefore implemented in the base class: 'help' option, version info,
 * logging, result output.
 *
 * To have logging fully configured at hand while parsing the command line
 * input, the global Logging is configured in the base class to have a common
 * implementation for all Configurator instances. The logging setup properties
 * as there are verbosity level and logging output stream is therefore not part
 * of the resulting Options.
 *
 * A subclass DefaultConfigurator is provided that does not add any application
 * specific options.
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
	 * \brief Parse, validate and configure options.
	 *
	 * CLI input is parsed. If the input is not syntactically wellformed or
	 * unrecognized options are present, a CallSyntaxException is thrown.
	 * The input is checked for semantic validity. It is checked that all valued
	 * options have legal values and that no illegal combination of options is
	 * present. Default values to options are applied, if defined. The input
	 * arguments are validated.
	 *
	 * \param[in] argc Number of CLI arguments
	 * \param[in] argv Array of CLI arguments
	 *
	 * \return The options object derived from the CLI arguments
	 *
	 * \throws CallSyntaxException If the input is not syntactically wellformed
	 */
	std::unique_ptr<Options> provide_options(const int argc,
		const char* const * const argv);

	/**
	 * \brief Return the list of options supported by every Configurator.
	 *
	 * \return List of options supported by every Configurator.
	 */
	static const std::vector<Option>& global_options();

	/**
	 * \brief Return the list of options supported by this Configurator.
	 *
	 * This will not contain the content of 'global_options()'.
	 *
	 * \return List of options supported by this Configurator
	 */
	const std::vector<std::pair<Option, OptionCode>>& supported_options()
		const;

protected:

	/**
	 * \brief \link OptionCode OptionCodes\endlink for global options.
	 *
	 * The order of symbols MUST match the order in global_options_.
	 * The symbols must be positive (minimal numerical value is 1).
	 */
	enum class OPTION : OptionCode
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
	static constexpr std::array<OPTION, 6> global_id_ =
	{
		OPTION::HELP,
		OPTION::VERSION,
		OPTION::VERBOSITY,
		OPTION::QUIET,
		OPTION::LOGFILE,
		OPTION::OUTFILE
	};

public:

	/**
	 * \brief Returns the minimal OptionCode constant to be used by subclasses.
	 *
	 * Subclasses my declare their code range starting with this OptionCode + 1.
	 *
	 * \see ARIdOptions
	 * \see CALCBASE
	 */
	static constexpr OptionCode BASE_CODE() { return global_id_.size(); };

protected:

	/**
	 * \brief Access a global option by its OptionCode.
	 *
	 * \param[in] conf OptionCode to get the Option for
	 *
	 * \return The Option corresponding to \c conf
	 */
	static const Option& global(const OPTION conf);

	/**
	 * \brief Create a list of all supported options of this Configurator.
	 *
	 * Creates the union of supported_options() and global_options().
	 *
	 * \return List of options supported by this Configurator
	 */
	std::vector<std::pair<Option, OptionCode>> all_supported() const;

	/**
	 * \brief Worker: process the global options in the parsed input.
	 *
	 * \return The Options after processing the global options
	 */
	std::unique_ptr<Options> process_global_options(const CLIInput &input)
		const;

private:

	/**
	 * \brief List of options supported by every Configurator.
	 */
	static const std::vector<Option> global_options_;

	/**
	 * \brief Implements supported_options().
	 *
	 * The returned options will NOT include the global options.
	 *
	 * \return List of options supported by this Configurator
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
	 */
	LogManager logman_;
};


/**
 * \brief Default Configurator without any specific options.
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

