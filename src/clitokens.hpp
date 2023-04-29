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

#include <cstddef>       // for size_t
#include <cstdint>       // for uint64_t
#include <functional>    // for function
#include <stdexcept>     // for runtime_error
#include <string>        // for string
#include <utility>       // for pair
#include <vector>        // for vector


namespace arcsapp
{

/**
 * \brief Unique id of a supported option
 */
using OptionCode = uint64_t;


class Option;
bool operator == (const Option &lhs, const Option &rhs) noexcept;

/**
 * \brief Descriptor for a single command line option.
 *
 * An Option has a symbol (e.g. '--print-all', '--boolean') and may or may not
 * have a shorthand symbol (e.g. '-r'). It may or may not expect a value and
 * most options have some default. Options that do not expect a value are also
 * called 'boolean'. An option has an additional short description that
 * can be printed in a usage or help message.
 */
class Option
{
public:

	friend bool operator == (const Option &lhs, const Option &rhs) noexcept;

	/**
	 * \brief Option Id for non-options.
	 */
	static constexpr OptionCode NONE = 0;

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

	/**
	 * \brief Internal shorthand symbol.
	 */
	const char        shorthand_;

	/**
	 * \brief Internal symbol.
	 */
	const std::string symbol_;

	/**
	 * \brief Flag to indicate whether the option requires a value.
	 */
	const bool        needs_value_;

	/**
	 * \brief Default argument, if any.
	 */
	const std::string default_arg_;

	/**
	 * \brief Short description.
	 *
	 * Can be printed in usage message for example.
	 */
	const std::string description_;
};


/**
 * \brief Reports a syntax error on parsing the command line input.
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


namespace input
{

/**
 * \brief Uniform representation of an input token.
 *
 * A Token can be an argument or an option with or without a value.
 *
 * An argument is represented as Option with code Option::NONE.
 */
class Token
{
public:

	/**
	 * \brief Construct item with specified code and value.
	 *
	 * \param[in] code  Code for this Token
	 * \param[in] value Value for this Token
	 */
	Token(const OptionCode code, const std::string &value)
		: code_  { code }
		, value_ { value }
	{ /* empty */ }

	/**
	 * \brief Construct option item with specified code.
	 *
	 * \param[in] code Code for this option
	 */
	Token(const OptionCode code)
		: Token { code, std::string{} }
	{ /* empty */ };

	/**
	 * \brief Construct argument item with specified value.
	 *
	 * \param[in] value Argument value
	 */
	Token(const std::string &value)
		: Token { Option::NONE, value }
	{ /* empty */ };

	/**
	 * \brief OptionCode of the Token.
	 *
	 * \return OptionCode of the Token
	 */
	OptionCode code() const { return code_; }

	/**
	 * \brief Set the value of the Token.
	 *
	 * \param[in] value New value of the Token
	 */
	void set_value(const std::string &value) { value_ = value; }

	/**
	 * \brief Get the value of the Token.
	 *
	 * \return Value of the Token
	 */
	const std::string& value() const noexcept { return value_; }

	/**
	 * \brief Returns an empty option value.
	 *
	 * Convenience: do not have to create empty string objects while parsing.
	 *
	 * \return An empty option value
	 */
	static const std::string& empty_value() noexcept
	{
		static const auto empty_string = std::string{};
		return empty_string;
	}

private:

	OptionCode code_;

	std::string value_;
};


/**
 * \brief Get all CLI input tokens.
 *
 * \param[in] argc           Number of command line arguments
 * \param[in] argv           Command line arguments
 * \param[in] supported      Supported options
 *
 * \return List of tokens
 */
std::vector<Token> get_tokens(const int argc,
		const char* const * const argv,
		const std::vector<std::pair<Option, OptionCode>> &supported);


/**
 * \brief Type of callback function pointer for notifying about parsed options.
 *
 * Called by parse() whenever a token is parsed.
 */
using option_callback =
			std::function<void(const OptionCode, const std::string&)>;


/**
 * \brief Consume all tokens.
 *
 * The returned list will contain the tokens in the same order as the occurred
 * in the input.
 *
 * \param[in] argc           Number of command line arguments
 * \param[in] argv           Command line arguments
 * \param[in] supported      Supported options
 * \param[in] pass_token     Function to call on each parsed token
 */
void parse(const int argc, const char* const * const argv,
		const std::vector<std::pair<Option, OptionCode>> &supported,
		const option_callback& pass_token);


/**
 * \brief Consume input chars as an option symbol.
 *
 * \param[in] opt         The option symbol to consume
 * \param[in] val         The option value to consume
 * \param[in] supported   The supported options to match
 * \param[in,out] pos     Character position in the call string
 * \param[in] pass_token  Function to call on each parsed token
 */
void consume_as_symbol(const char * const opt, const char * const val,
		const std::vector<std::pair<Option, OptionCode>>& supported, int &pos,
		const option_callback& pass_token);


/**
 * \brief Consume input chars as an option shorthand symbol.
 *
 * \param[in] opt        The option symbol to consume
 * \param[in] val        The option value to consume
 * \param[in] supported  The supported options to match
 * \param[in,out] pos    Character position in the call string
 * \param[in] pass_token Function to call on each parsed token
 */
void consume_as_shorthand(const char * const opt,
		const char * const val,
		const std::vector<std::pair<Option, OptionCode>>& supported, int &pos,
		const option_callback& pass_token);

} // namespace input


