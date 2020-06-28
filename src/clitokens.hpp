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

#include <string>
#include <tuple>
#include <vector>
#include <deque>

namespace arcsapp
{

/**
 * \brief Represents command line input as options and arguments.
 *
 * The callers responsibility is to consume everything legal and if this
 * succeeds without errors, tokens_left() must be FALSE thereafter. If it
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
	CLITokens(int argc, const char* const * const argv);

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
	std::tuple<bool, std::string> consume_option_token(
			const std::string &token, const bool value_requested) noexcept;

	/**
	 * \brief Consume an argument if it is available and return its value.
	 *
	 * \return The argument if any, otheriwse an empty string
	 */
	const std::string consume_argument() noexcept;

	/**
	 * \brief Test whether a certain token is present.
	 *
	 * Does not alter the token sequence.
	 *
	 * \param[in] token The token to test for
	 *
	 * \return TRUE iff the token is present in the token set, otherwise FALSE
	 */
	bool token_present(const std::string &token) noexcept;

	/**
	 * \brief Returns TRUE if there are any unconsumed tokens.
	 *
	 * \return TRUE iff there are any unconsumed tokens, otherwise FALSE
	 */
	bool tokens_left() noexcept;

	/**
	 * \brief Returns  tokens not yet consumed to stdout.
	 *
	 * The order of the occurrence in the input is preserved.
	 *
	 * If tokens_left() is FALSE, the result is empty.
	 *
	 * \return The tokens not consumed so far
	 */
	const std::vector<std::string> unconsumed_tokens() noexcept;

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

