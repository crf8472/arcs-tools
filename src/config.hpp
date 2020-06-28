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
#include <stdexcept>     // for runtime_error
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

//class Option;
//class Options;

using arcstk::Logging;
using arcstk::Log;
using arcstk::LOGLEVEL;


/**
 * \brief Reports a syntax error on parsing the command line input.
 */
class CallSyntaxException : public std::runtime_error
{
public:

	/**
	 * \brief Constructor
	 *
	 * \param[in] what_arg What-Message
	 */
	CallSyntaxException(const std::string &what_arg);
};


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
	 * \brief Empty constructor.
	 *
	 * \param[in] argc Number of CLI arguments
	 * \param[in] argv Array of CLI arguments
	 */
	Configurator(const int argc, const char* const * const argv);

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~Configurator() noexcept;

	/**
	 * \brief Configure global logging using CLI arguments.
	 */
	void configure_logging();

	/**
	 * \brief Configure options applying configuration logic.
	 *
	 * CLI input is checked for semantic validity. It is checked that all valued
	 * options have legal values and that no illegal combination of options is
	 * present. Default values to options are applied, if defined. The input
	 * arguments are validated.
	 *
	 * \return The options object derived from the CLI arguments
	 */
	std::unique_ptr<Options> provide_options();

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
	const std::vector<std::pair<Option, OptionValue>>& supported_options()
		const;

protected:

	/**
	 * \brief Worker: consume an option from the command line if present.
	 *
	 * \param[in] tokens The command line input to inspect for options
	 * \param[in] option The option to parse
	 *
	 * \return TRUE and the option value if the option was successfully consumed
	 *
	 * \throws CallSyntaxException On valued options with empty value
	 */
	std::pair<bool, std::string> option(CLITokens &tokens, const Option &option)
		const;

	/**
	 * \brief Service: consume a single command line argument if present.
	 *
	 * Intended as default implementation of do_parse_arguments() for subclasses
	 * that wish to support a single argument.
	 *
	 * If no argument was present, an empty string will be returned.
	 *
	 * This function can be called multiple times and each call will try to
	 * consume an argument.
	 *
	 * \param[in] tokens The command line input to inspect for an argument
	 *
	 * \return Argument string iff an argument could be consumed, otherwise
	 * empty string
	 */
	std::string argument(CLITokens &tokens) const;

	/**
	 * \brief Service: consume all command line arguments.
	 *
	 * Intended as default implementation of do_parse_arguments() for subclasses
	 * that wish to support multiple arguments.
	 *
	 * Warning: this consumes every present token as part of an argument list.
	 * After this function has been called, \tokens.empty() will be TRUE. Use
	 * this only as last parsing operation.
	 *
	 * \param[in] tokens The command line input to inspect for arguments
	 *
	 * \return List of arguments consumed
	 */
	std::vector<std::string> arguments(CLITokens &tokens) const;

	/**
	 * \brief Worker: deduce the loglevel value from command line and activate
	 * it.
	 *
	 * This function implements the effect of the VERBOSITY option.
	 *
	 * \return The current loglevel
	 */
	int configure_loglevel();

	/**
	 * \brief Configure the log appenders from command line.
	 *
	 * This function implements the effect of the LOGFILE option.
	 *
	 * If stdout is the only appender, the first element of the tuple will be
	 * 'stdout' while the second element of the tuple will be an empty string.
	 * (Giving the default Appender a name would disqualify this name as
	 * potential name of a concrete logfile.)
	 *
	 * \return A tuple of the name of the appender name and the filename
	 */
	std::tuple<std::string, std::string> configure_logappender();

	/**
	 * \brief Worker: called by parse_input() to parse the options
	 *
	 * \param[in] tokens The tokens to parse
	 *
	 * \return Options parsed from the command line input
	 *
	 * \throw CallSyntaxException Iff the options cannot be parsed
	 */
	std::unique_ptr<Options> parse_options(CLITokens& tokens);

	/**
	 * \brief Worker: called by parse_input() to parse the arguments.
	 *
	 * The CLITokens passed has the state immediately after parse_options() has
	 * been finished. Hence, whatever is left unparsed is not a supported
	 * option.
	 *
	 * The arguments parsed by this function can be append()ed to the
	 * options object passed.
	 *
	 * Directly after this method was called, it is checked whether the
	 * command line has been completely consumed. If tokens were left, after
	 * parse_arguments() has been finished, a CallSyntaxException will be
	 * thrown.
	 *
	 * Overriding this method can define whether the application supports
	 * no arguments, a single argument or multiple arguments. It also defines
	 * whether those arguments are optional or mandatory.
	 *
	 * The default implementation consumes exactly one argument.
	 *
	 * \param[in] tokens  The command line input to inspect for arguments
	 * \param[in] options The Options to append the arguments to
	 *
	 * \return Number of arguments that have been parsed
	 *
	 * \throws CallSyntaxException Iff the arguments are not syntactically ok
	 */
	int parse_arguments(CLITokens& tokens, Options &options) const;

	/**
	 * \brief Worker: parse the command line input to an object representation.
	 *
	 * Command line input is checked for syntactic wellformedness. It is checked
	 * that only legal options are present and that every option is present at
	 * most once.
	 *
	 * If <tt>parse_options()</tt> leaves unconsumed tokens from the command
	 * line input, this will result in a subsequent <tt>std::runtime_error</tt>.
	 *
	 * \param[in] tokens The command line input
	 *
	 * \return Options parsed from the command line input
	 *
	 * \throw CallSyntaxException if the call command cannot be parsed
	 */
	std::unique_ptr<Options> parse_input(CLITokens& tokens);

	/**
	 * \brief Globally managed options
	 *
	 * The order MUST match the order in supported_options_.
	 */
	enum class CONFIG : int
	{
		VERSION   = 0,
		VERBOSITY = 1,
		QUIET     = 2,
		LOGFILE   = 3,
		OUTFILE   = 4
	};

	/**
	 * \brief Access a supported option by its index.
	 *
	 * Equivalent to supported()[index].
	 *
	 * \param[in] conf Configuration item to get the option for
	 *
	 * \return The Option for \c conf
	 */
	static const Option& global(const CONFIG conf);

