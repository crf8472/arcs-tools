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

#ifndef __ARCSTOOLS_CLIPARSE_HPP__
#include "cliparse.hpp"  // for CLIParser
#endif

class Options;


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
	std::unique_ptr<Options> configure_options();


protected:

	/**
	 * Worker: consume an option from the command line and set TRUE/FALSE
	 * accordingly.
	 *
	 * \param[in] cli_option The command line switch for this option
	 * \param[in] option     The option id
	 * \param[in] cli        The command line input to inspect for options
	 * \param[in] options    The Options to add the value
	 */
	void check_for_option(const std::string cli_option,
		const uint8_t option, CLIParser &cli, Options &options) const;

	/**
	 * Worker: consume a valued option from the command line and set the value
	 * accordingly.
	 *
	 * \param[in] cli_option The command line switch for this option
	 * \param[in] option     The option id
	 * \param[in] cli        The command line input to inspect for options
	 * \param[in] options    The Options to add the value
	 */
	void check_for_option_with_argument(const std::string cli_option,
			const uint8_t option, CLIParser &cli, Options &options);

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


private:

	/**
	 * \brief Implements Configurator::configure_logging();
	 */
	virtual void do_configure_logging();

	/**
	 * \brief Worker method: parse the command line input to an object
	 * representation.
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
	virtual std::unique_ptr<Options> parse_options(CLIParser& cli)
	= 0;

	/**
	 * \brief Implements configure_options().
	 *
	 * \return The Options instance derived from the command line input
	 */
	virtual std::unique_ptr<Options> do_configure_options(
			std::unique_ptr<Options> options)
	= 0;

	/**
	 * \brief Internal representation of the CLIParser.
	 */
	CLIParser cli_;
};

#endif

