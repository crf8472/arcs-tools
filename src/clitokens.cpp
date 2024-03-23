#ifndef __ARCSTOOLS_CLITOKENS_HPP__
#include "clitokens.hpp"
#endif

#include <cstring>    // for strncmp
#include <sstream>    // for ostringstream
#include <stdexcept>  // for runtime_error
#include <string>     // for string

#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif


namespace arcsapp
{

// Option


OptionCode NONE;


Option::Option(const char shorthand, const std::string &symbol,
		const bool needs_value, const std::string &default_arg,
		const std::string &desc)
	: shorthand_   { shorthand }
	, symbol_      { symbol }
	, needs_value_ { needs_value }
	, default_arg_ { default_arg }
	, description_ { desc }
{
	// empty
}


const char& Option::shorthand_symbol() const
{
	return shorthand_;
}


const std::string& Option::symbol() const
{
	return symbol_;
}


bool Option::needs_value() const
{
	return needs_value_;
}


const std::string& Option::default_arg() const
{
	return default_arg_;
}


const std::string& Option::description() const
{
	return description_;
}


std::string Option::tokens_str() const
{
	std::ostringstream oss;

	if (this->shorthand_symbol() != '\0')
	{
		oss << '-' << this->shorthand_symbol() << ",";
	}
	if (!this->symbol().empty())
	{
		oss << "--" << this->symbol();
	}

	return oss.str();
}


bool operator == (const Option &lhs, const Option &rhs) noexcept
{
	return lhs.symbol() == rhs.symbol()
		&& lhs.shorthand_symbol() == rhs.shorthand_symbol()
		&& lhs.needs_value() == rhs.needs_value()
		&& lhs.default_arg() == rhs.default_arg();
}


namespace input
{


// CallSyntaxException


CallSyntaxException::CallSyntaxException(const std::string &what_arg)
	: std::runtime_error { what_arg }
{
	// empty
}

/**
 * \brief Parse input chars as an option symbol.
 *
 * \param[in] opt         The option symbol to consume
 * \param[in] val         The option value to consume
 * \param[in] supported   The supported options to match
 * \param[in,out] pos     Character position in the call string
 * \param[in] pass_token  Function to call on each parsed token
 */
void parse_symbol(const char * const opt, const char * const val,
		const OptionRegistry& supported, int &pos,
		const option_callback& pass_token);


/**
 * \brief Parse input chars as an option shorthand symbol.
 *
 * \param[in] opt        The option symbol to consume
 * \param[in] val        The option value to consume
 * \param[in] supported  The supported options to match
 * \param[in,out] pos    Character position in the call string
 * \param[in] pass_token Function to call on each parsed token
 */
void parse_shorthand(const char * const opt, const char * const val,
		const OptionRegistry& supported, int &pos,
		const option_callback& pass_token);


void parse(const int argc, const char* const * const argv,
		const OptionRegistry& supported, const option_callback& pass_token)
{
	if (argc < 2 or !argv)
	{
		return; // No Options or Arguments
	}

	auto pos = int { 1 };   // Current Position in argv, ignore argv[0]
	const char * token = 0; // Current token
	const char * next  = 0; // Next token

	using unsigned_char = unsigned char;
	auto first_ch  = unsigned_char { 0 }; // First char in argv[pos]
	auto second_ch = unsigned_char { 0 }; // Second char in argv[pos]

	while (pos < argc)
	{
		// Leading chars of current token
		first_ch  = argv[pos][0];
		second_ch = first_ch ? argv[pos][1] : 0;

		if (first_ch == '-') // An Option Starts
		{
			if (second_ch)
			{
				// Get Next token
				token = argv[pos];
				next  = (pos + 1 < argc) ? argv[pos + 1] : 0;

				if (second_ch == '-')
				{
					// Token starts with '--'

					if (!argv[pos][2])
					{
						// Token is only '--'
						pass_token(DDASH, "");
						++pos;
						//break;
					} else
					{
						// Expected Syntax: --some-option
						parse_symbol(token, next, supported, pos, pass_token);
					}
				} else
				{
					// Expected Syntax: -o
					parse_shorthand(token, next, supported, pos, pass_token);
				}
			} else
			{
				// Token is only '-':
				pass_token(DASH, "");
				++pos;
			}
		} else
		{
			// An Argument
			pass_token(ARGUMENT, argv[pos]);
			++pos;
		}
	} // while

	while (pos < argc)
	{
		pass_token(ARGUMENT, argv[pos]);
		++pos;
	}
}


void parse_symbol(const char * const token, const char * const next,
		const OptionRegistry& supported, int &pos,
		const option_callback& pass_token)
{
	// Determine Length of Option Symbol in Token
	auto sym_len = unsigned { 0 };
	while (token[sym_len + 2] && token[sym_len + 2] != '=') { ++sym_len; }

	auto exact    = bool { false }; // Indicates Exact Match
	auto is_alias = bool { false }; // Indicates potential Alias to other Option

	// Traverse Supported Options for a Match of the Symbol

	auto code = OptionCode { ARGUMENT }; // Code to identify Option
	const Option* option = nullptr;      // Identified Option in 'supported'

	for (const auto& entry : supported)
	{
		const auto& sup_op { entry.second };

		if (std::strncmp(sup_op.symbol().c_str(), &token[2], sym_len) == 0)
		{
			if (sup_op.symbol().length() == sym_len)
			{
				// Exact Match: Stop Search
				option = &sup_op;
				code = entry.first;
				exact = true;
				break;
			} else
			{
				// Substring Match:
				// 'token[2]' is a substring of 'o', but it may still be a valid
				// option on its own.

				if (!option)
				{
					// First substring match: just memorize.
					option = &sup_op;
					// If no second match follows, the name of the option is
					// just a substring of another option name and there is
					// nothing more to do.
				} else
				{
					// Second substring match: hm...
					is_alias = (sup_op == *option);
					// If this is in fact the same option as the memorized first
					// match, it could be a second occurrence for this.
				}
			}
		}
	}

	if(!option)
	{
		std::ostringstream msg;
		msg << "Invalid option '--" << &token[2] << "'";
		throw CallSyntaxException(msg.str());
	}

	if (!exact and !is_alias) // Somehow imprecise
	{
		std::ostringstream msg;
		msg << "Option '--" << &token[2] << "' is unknown, did you mean '--"
			<< option->symbol() << "'?";
		throw CallSyntaxException(msg.str());
	}

	// Move Token Pointer for Caller
	++pos;

	if (token[sym_len + 2]) // Expect syntax '--some-option=foo'
	{
		if (option->needs_value())
		{
			if (!token[sym_len + 3])
			{
				// Value required, but nothing after the '='

				std::ostringstream msg;
				msg << "Option '--" << option->symbol()
					<< "' requires an argument but none is passed";
				throw CallSyntaxException(msg.str());
			} // else: fine
		} else
		{
			std::ostringstream msg;
			msg << "Option '--" << option->symbol();

			if (!token[sym_len + 3])
			{
				msg << "' has an unexpected trailing character '"
					<< token[sym_len + 2] << "'";
			} else
			{
				msg << "' is assigned an unexpected value: '"
					<< &token[sym_len + 3] << "'";
			}

			throw CallSyntaxException(msg.str());
		}

		pass_token(code, &token[sym_len + 3]);
	} else if (option->needs_value()) // Expect syntax '--foo bar'
	{
		if (!next or !next[0] or next[0] == '-')
		{
			if (OP_VALUE::NONE == option->default_arg()
					|| option->default_arg().empty())
			{
				std::ostringstream msg;
				msg << "Option '" << token
					<< "' requires a value but none is passed";
				throw CallSyntaxException(msg.str());
			} else
			{
				// If option is present, requires a value and has a default
				// but no actual value is passed, then use the default
				pass_token(code, option->default_arg().c_str());
			}
		} else
		{
			// Move Token Pointer for Caller
			++pos;
			pass_token(code, next);
		}
	} else
	{
		pass_token(code, "");
	}
}


void parse_shorthand(const char * const token, const char * const next,
		const OptionRegistry& supported, int &pos,
		const option_callback& pass_token)
{
	auto code = OptionCode { ARGUMENT }; // Code to identify Option
	const Option* option = nullptr;      // Identified Option in 'supported'

	// We may have concatenated options like '-lsbn':
	// Traverse all characters in token as separate options.

	using unsigned_char = unsigned char;
	auto c = unsigned_char { 0 }; // Current character
	auto cind = int { 1 };        // Position Index of Character c in Token

	while (cind > 0)
	{
		const auto c = unsigned_char (token[cind]); // Consider Next Character
		option = nullptr;

		if (c != 0)
		{
			// Traverse supported options till c is found as shorthand symbol
			for (const auto& [o_code, o] : supported)
			{
				if (c == o.shorthand_symbol())
				{
					option = &o;
					code   =  o_code;
					break;
				}
			}
		}

		// Supported Option Represented by 'c' is 'option'

		if (!option)
		{
			std::ostringstream msg;
			msg << "Invalid option '-" << c << "'";
			throw CallSyntaxException(msg.str());
		}

		++cind;

		if (token[cind] == 0) // Token is Completely Processed
		{
			cind = 0;
			++pos; // Move Token Pointer for Caller
		}

		if (option->needs_value())
		{
			if (cind > 0 and token[cind])
			{
				// Consume Trailing Part as Option Value
				pass_token(code, &token[cind]);
			} else
			{
				// No trailing part, consider Next Token as Value

				if (!next or !next[0])
				{
					std::ostringstream msg;
					msg << "Option '-" << token
						<< "' requires a value but none was passed";
					throw CallSyntaxException(msg.str());
				}
				pass_token(code, next);
			}

			cind = 0;
			++pos; // Move Token Pointer for Caller
		} else
		{
			pass_token(code, "");
		}
	} // while
}

} // namespace input

} // namespace arcsapp

