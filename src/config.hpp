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

#include <any>           // for any
#include <cstddef>       // for size_t
#include <functional>    // for function
#include <iterator>      // for end
#include <map>           // for map
#include <memory>        // for unique_ptr
#include <ostream>       // for ostream
#include <stdexcept>     // for runtime_error
#include <string>        // for string
#include <utility>       // for pair
#include <vector>        // for vector

#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#ifndef __ARCSTOOLS_CLITOKENS_HPP__
#include "clitokens.hpp"          // for Option, OptionCode
#endif
#ifndef __ARCSTOOLS_TOOLS_INPUT_HPP__
#include "tools-input.hpp"        // for StringParser
#endif

namespace arcsapp
{
inline namespace v_1_0_0
{


/**
 * \brief Reports a problem while defining the configuration.
 */
class ConfigurationException final : public std::runtime_error
{
public:

	/**
	 * \brief Constructor
	 *
	 * \param[in] what_arg What-Message
	 */
	ConfigurationException(const std::string& what_arg);
};


class Options;

/**
 * \brief Log an option object.
 *
 * \param[in] options  Options to log
 * \param[in] registry Option data
 */
void log_cli_input(const Options& options, const OptionRegistry& registry);


/**
 * \brief Configuration for an Application instance.
 *
 * An Options object contains the boolean as well as the valued options and
 * arguments for an Application. It represents the complete string input for an
 * application instance.
 *
 * Option values are just strings. They may require parsing or evaluation.
 */
class Options final
{
	// TODO Not the best solution, just make Options range-iterable
	friend void log_cli_input(const Options& options,
			const OptionRegistry& registry);

public:

	/**
	 * \brief Constructor.
	 */
	Options();

	/**
	 * \brief Returns TRUE iff the option is set, otherwise FALSE.
	 *
	 * \param[in] option The option to check for
	 *
	 * \return TRUE iff the option is set, otherwise FALSE
	 */
	bool is_set(const OptionCode& option) const;

	/**
	 * \brief Set the option to TRUE with an empty value.
	 *
	 * If the option is currently set, the call has no effect.
	 *
	 * If \p option is OPTION::NONE or the setting of the option fails for some
	 * reason, an exception is thrown.
	 *
	 * Equivalent to set(option, std::string{}).
	 *
	 * \param[in] option The option to be set to TRUE
	 *
	 * \throws ConfigurationException Iff passed OPTION::NONE or on failure
	 */
	void set(const OptionCode& option);

	/**
	 * \brief Set the option to TRUE and add the specified value to it.
	 *
	 * If \p option is currently set, the value will be updated to \p value.
	 *
	 * If \p option is OPTION::NONE or the setting of the option fails for some
	 * reason, an exception is thrown.
	 *
	 * \param[in] option The option to be set to TRUE
	 * \param[in] value  The value for the option to put in
	 *
	 * \throws ConfigurationException Iff passed OPTION::NONE or on failure
	 */
	void set(const OptionCode& option, const std::string& value);

	/**
	 * \brief Set the option to FALSE.
	 *
	 * If the option is currently unset, the call has no effect. If the option
	 * is currently set and has a value, the value is erased.
	 *
	 * \param[in] option The option to be set to FALSE
	 */
	void unset(const OptionCode& option);

	/**
	 * \brief Get the value for a specified option.
	 *
	 * If the option is currently unset, the resulting value is empty.
	 *
	 * \param[in] option The option whose value to get
	 *
	 * \return The n-th value of the option passed
	 */
	std::string value(const OptionCode& option) const;

	/**
	 * \brief Puts an argument to the end of the argument list.
	 *
	 * \param[in] argument The argument to be appended to the list of arguments
	 */
	void put_argument(const std::string& argument);

	/**
	 * \brief Get an input argument by 0-based index.
	 *
	 * Will return the \p i-th argument inserted on command line.
	 *
	 * \param[in] i Index of the argument in the argument list
	 *
	 * \return Argument
	 */
	std::string argument(const std::size_t i) const;

	/**
	 * \brief Get all input arguments in order of occurrence.
	 *
	 * \return All input arguments
	 */
	const std::vector<std::string>* arguments() const;

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
	 * \brief Arguments in the order they are passed.
	 */
	std::vector<std::string> arguments_;
};


/**
 * \brief OptionCode for global options.
 */
struct OPTION
{
	// This is not an enum but a struct to provide inheritance.
	// Concrete configurators should be able to add their options.

	static constexpr OptionCode NONE      = input::ARGUMENT; // MUST be 0
	static constexpr OptionCode HELP      = 1;
	static constexpr OptionCode VERSION   = 2;
	static constexpr OptionCode VERBOSITY = 3;
	static constexpr OptionCode QUIET     = 4;
	static constexpr OptionCode LOGFILE   = 5;
	static constexpr OptionCode OUTFILE   = 6;
	// BASE will be 7 for subclasses to start with

// TODO SUBCLASS_BASE would be systematic / wouldn't need Configuratior::BASE()
//
// protected:
//
// 	static constexpr OptionCode SUBCLASS_BASE = BASE + 7;
};


using OptionRegistry = std::vector<std::pair<OptionCode, Option>>;
//FIXME This definition is repeated from clitokens.hpp

/**
 * \brief TRUE iff OptionRegistry \p r contains a pair with OptionCode \p c.
 *
 * \param[in] c OptionCode to search for
 * \param[in] r Registry to search
 *
 * \return TRUE iff \p r contains \p c, otherwise FALSE
 */
bool contains(const arcsapp::OptionCode c, const arcsapp::OptionRegistry& r);


/**
 * \brief Registry to associate OptionCodes with StringParser implementations.
 *
 * Concrete Applications will implement this specifically. Note that the parsers
 * is represented by a lambda that instantiates it. The concrete parser class is
 * only instantiated when required.
 */
using OptionParsers = std::vector<std::pair<OptionCode,
		std::function<std::unique_ptr<StringParser>(void)>
		>>;


class Configuration;

/**
 * \brief Abstract base class for creating a configuration from options.
 *
 * A Configurator performs every step necessary to provide the configuration
 * object.
 *
 * The following is the responsibility of the Configurator:
 *   - Parse the command line tokens
 *   - Ensure syntactic wellformedness or signal an error
 *   - Verify that mandatory input is present
 *   - Prevent illegal combination of common options
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
 * verbosity or quietness, result output.
 *
 * A subclass DefaultConfigurator is provided that does not add any application
 * specific options.
 */
class Configurator
{
public:

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~Configurator() noexcept;

	/**
	 * \brief Parse, validate and configure options.
	 *
	 * Command line input \p argv is parsed. The caller is responsible that
	 * \p argc is the exact size of \p argv. Otherwise, crashes are likely.
	 *
	 * If the call string is not syntactically wellformed or unrecognized options are
	 * present, a CallSyntaxException is thrown.
	 *
	 * \param[in] argc Number of CLI arguments
	 * \param[in] argv Array of CLI arguments
	 *
	 * \return The options object derived from the command line arguments
	 *
	 * \throws CallSyntaxException If the call string is not syntactically wellformed
	 */
	std::unique_ptr<Options> read_options(const int argc,
		const char* const* const argv) const;

	/**
	 * \brief Check and validate options
	 *
	 * The input is checked for semantic validity. It is checked that all valued
	 * options have legal values and that no illegal combination of options is
	 * present. Default values to options are applied, if defined.
	 *
	 * After this checks are completed, do_configure_options() is called which
	 * is defined by subclasses.
	 *
	 * It is not guaranteed that the returned pointer points to the same object
	 * as the input pointer. It is safe to assign the output to the input.
	 *
	 * \param[in] options The options to configure
	 *
	 * \return Configured options
	 *
	 * \throws ConfigurationException If the command line input does not form
	 * semantically valid run configuration
	 */
	std::unique_ptr<Options> configure_options(std::unique_ptr<Options> options)
		const;

	/**
	 * \brief Return the list of options supported by this Configurator.
	 *
	 * The list is generated whenever this function is called.
	 *
	 * \return List of options supported by this Configurator
	 */
	OptionRegistry supported_options() const;

	/**
	 * \brief Returns the minimal OptionCode constant to be used by subclasses.
	 *
	 * Subclasses may declare their numerical range starting with this
	 * OptionCode.
	 *
	 * \return Minimal OptionCode to be used by subclasses
	 *
	 * \see ARIdOptions
	 * \see CALCBASE
	 * \see FORMATBASE
	 */
	static constexpr OptionCode BASE() { return 7/* last OPTION + 1 */; };

	/**
	 * \brief Load the specified options into a Configuration.
	 *
	 * All parseable option value strings are parsed. The resulting
	 * configuration object is validated..
	 *
	 * \param[in] options Options as provided by this configurator.
	 *
	 * \return Configuration reflecting options
	 *
	 * \throws ConfigurationException If options are illegal or values are
	 * unparseable
	 */
	std::unique_ptr<Configuration> create(std::unique_ptr<Options> options)
		const;

protected:

	/**
	 * \brief Options common to all subclasses of Configurator.
	 *
	 * The list is generated whenever this function is called.
	 *
	 * \return List of options supported by every Configurator.
	 */
	OptionRegistry common_options() const;

	/**
	 * \brief Worker: apply all parsers for option values.
	 *
	 * This parses all parseable option value strings. The list of parsers to
	 * apply is created by do_parser_list().
	 *
	 * \param[in] config Configuration to parse values
	 */
	void apply_parsers(Configuration& config) const;

private:

	/**
	 * \brief Hook: called by supported_options() to flush options supported by
	 * the specific subclass so support list.
	 *
	 * \param[in,out] supported List of supported options.
	 */
	virtual void do_flush_local_options(OptionRegistry& supported) const
	= 0;

	/**
	 * \brief Hook: called by provide_options() after all options have
	 * been parsed.
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
	 * \brief Hook: called by provide_options() on the result of
	 * do_configure_options().
	 *
	 * The default implementation does not perform any checks.
	 *
	 * Place all checks here that can be done without having parsed the option
	 * value strings. This entails e.g. checks for illegal option combinations.
	 *
	 * \param[in] options The Options to validate
	 */
	virtual void do_validate(const Options& options) const;

	/**
	 * \brief Hook: called by create() for the list of parseable options and
	 * their corresponding parsers.
	 *
	 * The default implementation provides an empty list.
	 *
	 * \return List of parseable options and their corresponding parsers
	 */
	virtual OptionParsers do_parser_list() const;

	/**
	 * \brief Hook: called by create() to validate configuration.
	 *
	 * The default implementation does not perform any checks.
	 *
	 * Place all checks here that presuppose the option values to be parsed.
	 * This entails e.g. checks for the number of elements in lists or whether
	 * values were empty..
	 *
	 * \param[in] configuration Configuration to validate
	 */
	virtual void do_validate(const Configuration& configuration) const;
};


/**
 * \brief Application input as there is configuration and arguments.
 */
class Configuration final
{
	/**
	 * \brief Options.
	 */
	std::unique_ptr<Options> options_;

	/**
	 * \brief Configuration objects.
	 */
	std::map<OptionCode, std::any> objects_;

public:

	/**
	 * \brief Constructor.
	 *
	 * \param[in] options Options to configure the application
	 */
	Configuration(std::unique_ptr<Options> options);

	/**
	 * \brief Put an object into the Configuration.
	 *
	 * Inspect the internal Options and parse all their input strings to
	 * objects.
	 *
	 * \param[in] option The option to put the value object for
	 * \param[in] object The value object to put
	 */
	void put(const OptionCode& option, const std::any& object);

	/**
	 * \brief Get a configuration object.
	 *
	 * \param[in] option The option to get the value object for
	 *
	 * \return Value object for the option passed
	 */
	template <typename T>
	auto object_ptr(const OptionCode& option) const -> const T*
	{
		auto p { objects_.find(option) };

		using std::end;
		if (end(objects_) == p)
		{
			return nullptr;
		}

		return std::any_cast<T>(&p->second);
	}

	/**
	 * \brief Get a configuration object.
	 *
	 * \tparam T Configuration object type
	 *
	 * \param[in] option Option code to get value for
	 *
	 * \return Object for option \p option
	 */
	template <typename T>
	auto object(const OptionCode& option) const -> T
	{
		auto p { this->object_ptr<T>(option) };
		return p != nullptr ? *p : T{ /* empty */ };
	}

	// Provide interface for options

	/**
	 * \brief Returns TRUE iff the option is set, otherwise FALSE.
	 *
	 * \param[in] option The option to check for
	 *
	 * \return TRUE iff the option is set, otherwise FALSE
	 */
	bool is_set(const OptionCode& option) const;

	/**
	 * \brief Get the value for a specified option.
	 *
	 * If the option is currently unset, the resulting value is empty.
	 *
	 * \param[in] option The option whose value to get
	 *
	 * \return The n-th value of the option passed
	 */
	std::string value(const OptionCode& option) const;

	/**
	 * \brief Get an input argument by 0-based index.
	 *
	 * Will return the \p i-th argument inserted on command line.
	 *
	 * \param[in] i Index of the argument in the argument list
	 *
	 * \return Argument
	 */
	std::string argument(const std::size_t i) const;

	/**
	 * \brief Get all input arguments in order of occurrence.
	 *
	 * \return All input arguments
	 */
	const std::vector<std::string>* arguments() const;

	/**
	 * \brief Returns TRUE iff no arguments are present.
	 *
	 * \return TRUE iff no arguments are present otherwise FALSE
	 */
	bool no_arguments() const;
};


/**
 * \brief Default Configurator without any specific options.
 */
class DefaultConfigurator final : public Configurator
{
public:

	using Configurator::Configurator;

private:

	// Configurator

	void do_flush_local_options(OptionRegistry& r) const override;

	//std::unique_ptr<Options> do_configure_options(
	//		std::unique_ptr<Options> options) const final;

	// void do_validate(const Options& options) const;

	// OptionParsers do_parser_list() const;

	// void do_validate(const Configuration& configuration) const;
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

	static constexpr OptionCode LIST_TOC_FORMATS   = BASE +  0; //  7
	static constexpr OptionCode LIST_AUDIO_FORMATS = BASE +  1;

	// Tool Selection Options

	static constexpr OptionCode READERID           = BASE +  2;
	static constexpr OptionCode PARSERID           = BASE +  3; // 10

protected:

	static constexpr OptionCode SUBCLASS_BASE      = BASE + 4;
};


/**
 * \brief Parse \p list as a sequence of strings separated by \p delim and call
 * \p entry_hook on each of them.
 *
 * \param[in] list       Input string to parse as a list
 * \param[in] delim      Delimiter for list entries
 * \param[in] entry_hook Call this function on each entry
 */
void parse_list(const std::string& list, const char delim,
		std::function<void(const std::string& s)> entry_hook);


/**
 * \brief Parse \p list as a sequence of strings separated by \p delim and
 * convert each entry by \p convert_func.
 *
 * \tparam T Type of requested objects
 *
 * \param[in] list         Input string to parse as a list
 * \param[in] delim        Delimiter for list entries
 * \param[in] convert_func Function to convert std::string to T
 *
 * \return Sequence of input values converted from strings
 */
template <typename T>
inline std::vector<T> parse_list_to_objects(const std::string& list,
		const char delim,
		const std::function<T(const std::string& s)>& convert_func)
{
	auto results = std::vector<T> {};
	// TODO reserve default?

	parse_list(list, delim,
			[&convert_func,&results](const std::string& s)
			{
				results.emplace_back(convert_func(s));
			});

	return results;
}


} // namespace v_1_0_0
} // namespace arcsapp

#endif

