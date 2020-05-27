#ifndef __ARCSTOOLS_CLIPARSE_HPP__
#define __ARCSTOOLS_CLIPARSE_HPP__

/**
 * \file
 *
 * \brief A very simple command line parser.
 *
 * This parser puts every command line token in an aggregate. It distinguishes
 * arguments, pure (value-less) options and options having values. The user
 * just consumes those tokens for the rules defined in the application. If all
 * tokens can be consumed by this method, the input is valid.
 */

#include <string>
#include <tuple>
#include <vector>
#include <deque>

namespace arcsapp
{

/**
 * \brief Parses command line input as boolean options, valued options and
 * arguments.
 *
 * It is not necessary to define the legal options. Just try to consume
 * everything legal and if this succeeds without errors, tokens_left() must be
 * FALSE thereafter. If it returns TRUE instead, the command line call was not
 * syntactically wellformed. Thus, with the API of this class, complete syntax
 * check of the CLI input is possible.
 */
class CLITokens
{
public:

	/**
	 * \brief Construct parser for command line input.
	 *
	 * Assumes argc to be the number of null-terminated character sequences
	 * in argv.
	 *
	 * \param[in] argc Number of command line arguments
	 * \param[in] argv Command line arguments
	 */
	CLITokens(int argc, char **argv);

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~CLITokens() noexcept;

	/**
	 * \brief Consume the option passed and return its parameter if it is
	 * available.
	 *
	 * The option token will be deleted from the token set, if present.
	 *
	 * The first element of the result tuple is TRUE iff the token was
	 * successfully consumed/found. The second element will contain the value
	 * of the option, if a value is required by the option and present on the
	 * command line, otherwise an empty string.
	 *
	 * It is in the responsibility of the caller to check for syntactic
	 * correctness, i.e. whether a valued option indeed has a non-empty value.
	 *
	 * The function will not throw;
	 *
	 * \param[in] token           The option token to consume
	 * \param[in] value_requested Indicate whether a value should be parsed
	 *
	 * \return The value for the option passed or an empty string
	 */
	std::tuple<bool, std::string> consume_option_token(
			const std::string &token, const bool value_requested) noexcept;

	/**
	 * \brief Consume the argument if it is available and return its value.
	 *
	 * \return The argument if any
	 */
	const std::string consume_argument() noexcept;

	/**
	 * \brief Test whether a certain option is present.
	 *
	 * Does not alter the token set.
	 *
	 * \param[in] token The option to test for
	 *
	 * \return TRUE iff the option string is present in the token set
	 */
	bool token_present(const std::string &token) noexcept;

	/**
	 * \brief Returns true if there are any CLI tokens left that were not
	 * consumed.
	 *
	 * \return TRUE iff there are any tokens not consumed
	 */
	bool tokens_left() noexcept;

	/**
	 * \brief Returns all tokens not yet consumed to stdout.
	 *
	 * \return The tokens not consumed so far
	 */
	const std::vector<std::string> unconsumed_tokens() noexcept;

	/**
	 * \brief Returns an empty option value.
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

