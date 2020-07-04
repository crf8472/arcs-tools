#ifndef __ARCSTOOLS_CLITOKENS_HPP__
#define __ARCSTOOLS_CLITOKENS_HPP__

/**
 * \file
 *
 * \brief A very simple command line parser.
 *
 * This parser distinguishes arguments, boolean (value-less) options and options
 * requiring values. The caller just pull-consumes those tokens for the rules
 * defined in the application. If all tokens can be consumed by this method, the
 * input is syntactically valid.
 */

#include <cstdint>       // for uint64_t
#include <stdexcept>     // for runtime_error
#include <string>
#include <tuple>
#include <vector>
#include <deque>

namespace arcsapp
{

/**
 * \brief ID for a supported option
 */
using OptionCode = uint64_t;


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
	 * \brief Option Id for non-options.
	 */
	static constexpr OptionCode NONE = 0;

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
 * \brief Represents command line input as options (w/o values) and arguments.
 *
 * The callers responsibility is to consume everything legal and if this
 * succeeds without errors, empty() must be TRUE thereafter. If it
 * returns TRUE instead, the command line call was not syntactically wellformed.
 * Thus, with the API of this class, complete syntax check of the CLI input is
 * possible.
 */
class CLIInput final
{
public:

	/**
	 * \brief Constructor for token view on the command line input.
	 *
	 * \param[in] argc      Number of command line arguments
	 * \param[in] argv      Command line arguments
	 * \param[in] supported Supported options
	 * \param[in] preserve_order TRUE if order of occurrences is to be preserved
	 */
	CLIInput(const int argc, const char* const * const argv,
			const std::vector<std::pair<Option, OptionCode>> &supported,
			const bool preserve_order);

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~CLIInput() noexcept;

	/**
	 * \brief The i-th input option.
	 *
	 * If \c option_code(i) is Option::NONE, \c option_value(i) is an argument.
	 * Otherwise, \c option_value(i) is the value for option \c i.
	 *
	 * \return Option \c i.
	 */
	OptionCode option_code(const int i) const;

	/**
	 * \brief Access value for i-ith input option.
	 *
	 * If \c option_code(i) is Option::NONE, \c option_value(i) is an argument.
	 * Otherwise, \c option_value(i) is the value for option \c i.
	 *
	 * \return Value for option \c i.
	 */
	const std::string& option_value(const int i) const;

	/**
	 * \brief Returns TRUE iff the instance does not hold any input tokens.
	 *
	 * \return TRUE iff the instance does not hold any input tokens
	 */
	bool empty() const;

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
	 * If \c contains(code) is \c FALSE, the result will be an empty string.
	 *
	 * \param[in] option The option to get the value for
	 *
	 * \return Parsed value for the specified code
	 */
	const std::string& value(const OptionCode &option) const;

private:

	/**
	 * \brief Description.
	 *
	 * \param[in] opt       The option symbol to consume
	 * \param[in] val       The option value to consume
	 * \param[in] supported The supported options to match
	 * \param[in,out] pos   Character position in the call string
	 */
	void consume_as_symbol(const char * const opt, const char * const val,
			const std::vector<std::pair<Option, OptionCode>> supported,
			int &pos);

	/**
	 * \brief Description.
	 *
	 * \param[in] opt       The option symbol to consume
	 * \param[in] val       The option value to consume
	 * \param[in] supported The supported options to match
	 * \param[in,out] pos   Character position in the call string
	 */
	void consume_as_shorthand(const char * const opt, const char * const val,
			const std::vector<std::pair<Option, OptionCode>> supported,
			int &pos);

	/**
	 * \brief Returns an empty option value.
	 *
	 * Convenience: do not have to create empty string objects while parsing.
	 *
	 * \return An empty option value
	 */
	const std::string& empty_value() const noexcept;

	/**
	 * \brief An option with or without a value or an argument.
	 *
	 * An argument is represented as Option with code 0.
	 */
	class InputItem
	{
	public:

		/**
		 * \brief Construct item with specified code.
		 */
		InputItem(const OptionCode id)
			: id_ { id }
			, value_ { std::string{} }
		{ /* empty */ };