/**
 * \brief Parses command line input in a POSIX-style.
 *
 * Parses the argc/argv command line input to a sequence of input tokens. A
 * token is either an option along with its respective value or a non-option
 * (i.e. an argument). Arguments are represented as options of type
 * Option::NONE.
 *
 *  - Syntactically, an option is a hyphen '-' followed by a single alphanumeric
 *    character, like this: <tt>-v</tt>.
 *  - An option may require a value. If a value is expected, it must appear
 *    immediately after the option token. A blank ' ' may or may not separate
 *    the option from its value. Example: <tt>-i argument</tt> or
 *    <tt>-iargument</tt>.
 *  - Options that do not require values can be grouped after a single hyphen,
 *    so, for example, <tt>-tbn</tt> is equivalent to <tt>-t -b -n</tt>.
 *  - Options can appear in any order, thus -tbn is equivalent to <tt>-ntb</tt>.
 *  - Options can appear multiple times.
 *  - Options typically precede other nonoption arguments:
 *    <tt>-ltr nonoption</tt>.
 *  - The '--' argument terminates options.
 *
 * This intends to obey the POSIX conventions.
 *
 * CLITokens will never modify any command line input, neither will tokens be
 * erased nor added. CLITokens does not perform any semantic validation of the
 * input.
 */
class CLITokens final
{
	using Token = input::Token;

	input::option_callback option_;

	/**
	 * \brief Parsed items.
	 */
	std::vector<Token> tokens_;

public:

	/**
	 * \brief Size type of CLITokens.
	 */
	using size_type = decltype( tokens_ )::size_type;

	/**
	 * \brief Iterator type of CLITokens.
	 */
	using iterator = decltype( tokens_ )::iterator;

	/**
	 * \brief Const iterator type of CLITokens.
	 */
	using const_iterator = decltype( tokens_ )::const_iterator;

	CLITokens();

	/**
	 * \brief Constructor for token view on the command line input.
	 *
	 * The supported options are represented as a sequence of pairs of the
	 * actual option and the numerical code for this option.
	 *
	 * The caller is responsible that \c argc is the precise number of
	 * elements in \c argv.
	 *
	 * \param[in] argc           Number of command line arguments
	 * \param[in] argv           Command line arguments
	 * \param[in] supported      Supported options
	 */
	CLITokens(const int argc, const char* const * const argv,
			const std::vector<std::pair<Option, OptionCode>> &supported);

	/**
	 * \brief Default destructor.
	 */
	~CLITokens() noexcept;

	void register_callback(const input::option_callback f);

	/**
	 * \brief Constructor for token view on the command line input.
	 *
	 * The supported options are represented as a sequence of pairs of the
	 * actual option and the numerical code for this option.
	 *
	 * The caller is responsible that \c argc is the precise number of
	 * elements in \c argv.
	 *
	 * \param[in] argc           Number of command line arguments
	 * \param[in] argv           Command line arguments
	 * \param[in] supported      Supported options
	 */
	void parse(const int argc, const char* const * const argv,
			const std::vector<std::pair<Option, OptionCode>> &supported);

	/**
	 * \brief Returns TRUE iff an option with this code is contained.
	 *
	 * \param[in] option The code to query the instance for
	 *
	 * \return TRUE iff an option with this code is contained
	 */
	bool contains(const OptionCode &option) const noexcept;

	/**
	 * \brief Returns the parsed value for the specified code.
	 *
	 * If \c contains(option) is \c FALSE or \c option is \c Option::NONE, the
	 * result will be an empty string.
	 *
	 * \param[in] option The option to get the value for
	 *
	 * \return Parsed value for the specified code
	 */
	const std::string& value(const OptionCode &option) const;

	/**
	 * \brief The i-th input option.
	 *
	 * If \c option_code(i) is Option::NONE then \c option_value(i) is an
	 * argument. Otherwise, \c option_value(i) is the value for option \c i.
	 *
	 * \return Option \c i.
	 */
	OptionCode option_code(const size_type i) const;

	/**
	 * \brief Access value for i-ith input option.
	 *
	 * If \c option_code(i) is Option::NONE then \c option_value(i) is an
	 * argument. Otherwise, \c option_value(i) is the value for option \c i.
	 *
	 * \return Value for option \c i.
	 */
	const std::string& option_value(const size_type i) const;

	/**
	 * \brief Returns the i-th argument (where 0 is the first argument).
	 *
	 * If the total number of arguments is less than \c i, an empty
	 * string will be returned. If \c i is greater or equal than the total
	 * number of arguments, or \c i is 0, an empty string is returned.
	 *
	 * \return Argument with index \c i or empty string if \c i >= size().
	 */
	const std::string& argument(const size_type &i) const;

	/**
	 * \brief Return number of input tokens.
	 *
	 * May differ from \c argc.
	 *
	 * \return Number of input tokens.
	 */
	size_type size() const;

	/**
	 * \brief Returns TRUE iff the instance does not hold any input tokens.
	 *
	 * \return TRUE iff the instance does not hold any input tokens
	 */
	bool empty() const;

	iterator begin();
	iterator end();
	const_iterator begin() const;
	const_iterator end() const;
	const_iterator cbegin() const;
	const_iterator cend() const;

private:

	/**
	 * \brief Returns the specified option token, if present, otherwise nullptr.
	 *
	 * \param[in] option The option to lookup
	 *
	 * \return The specified option token, if present, otherwise nullptr.
	 */
	const Token* lookup(const OptionCode &option) const;

	/**
	 * \brief Get i-th token.
	 *
	 * \param[in] i Index of the token
	 *
	 * \return i-ith input Token
	 */
	const Token* get(const size_type i) const;
};

} // namespace arcsapp

#endif

