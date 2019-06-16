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

/**
 * \brief Parses command line input as boolean options, valued options and
 * arguments.
 *
 * It is not necessary to define the legal options. Just try to consume
 * everything legal and if this succeeds without errors, call tokens_left()
 * thereafter. If it returns true, the command line call was not wellformed.
 * Thus, with the API of this class, complete syntax check of the CLI input is
 * possible.
 */
class CLIParser
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
	CLIParser(int argc, char **argv);

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~CLIParser() noexcept;

	/**
	 * \brief Consume the argument if it is available and return its value.
	 *
	 * \return The argument if any
	 */
	const std::string consume_argument();

	/**
	 * \brief Consume the option passed and return its parameter if it is
	 * available.
	 *
	 * The option will be deleted from the token set, if present.
	 *
	 * If the option is not present, the default value will be returned.
	 *
	 * \param[in] option The option to consume
	 *
	 * \return The value for the option passed or an empty string
	 */
	std::tuple<bool, std::string> consume_valued_option(
			const std::string &option);

	/**
	 * \brief Test whether a certain option is present.
	 *
	 * Does not alter the token set.
	 *
	 * \param[in] option The option to test for
	 *
	 * \return TRUE iff the option string is present in the token set
	 */
	bool option_present(const std::string &option);

	/**
	 * \brief Consume the boolean option and return its value.
	 *
	 * If the option is not present, the default value will be returned.
	 *
	 * \param[in] option The option to consume
	 *
	 * \return The value for the option passed
	 */
	bool consume_option(const std::string &option);

	/**
	 * \brief Returns true if there are any CLI tokens left that were not
	 * consumed.
	 *
	 * \return TRUE iff there are any tokens not consumed
	 */
	bool tokens_left();

	/**
	 * \brief Returns all tokens not yet consumed to stdout.
	 *
	 * \return The tokens not consumed so far
	 */
	const std::vector<std::string> get_unconsumed_tokens();


private:

	/**
	 * \brief All tokens entered in the CLI.
	 */
	std::deque<std::string> tokens_;
};

#endif