		/**
		 * \brief Construct item with specified content.
		 */
		InputItem(const std::string &value)
			: id_ { Option::NONE }
			, value_ { value }
		{ /* empty */ };

		OptionCode id() const { return id_; }

		void set_value(const std::string &value) { value_ = value; }

		const std::string& value() const { return value_; }

	private:

		OptionCode id_;
		std::string value_;
	};

	/**
	 * \brief Returns the specified option token, if present, otherwise nullptr.
	 *
	 * \param[in] option The option to lookup
	 *
	 * \return The specified option token, if present, otherwise nullptr.
	 */
	const InputItem* lookup(const OptionCode &option) const;

private:

	/**
	 * \brief Parsed items.
	 */
	std::vector<InputItem> items_;

public:

	using size_type = decltype( items_ )::size_type;

	/**
	 * \brief Return number of input tokens.
	 *
	 * May differ from \c argc.
	 *
	 * \return Number of input tokens.
	 */
	size_type size() const;

	using iterator = decltype( items_ )::iterator;

	using const_iterator = decltype( items_ )::const_iterator;

	iterator begin();
	iterator end();
	const_iterator begin() const;
	const_iterator end() const;
	const_iterator cbegin() const;
	const_iterator cend() const;
};


/**
 * \brief Represents command line input as options and arguments.
 *
 * The callers responsibility is to consume everything legal and if this
 * succeeds without errors, empty() must be TRUE thereafter. If it
 * returns TRUE instead, the command line call was not syntactically wellformed.
 * Thus, with the API of this class, complete syntax check of the CLI input is
 * possible.
 */
class CLITokens
{
public:

	/**
	 * \brief Constructor for token view on the command line input.
	 *
	 * \param[in] argc Number of command line arguments
	 * \param[in] argv Command line arguments
	 */
	CLITokens(const int argc, const char* const * const argv);

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~CLITokens() noexcept;

	/**
	 * \brief Consume the token passed and return its parameter if it is
	 * available.
	 *
	 * The option token will be deleted from the token sequence, if present.
	 *
	 * The first element of the result tuple is TRUE iff the token was
	 * successfully consumed/found. The second element will contain the value
	 * of the option, if a value is required by the option and present on the
	 * command line, otherwise an empty string.
	 *
	 * It is in the responsibility of the caller to check for syntactic
	 * correctness, i.e. whether a valued option indeed has a non-empty value.
	 *
	 * \param[in] token           The option token to consume
	 * \param[in] value_requested Indicate whether a value should be consumed
	 *
	 * \return Found flag and value string, empty if not found
	 */
	std::tuple<bool, std::string> consume(const std::string &token,
			const bool value_requested) noexcept;

	/**
	 * \brief Consume next token if it is available and return its value.
	 *
	 * \return The token if any, otheriwse an empty string
	 */
	const std::string consume() noexcept;

	/**
	 * \brief Test whether a certain token is present.
	 *
	 * Does not alter the token sequence.
	 *
	 * \param[in] token The token to test for
	 *
	 * \return TRUE iff the token is present in the token set, otherwise FALSE
	 */
	bool contains(const std::string &token) const noexcept;

	/**
	 * \brief Returns TRUE if there are no unconsumed tokens.
	 *
	 * \return TRUE iff there are no unconsumed tokens, otherwise FALSE
	 */
	bool empty() noexcept;

	/**
	 * \brief Returns  tokens not yet consumed to stdout.
	 *
	 * The order of the occurrence in the input is preserved.
	 *
	 * If empty() is TRUE, the result is empty.
	 *
	 * \return The tokens not consumed so far
	 */
	const std::vector<std::string> unconsumed() noexcept;

	/**
	 * \brief Returns an empty option value.
	 *
	 * Convenience: do not have to create empty string objects while parsing.
	 *
	 * \return An empty option value
	 */
	const std::string& empty_value() noexcept;

private:

	/**
	 * \brief All tokens entered in the CLI.
	 */
	std::deque<std::string> tokens_;
};

} // namespace arcsapp

#endif

