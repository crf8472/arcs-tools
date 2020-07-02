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

#include <cstdint>  // for uint64_t
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

