#ifndef __ARCSTOOLS_CONFIG_HPP__
#define __ARCSTOOLS_CONFIG_HPP__

/**
 * \file
 *
 * \brief Turn command line arguments to a Options object.
 *
 * Provides class Configurator, the abstract base class for configurators. A
 * Configurator parses the command line input to an Options instance if
 * (and only if) they are syntactically wellformed and semantically valid.
 * The Options object represents the runtime configuration for an ARApplication
 * class.
 */

#include <stdint.h>      // for uint8_t
#include <memory>        // for unique_ptr
#include <stdexcept>     // for runtime_error
#include <string>        // for string
#include <type_traits>   // for underlying_type

#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#ifndef __ARCSTOOLS_CLIPARSE_HPP__
#include "cliparse.hpp"           // for CLIParser
#endif
#ifndef __ARCSTOOLS_OPTIONS_HPP__
#include "options.hpp"            // for Options, __ARCSTOOLS_OPTIONS_HPP__
#endif


class Option;
class Options;

using arcstk::Logging;
using arcstk::Log;
using arcstk::LOGLEVEL;


/**
 * \brief Reports an error on parsing the application call.
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
 * \brief Transform the cli values for logging to options.
 */
class LogManager
{
public:

	/**
	 * \brief Constructor
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
 * A Configurator parses the
 * command line input to an Options instance, thereby checking the input
 * for its syntactic wellformedness and its semantic validity. It applies
 * default values iff necessary.
 *
 * To have logging fully configured at hand while parsing the command line
 * input, the global Logging is configured separately and the logging setup is
 * not part of the Options.
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
	Configurator(int argc, char** argv);

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
	static const std::vector<Option>& supported();

protected:

	/**
	 * \brief Declare an option as supported.
	 *
	 * \param[in] option The option to support
	 * \param[in] id     ID for the option
	 */
	void support(const Option &option, const uint32_t id);

	/**
	 * \brief Worker: consume an option from the command line if present.
	 *
	 * \param[in] cli        The command line input to inspect for options
	 * \param[in] option     The option to parse
	 *
	 * \return TRUE and the value if the option could be consumed
	 *
	 * \throws CallSyntaxException On valued options with empty value
	 */
	std::pair<bool, std::string> option(CLIParser &cli, const Option &option)
		const;

	/**
	 * \brief Worker: consume a command line argument if present.
	 *
	 * Intended as default implementation of do_parse_arguments() for subclasses
	 * that wish to support a single argument.
	 *
	 * \param[in] cli The command line input to inspect for an argument
	 *
	 * \return TRUE if an argument could be consumed
	 */
	std::string argument(CLIParser &cli) const;

	/**
	 * \brief Worker: consume all command line arguments.
	 *
	 * Intended as default implementation of do_parse_arguments() for subclasses
	 * that wish to support multiple arguments.
	 *
	 * Warning: this consumes every present token as part of an argument list.
	 * Thereafter, tokens_left() will be FALSE. Use this only as last parsing
	 * step.
	 *
	 * \param[in] cli The command line input to inspect for arguments
	 *
	 * \return Number of arguments parsed
	 */
	std::vector<std::string> arguments(CLIParser &cli) const;

	/**
	 * \brief Deduce the loglevel value from command line and activate it.
	 *
	 * \return The current loglevel
	 */
	int configure_loglevel();

	/**
	 * \brief Configure the log appenders from command line.
	 *
	 * If stdout is the only appender, the first element of the tuple will be
	 * 'stdout' while the second element of the tuple will be an empty string.
	 *
	 * \return A tuple of the name of the appender and the filename
	 */
	std::tuple<std::string, std::string> configure_logappender();

	/**
	 * \brief Worker: called by parse_input() to parse the options
	 *
	 * \return Options parsed from the command line input
	 *
	 * \throw CallSyntaxException Iff the options cannot be parsed
	 */
	std::unique_ptr<Options> parse_options(CLIParser& cli);

	/**
	 * \brief Worker: called by parse_input() to parse the arguments
	 *
	 * The CLIParser holds the state after parse_options() has been executed,
	 * so whatever is left is not a supported option.
	 *
	 * The arguments parsed by this function can be append()ed to the
	 * options object passed.
	 *
	 * Directly after this method was called, it is checked whether the
	 * command line has been completely consumed. Iff tokens were left, a
	 * CallSyntaxException will fly.
	 *
	 * Overriding this method can define whether the application supports
	 * no arguments, a single argument or multiple arguments.
	 *
	 * The default implementation consumes exactly one argument.
	 *
	 * \param[in] cli The command line input to inspect for arguments
	 *
	 * \return Number of arguments that have been parsed
	 *
	 * \throws CallSyntaxException Iff the arguments are not syntactically ok
	 */
	int parse_arguments(CLIParser& cli, Options &options) const;

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
	 * \return Options parsed from the command line input
	 *
	 * \throw CallSyntaxException if the call command cannot be parsed
	 */
	std::unique_ptr<Options> parse_input(CLIParser& cli);

private:

	/**
	 * \copydoc parse_arguments(CLIParser&, Options&) const
	 */
	virtual int do_parse_arguments(CLIParser& cli, Options &options) const;

	/**
	 * \brief Implements configure_options().
	 *
	 * The Options hold the options hat have been returned by parse_input().
	 * This means, they already have been intercepted by parse_arguments().
	 *
	 * The default implementation just returns the input.
	 *
	 * \return The Options instance derived from the command line input
	 */
	virtual std::unique_ptr<Options> do_configure_options(
			std::unique_ptr<Options> options);

	/**
	 * \brief The loglevel manager.
	 */
	LogManager logman_;

	/**
	 * \brief Internal representation of the CLIParser.
	 */
	CLIParser cli_;

	/**
	 * \brief List of options supported by any ARApplication.
	 */
	static std::vector<Option> supported_options_;

	/**
	 * \brief List of ids
	 */
	static std::vector<uint32_t> supported_option_ids_;

	/**
	 * \brief Start index for processing in parse_input().
	 */
	const static std::size_t FIRST_UNPROCESSED_OPTION;
};

#endif

