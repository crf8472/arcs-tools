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

#include <array>         // for array
#include <cstdint>       // for uint8_t
#include <climits>       // CHAR_BIT
#include <map>           // for map
#include <memory>        // for unique_ptr
#include <string>        // for string
#include <type_traits>   // for underlying_type
#include <utility>       // for pair, make_pair, move
#include <vector>        // for vector

#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#ifndef __LIBARCSDEC_SELECTION_HPP__
#include <arcsdec/selection.hpp>      // for FileReaderSelection
#endif

#ifndef __ARCSTOOLS_CLITOKENS_HPP__
#include "clitokens.hpp"              // for Option, OptionCode
#endif

namespace arcsapp
{


/**
 * \brief Reports a problem with the OptionsObject.
 */
class ConfigurationException : public std::runtime_error
{
public:

	/**
	 * \brief Constructor
	 *
	 * \param[in] what_arg What-Message
	 */
	ConfigurationException(const std::string &what_arg);
};


class Options;
std::ostream& operator << (std::ostream& out, const Options &options);


/**
 * \brief Configuration for an Application instance.
 *
 * An Options object contains the boolean as well as the valued options and
 * arguments for an Application. It represents the complete input configuration
 * for an application instance.
 */
class Options final
{
public:

	friend std::ostream& operator << (std::ostream& out,
			const Options &options);

	/**
	 * \brief Constructor.
	 */
	Options();

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~Options() noexcept;

	/**
	 * \brief Returns TRUE iff the option is set, otherwise FALSE.
	 *
	 * \param[in] option The option to check for
	 *
	 * \return TRUE iff the option is set, otherwise FALSE
	 */
	bool is_set(const OptionCode &option) const;

	/**
	 * \brief Set the option to TRUE with an empty value.
	 *
	 * If the option is currently set, the call has no effect.
	 *
	 * If \c option is OPTION::NONE or the setting of the option fails for some
	 * reason, an exception is thrown.
	 *
	 * Equivalent to set(option, std::string{}).
	 *
	 * \param[in] option The option to be set to TRUE
	 *
	 * \throws ConfigurationException Iff passed OPTION::NONE or on failure
	 */
	void set(const OptionCode &option);

	/**
	 * \brief Set the option to TRUE and add the specified value to it.
	 *
	 * If \c option is currently set, the value will be updated to \c value.
	 *
	 * If \c option is OPTION::NONE or the setting of the option fails for some
	 * reason, an exception is thrown.
	 *
	 * \param[in] option The option to be set to TRUE
	 * \param[in] value  The value for the option to put in
	 *
	 * \throws ConfigurationException Iff passed OPTION::NONE or on failure
	 */
	void set(const OptionCode &option, const std::string &value);

	/**
	 * \brief Set the option to FALSE.
	 *
	 * If the option is currently unset, the call has no effect. If the option
	 * is currently set and has a value, the value is erased.
	 *
	 * \param[in] option The option to be set to FALSE
	 */
	void unset(const OptionCode &option);

	/**
	 * \brief Get the value for a specified option.
	 *
	 * If the option is currently unset, the resulting value is empty.
	 *
	 * \param[in] option The option whose value to get
	 *
	 * \return The n-th value of the option passed
	 */
	std::string value(const OptionCode &option) const;

	/**
	 * \brief Puts an argument to the end of the argument list.
	 *
	 * \param[in] argument The argument to be appended to the list of arguments
	 */
	void put_argument(const std::string &argument);

	/**
	 * \brief Get an input argument by 0-based index.
	 *
	 * Will return the \c i-th argument inserted on command line.
	 *
	 * \param[in] i Index of the argument in the argument list
	 *
	 * \return Argument
	 */
	std::string const argument(const std::size_t i) const;

	/**
	 * \brief Get all input arguments in order of occurrence.
	 *
	 * \return All input arguments
	 */
	std::vector<std::string> const arguments() const;

	/**
	 * \brief Returns TRUE iff no arguments are present.
	 *
	 * \return TRUE iff no arguments are present otherwise FALSE
	 */
	bool no_arguments() const;

	/**
	 * \brief Returns TRUE iff no information is contained in this Options
	 * instance.
	 *
	 * \return TRUE if no information is contained in this Options instance
	 */
	bool empty() const;

private:

	/**
	 * \brief Options with their respective values.
	 *
	 * An Option is set iff it is present in this aggregate, otherwise it
	 * is unset.
	 */
	std::map<OptionCode, std::string> options_;

	/**
	 * \brief Arguments.
	 */
	std::vector<std::string> arguments_;
};


/**
 * \brief OptionCode for global options.
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
	 * \brief Internal type used by Configurator.
	 */
	using OptionRegistry = std::vector<std::pair<Option, OptionCode>>;

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
		const char* const * const argv) const;

	/**
	 * \brief Return the list of options supported by this Configurator.
	 *
	 * This will not contain the content of 'global_options()'.
	 *
	 * \return List of options supported by this Configurator
	 */
	const OptionRegistry& supported_options() const;

	/**
	 * \brief Options common to all subclasses of Configurator.
	 *
	 * \return List of options supported by every Configurator.
	 */
	static const OptionRegistry& common_options();

	/**
	 * \brief Returns the minimal OptionCode constant to be used by subclasses.
	 *
	 * Subclasses may declare their code range starting with this
	 * OptionCode + 1.
	 *
	 * \see ARIdOptions
	 * \see CALCBASE
	 */
	static constexpr OptionCode BASE() { return 6/*last member in OPTION*/; };

protected:

	/**
	 * \brief Worker: subclass may want to flush common options to own registry.
	 *
	 * \param[in] r Registry to flush common options to
	 */
	void flush_common_options_to(OptionRegistry& r) const;

private:

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
			std::unique_ptr<Options> options) const;

	/**
	 * \brief Registry of supported Options and their respective OptionCodes.
	 */
	OptionRegistry supported_options_;
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


/**
 * \brief Options to configure listing and reading/parsing of input files.
 *
 * Those options can be implemented by all applications that use libarcsdec
 * provided parsers and readers.
 */
struct FORMATBASE
{
private:

	static constexpr OptionCode BASE = Configurator::BASE();

public:

	// Info Output Options (no calculation)

	static constexpr OptionCode LIST_TOC_FORMATS   = BASE +  0; // 6
	static constexpr OptionCode LIST_AUDIO_FORMATS = BASE +  1;

	// Tool Selection Options

	static constexpr OptionCode READERID           = BASE +  2;
	static constexpr OptionCode PARSERID           = BASE +  3; // 9

protected:

	static constexpr OptionCode SUBCLASS_BASE      = BASE + 4;
};

} // namespace arcsapp

#endif

