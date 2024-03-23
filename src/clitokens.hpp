#ifndef __ARCSTOOLS_CLITOKENS_HPP__
#define __ARCSTOOLS_CLITOKENS_HPP__

/**
 * \file
 *
 * \brief A simple command line pull-parser.
 *
 * This parser distinguishes arguments, boolean (value-less) options and options
 * requiring values. The caller just pull-consumes those tokens for the rules
 * defined in the application. If all tokens can be consumed by this method, the
 * input is syntactically valid.
 */

#include <cstdint>       // for uint64_t
#include <functional>    // for function
#include <limits>        // for numeric_limits
#include <stdexcept>     // for runtime_error
#include <string>        // for string
#include <utility>       // for pair
#include <vector>        // for vector


namespace arcsapp
{

/**
 * \brief Type for the unique id of a supported option
 *
 * The OptionCode is the token to which an input option is parsed. While an
 * input option can be represented by either a symbol or its shorthand version,
 * the OptionCode is a unique identifier for this option after parsing.
 */
using OptionCode = uint64_t;


class Option;
bool operator == (const Option &lhs, const Option &rhs) noexcept;

/**
 * \brief Descriptor for a single command line option.
 *
 * An Option has a symbol (e.g. '--print-all', '--boolean') and may or may not
 * have a shorthand symbol (e.g. '-p', '-b'). It may or may not expect a value
 * and most options have some default value. Options that do not expect a value
 * are also called 'boolean'. An option has an additional short description that
 * can be printed in a usage or help message.
 *
 * To options qualify as equal, eg. <tt>option1 == option2</tt> iff their
 * symbols, shorthand symbols, value requirements and their default argument is
 * equal. Their description is allowed to differ.
 */
class Option final
{
public:

	friend bool operator == (const Option &lhs, const Option &rhs) noexcept;

	/**
	 * \brief Constructor for options with shorthand and symbol.
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
	 * \brief Constructor for options with symbol only.
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
	const char& shorthand_symbol() const;

	/**
	 * \brief Symbol of this option.
	 *
	 * A symbol may never be empty.
	 *
	 * \return Symbol of this option or empty string if none.
	 */
	const std::string& symbol() const;

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
	const std::string& default_arg() const;

	/**
	 * \brief Description of the symbol
	 */
	const std::string& description() const;

	/**
	 * \brief Return the list of associated tokens as a comma separated list.
	 *
	 * \return List of tokens
	 */
	std::string tokens_str() const;

private:

	/**
	 * \brief Internal shorthand symbol.
	 */
	char shorthand_;

	/**
	 * \brief Internal symbol.
	 */
	std::string symbol_;

	/**
	 * \brief Flag to indicate whether the option requires a value.
	 */
	bool needs_value_;

	/**
	 * \brief Default argument, if any.
	 */
	std::string default_arg_;

	/**
	 * \brief Short description.
	 *
	 * Can be printed in usage message for example.
	 */
	std::string description_;
};


/**
 * \brief Type to associate actual OptionCodes with Option objects.
 *
 * The contained type must be a pair with the OptionCode as first element and
 * the Option as second element. The container must allow iteration.
 */
using OptionRegistry = std::vector<std::pair<OptionCode, Option>>;
// FIXME This definition is repeated in config.hpp


/**
 * \brief Parse the command line input.
 */
namespace input
{


/**
 * \brief Option values to check for.
 */
struct OP_VALUE
{
	static constexpr const char* NONE        = "none";     // no value
	static constexpr const char* USE_DEFAULT = "default";  // existing default
	static constexpr const char* FALSE       = "FALSE";    // false
	static constexpr const char* TRUE        = "TRUE";     // true
	static constexpr const char* AUTO        = "auto";     // auto assign
};


/**
 * \brief Reports a syntax error on parsing the command line input.
 *
 * This exception may occurr for the following reasons:
 *   - An invalid option is passed.
 *   - A non-boolean option without its expected value is passed.
 *   - A boolean option is passed a value (e.g. as '-t0').
 */
class CallSyntaxException : public std::runtime_error
{
public:

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~CallSyntaxException() noexcept = default;

	/**
	 * \brief Constructor
	 *
	 * \param[in] what_arg What-Message
	 */
	CallSyntaxException(const std::string &what_arg);
};


/**
 * \brief OptionCode representing an argument.
 */
inline constexpr OptionCode ARGUMENT = 0;

/**
 * \brief OptionCode representing a single dash '-'.
 */
inline constexpr OptionCode DASH  = std::numeric_limits<OptionCode>::max() - 0;

/**
 * \brief OptionCode representing a double dash '--'.
 */
inline constexpr OptionCode DDASH = std::numeric_limits<OptionCode>::max() - 1;


/**
 * \brief Type of callback function pointer for notifying about parsed options.
 *
 * Parameter of this type is called by parse() whenever a token is parsed.
 */
using option_callback =
			std::function<void(const OptionCode, const std::string&)>;


/**
 * \brief Parses command line input in a POSIX-style.
 *
 * Parses the argc/argv command line input to a sequence of input tokens. A
 * token is either an option along with its respective value or a non-option
 * (i.e. an argument). Arguments are represented as options of type
 * ARGUMENT.
 *
 *  - Syntactically, an option is a double hyphen followed by a sequence
 *    of alphanumeric characters and hyphens like this: <tt>\-\-my\-option</tt>.
 *    The sequence is only allowed to contain single hyphens surrounded by
 *    alphanumeric characters. Other non-alphanumeric characters are forbidden.
 *  - Alternatively, an option is a single hyphen '\-' followed by a single
 *    alphanumeric character, like this: <tt>\-v</tt>.
 *  - An option may require a value. If a value is expected, it must appear
 *    immediately after the option token. A blank ' ' may or may not separate
 *    the option from its value. Example: <tt>\-i value</tt> or
 *    <tt>\-ivalue</tt> <tt>\-\-my\-option value</tt>.
 *  - The variant starting with a double hyphen may separate its value by a
 *    an 'equals' character like in <tt>\-\-my\-option=value</tt>.
 *  - Options that do not require values can be grouped after a single hyphen,
 *    so, for example, <tt>\-tbn</tt> is equivalent to <tt>\-t \-b \-n</tt>.
 *  - Options can appear in any order, thus <tt>\-tbn</tt> is equivalent to
 *    <tt>\-ntb</tt>.
 *  - The same option may or may not appear multiple times.
 *  - Options typically precede other nonoption arguments:
 *    <tt>\-ltr nonoption</tt>.
 *  - The '\-\-' argument terminates options. What follows thereafter is parsed
 *    as arguments.
 *  - The '\-' argument is accepted but not assigned any semantics.
 *
 * This intends to obey the POSIX conventions.
 *
 * Function parse() will never modify any command line input, neither will
 * tokens be erased nor added. Function parse() does not perform any semantic
 * validation of the input.
 *
 * \param[in] argc           Number of command line arguments
 * \param[in] argv           Command line arguments
 * \param[in] supported      Supported options
 * \param[in] pass_token     Function to call on each parsed token
 */
void parse(const int argc, const char* const * const argv,
		const OptionRegistry &supported, const option_callback& pass_token);

} // namespace input

} // namespace arcsapp

#endif