private:

	/**
	 * \brief List of options supported by any ARApplication.
	 */
	static const std::vector<Option> global_options_;

	/**
	 * \brief Start index for processing in parse_input().
	 *
	 * Every index < FIRST_UNPROCESSED_OPTION is a global option that will not
	 * make it into the Options object and can therefore just be skipped.
	 */
	//static const std::size_t FIRST_UNPROCESSED_OPTION;

	virtual const std::vector<std::pair<Option, OptionValue>>&
		do_supported_options() const
	= 0;

	/**
	 * \brief Implements parse_arguments(CLITokens&, Options&) const
	 *
	 * Consumes exactly one argument.
	 *
	 * \param[in] tokens  The command line input to inspect for arguments
	 * \param[in] options The Options to append the arguments to
	 *
	 * \return Number of arguments that have been parsed
	 *
	 * \throws CallSyntaxException Iff the arguments are not syntactically ok
	 */
	virtual int do_parse_arguments(CLITokens& tokens, Options &options) const;

	/**
	 * \brief Called by parse_input() after .
	 *
	 * The Options contain everything returned by parse_input(). This means,
	 * they already have been intercepted by parse_options() as
	 * well as parse_arguments().
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

	/**
	 * \brief Internal representation of the CLITokens.
	 */
	CLITokens tokens_;
};


/**
 * \brief Default Configurator to parse a single argument without any options.
 */
class DefaultConfigurator : public Configurator
{
public:

	DefaultConfigurator(int argc, char** argv)
		: Configurator(argc, argv)
	{ /* empty */ }

private:

	const std::vector<std::pair<Option, OptionValue>>&
		do_supported_options() const override;
};

} // namespace arcsapp

#endif

