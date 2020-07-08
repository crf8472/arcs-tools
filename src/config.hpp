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

#include <cstdint>       // for uint8_t
#include <climits>       // CHAR_BIT
#include <map>           // for map
#include <memory>        // for unique_ptr
#include <string>        // for string
#include <type_traits>   // for underlying_type
#include <vector>        // for vector

#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#ifndef __ARCSTOOLS_CLITOKENS_HPP__
#include "clitokens.hpp"           // for CLITokens
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


class Options;
std::ostream& operator << (std::ostream& out, const Options &options);


/**
 * \brief Base class for configuration options.
 *
 * An Options object contains the complete configuration information for an
 * ARApplication.
 */
class Options
{
public:

	friend std::ostream& operator << (std::ostream& out, const Options &options);

	/**
	 * \brief Options with predefined number of flags.
	 */
	Options(const std::size_t size);

	/**
	 * \brief Default constructor.
	 */
	Options() : Options(sizeof(OptionCode) * CHAR_BIT) { /* empty */ };

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~Options() noexcept;

	/**
	 * \brief Set or unset the help flag.
	 *
	 * The help flag indicates whether the option HELP was passed.
	 *
	 * \param[in] help The flag to set or unset
	 */
	void set_help(const bool help);

	/**
	 * \brief Return the help flag.
	 *
	 * \return TRUE iff the help option is set, otherwise FALSE
	 */
	bool is_set_help() const;

	/**
	 * \brief Set or unset the version flag.
	 *
	 * The version flag indicates whether the option VERSION was passed.
	 *
	 * \param[in] version The flag to set or unset
	 */
	void set_version(const bool version);

	/**
	 * \brief Return the version flag.
	 *
	 * \return TRUE iff the version option is set, otherwise FALSE
	 */
	bool is_set_version() const;

	/**
	 * \brief Set the name of the output file.
	 *
	 * \param[in] output Name of the output file
	 */
	void set_output(const std::string &output);

	/**
	 * \brief Return the name of the output file.
	 *
	 * \return Name of the output file
	 */
	std::string output() const;

	/**
	 * \brief Inherited worker to implement getters for option checks.
	 *
	 * \param[in] option The option to check for
	 *
	 * \return TRUE iff the option is set, otherwise FALSE
	 */
	bool is_set(const OptionCode &option) const;

	/**
	 * \brief Set the option to TRUE.
	 *
	 * If the option is currently set, the call has no effect.
	 *
	 * \param[in] option The option to be set to TRUE
	 */
	void set(const OptionCode &option);

	/**
	 * \brief Set the option to FALSE.
	 *
	 * If the option is currently unset, the call has no effect.
	 *
	 * \param[in] option The option to be set to FALSE
	 */
	void unset(const OptionCode &option);

	/**
	 * \brief Get option value by key.
	 *
	 * \param[in] option The option whose value to get
	 *
	 * \return The value of the option passed
	 */
	std::string get(const OptionCode &option) const;

	/**
	 * \brief Set option value for key.
	 *
	 * \param[in] option The option to put in
	 * \param[in] value  The value for the option to put in
	 */
	void put(const OptionCode &option, const std::string &value);

	/**
	 * \brief Get all input options.
	 *
	 * \return All input arguments
	 */
	std::vector<bool> const get_flags() const;

	/**
	 * \brief Get all input arguments.
	 *
	 * \return All input arguments
	 */
	std::vector<std::string> const get_arguments() const;

	/**
	 * \brief Get an input argument by index.
	 *
	 * Will return the \c i-th argument inserted on command line.
	 *
	 * \param[in] i Index of the argument in the argument list
	 *
	 * \return Argument
	 */
	std::string const get_argument(const OptionCode &i) const;

	/**
	 * \brief Returns TRUE iff no arguments are present.
	 *
	 * \return TRUE iff no arguments are present otherwise FALSE
	 */
	bool no_arguments() const;

	/**
	 * \brief Puts an argument to the end of the argument list.
	 *
	 * \param[in] arg The argument to be appended to the list of arguments
	 */
	void append(const std::string &arg);

	/**
	 * \brief Returns TRUE iff no information is contained in this Options
	 * instance.
	 *
	 * \return TRUE if no information is contained in this Options instance
	 */
	bool empty() const;

private:

	/**
	 * \brief Flag to indicate presence of --help option
	 */
	bool help_;

	/**
	 * \brief Flag to indicate presence of --version option
	 */
	bool version_;

	/**
	 * \brief Name of the output stream
	 */
	std::string output_;

	/**
	 * \brief Boolean and valued options
	 */
	std::vector<bool> flags_; // TODO redundant

	/**
	 * \brief Valued options' values
	 */
	std::map<OptionCode, std::string> values_;

	/**
	 * \brief Arguments (non-option values)
	 */
	std::vector<std::string> arguments_;
};


/**
 * \brief \link OptionCode OptionCodes\endlink for global options.
 *
 * The order of symbols MUST match the order in global_options_.
 * The symbols must be positive (minimal numerical value is 1).
 */
struct OPTION
{
	static constexpr OptionCode NONE = Option::NONE; // NONE MUST be 0 for this
	static constexpr OptionCode HELP      = 1;
	static constexpr OptionCode VERSION   = 2;
	static constexpr OptionCode VERBOSITY = 3;
	static constexpr OptionCode QUIET     = 4;
	static constexpr OptionCode LOGFILE   = 5;
	static constexpr OptionCode OUTFILE   = 6;
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
	 *  \brief Enumerable representation of global config options.
	 */
	static constexpr std::array<OptionCode, 6> global_id_ =
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
	static const Option& global(const OptionCode conf);

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

