#ifndef __ARCSTOOLS_CLITOKENS_MOCKS_HPP__
#define __ARCSTOOLS_CLITOKENS_MOCKS_HPP__

#include <string>  // for string
#include <vector>  // for vector

#ifndef __ARCSTOOLS_CLITOKENS_HPP__
#include "clitokens.hpp"
#endif


using arcsapp::OptionCode;
using arcsapp::OptionRegistry;


/**
 * \brief Uniform representation of an input token.
 *
 * A Token can be an argument or an option with or without a value.
 *
 * An argument is represented as Option with code ARGUMENT.
 */
class Token final
{
public:

	/**
	 * \brief Construct token with specified code and value.
	 *
	 * \param[in] code  Code for this Token
	 * \param[in] value Value for this Token
	 */
	Token(const OptionCode code, const std::string& value) noexcept
		: code_  { code }
		, value_ { value }
	{ /* empty */ }

	/**
	 * \brief OptionCode of the Token.
	 *
	 * \return OptionCode of the Token
	 */
	OptionCode code() const noexcept { return code_; }

	/**
	 * \brief Get the value of the Token.
	 *
	 * \return Value of the Token
	 */
	const std::string& value() const noexcept { return value_; }

private:

	OptionCode code_;

	std::string value_;
};


/**
 * \brief Get all commandline input tokens.
 *
 * The returned list will contain the tokens in the same order as the occurred
 * in the input. The name of the binary will not be contained in the list.
 *
 * \param[in] argc           Number of command line arguments
 * \param[in] argv           Command line arguments
 * \param[in] supported      Supported options
 *
 * \return List of command line input tokens
 */
inline std::vector<Token> get_tokens(const int argc,
		const char* const * const argv,
		const OptionRegistry& supported)
{
	std::vector<Token> tokens;
	tokens.reserve(12);
	const auto append_token =
		[&tokens](const OptionCode c, const std::string& v)
		{
			tokens.emplace_back(c, v);
		};
	arcsapp::input::parse(argc, argv, supported, append_token);
	return tokens;
}

#endif

