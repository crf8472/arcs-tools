#ifndef __ARCSTOOLS_OPTIONS_HPP__
#define __ARCSTOOLS_OPTIONS_HPP__

/**
 * \file
 *
 * \brief Options for runtime configuration.
 *
 * The Options class represents the complete command line input to an
 * application. Options are produced by a Configurator.
 *
 * A single Option instance represents a supported command line option.
 */

#include <cstdint>   // for uint16_t
#include <map>       // for map
#include <string>    // for string
#include <vector>    // for vector

namespace arcsapp
{

using OptionValue = uint64_t;

/**
 * \brief Descriptor for a single command line option.
 *
 * An Option has a symbol and may or may not have a shorthand symbol. It may
 * or may not expect a value and most options have some default. An option also
 * has a short description.
 */
class Option
{
public:

	/**
	 * \brief Constructor
	 *
	 * \param[in] shorthand   The shorthand symbol for the option
	 * \param[in] symbol      The outwritten symbol for the option
	 * \param[in] needs_value Indicate whether the option requires a value
	 * \param[in] default_arg Default argument as a string
	 * \param[in] desc        Option description
	 */
	Option(const char shorthand, const std::string &symbol,
		const bool needs_value, const std::string &default_arg,
		const std::string &desc);

	/**
	 * \brief Constructor
	 *
	 * \param[in] symbol      The outwritten symbol for the option
	 * \param[in] needs_value Indicate whether the option requires a value
	 * \param[in] default_arg Default argument as a string
	 * \param[in] desc        Option description
	 */
	Option(const std::string &symbol, const bool needs_value,
		   const std::string &default_arg, const std::string &desc)
		: Option ('\0', symbol, needs_value, default_arg, desc )
	{ /* empty */ }

	/**
	 *  \brief Shorthand symbol of this option or '\0' if none.
	 *
	 * \return Shorthand symbol of this option or '\0' if none
	 */
	char shorthand_symbol() const;

	/**
	 * \brief Symbol of this option.
	 *
	 * A symbol may never be empty.
	 *
	 * \return Symbol of this option or empty string if none.
	 */
	std::string symbol() const;

	/**
	 *  \brief Returns TRUE iff the option requires a value
	 *
	 * \return TRUE iff the option requires a value, otherwise FALSE
	 */
	bool needs_value() const;

	/**
	 * \brief Default value of the option
	 *
	 * \return Default value of the option
	 */
	std::string default_arg() const;

	/**
	 * \brief Description of the symbol
	 */
	std::string description() const;

	/**
	 * \brief Return command line tokens that represent that option
	 *
	 * \return List of tokens
	 */
	std::vector<std::string> tokens() const;

	/**
	 * \brief Return the list of tokens as a comma separated list
	 *
	 * \return List of tokens
	 */
	std::string tokens_str() const;

private:

	const char        shorthand_;
	const std::string symbol_;
	const bool        needs_value_;
	const std::string default_;
	const std::string description_;
};


/**
 * \brief Base class for configuration options.
 *
 * An Options object contains the complete configuration information for an
 * ARApplication.
 */
class Options
{
public:

	/**
	 * \brief Default constructor.
	 */
	Options();

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~Options() noexcept;

	/**
	 * \brief Set or unset the version flag.
	 *
	 * The version flag indicates whether the option VERSION was passed.
	 *
	 * \param[in] version The flag to set or unset
	 */
	void set_version(const bool &version);

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
	bool is_set(const OptionValue &option) const;

	/**
	 * \brief Set the option to TRUE.
	 *
	 * If the option is currently set, the call has no effect.
	 *
	 * \param[in] option The option to be set to TRUE
	 */
	void set(const OptionValue &option);

	/**
	 * \brief Set the option to FALSE.
	 *
	 * If the option is currently unset, the call has no effect.
	 *
	 * \param[in] option The option to be set to FALSE
	 */
	void unset(const OptionValue &option);

	/**
	 * \brief Get option value by key.
	 *
	 * \param[in] option The option whose value to get
	 *
	 * \return The value of the option passed
	 */
	std::string get(const OptionValue &option) const;

	/**
	 * \brief Set option value for key.
	 *
	 * \param[in] option The option to put in
	 * \param[in] value  The value for the option to put in
	 */
	void put(const OptionValue &option, const std::string &value);

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
	std::string const get_argument(const OptionValue &i) const;

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

	/**
	 * Position of the leftmost set flag.
	 *
	 * \return Position of the leftmost flag that is set.
	 */
	OptionValue leftmost_flag() const;

	/**
	 * Position of the rightmost set flag.
	 *
	 * \return Position of the rightmost flag that is set.
	 */
	OptionValue rightmost_flag() const;

private:

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
	OptionValue config_;

	/**
	 * \brief Valued options' values
	 */
	std::map<OptionValue, std::string> option_map_;

	/**
	 * \brief Arguments (non-option values)
	 */
	std::vector<std::string> arguments_;

};

} // namespace arcsapp

#endif

