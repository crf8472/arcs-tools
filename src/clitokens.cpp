#ifndef __ARCSTOOLS_CLITOKENS_HPP__
#include "clitokens.hpp"
#endif

#include <algorithm>  // for copy, find_if
#include <cstring>    // for strncmp
#include <iterator>   // for ostream_iterator, begin, end, cbegin, cend
#include <sstream>    // for ostringstream
#include <stdexcept>  // for runtime_error
#include <string>     // for string
#include <utility>    // for pair
#include <vector>     // for vector

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


char Option::shorthand_symbol() const
{
	return shorthand_;
}


std::string Option::symbol() const
{
	return symbol_;
}


bool Option::needs_value() const
{
	return needs_value_;
}


std::string Option::default_arg() const
{
	return default_arg_;
}


std::string Option::description() const
{
	return description_;
}


std::vector<std::string> Option::tokens() const
{
	std::vector<std::string> tokens;
	tokens.reserve(2);

	if (!this->symbol().empty())
	{
		tokens.push_back("--" + this->symbol());
	}

	if (this->shorthand_symbol() != '\0')
	{
		tokens.push_back({'-', this->shorthand_symbol()});
	}

	return tokens;
}


std::string Option::tokens_str() const
{
	std::ostringstream oss;

	if (const auto tokens { this->tokens() }; !tokens.empty())
	{
		using std::begin;
		using std::end;

		std::copy(begin(tokens), end(tokens) - 1,
			std::ostream_iterator<std::string>(oss, ","));
		oss << tokens.back();
	}

	return oss.str();
}


bool operator == (const Option &lhs, const Option &rhs) noexcept
{
	return lhs.symbol() == rhs.symbol()
		&& lhs.shorthand_symbol() == rhs.shorthand_symbol()
		&& lhs.needs_value() == rhs.needs_value();
}


// CallSyntaxException


CallSyntaxException::CallSyntaxException(const std::string &what_arg)
	: std::runtime_error { what_arg }
{
	// empty
}


namespace input
{


void get_tokens(const int argc, const char* const * const argv,
		const std::vector<std::pair<Option, OptionCode>> &supported,
		const option_callback& f)
{
	// TODO
}


std::vector<Token> get_tokens(const int argc,
		const char* const * const argv,
		const std::vector<std::pair<Option, OptionCode>>& supported)
{
	std::vector<Token> tokens;
	tokens.reserve(7);

	if (argc < 2 or !argv)
	{
		return {}; // No Options or Arguments Present
	}

	auto pos = int { 1 }; // Ignore argv[0]

	using unsigned_char = unsigned char;
	auto first_ch  = unsigned_char { 0 };
	auto second_ch = unsigned_char { 0 };

	while (pos < argc)
	{
		first_ch  = argv[pos][0];
		second_ch = first_ch ? argv[pos][1] : 0;

		if (first_ch == '-') // An Option Starts
		{
			if (second_ch)
			{
				const char * const token = argv[pos];
				const char * const next = (pos + 1 < argc) ? argv[pos + 1] : 0;

				if (second_ch == '-')
				{
					if (!argv[pos][2])
					{
						// Input is only '--'
						++pos;
						break;
					} else
					{
						// Expected Syntax: --some-option
						try
						{
							tokens.push_back(
								consume_as_symbol(token, next, supported, pos));
						}
						catch (const CallSyntaxException &e)
						{
							throw;
						}
					}
				} else
				{
					// Expected Syntax: -o
					try
					{
						auto sh_symbols {
							consume_as_shorthand(token, next, supported, pos)
						};
						if (!sh_symbols.empty())
						{
							using std::begin;
							using std::end;
							tokens.insert(end(tokens), begin(sh_symbols),
									end(sh_symbols));
						}
					}
					catch (const CallSyntaxException &e)
					{
						throw;
					}
				}
			} else
			{
				// Input item is only '-':
				throw CallSyntaxException("Illegal input (only '-')");
			}
		} else // found an argument
		{
			tokens.push_back(Token(argv[pos]));
			pos++;
		}
	} // while

	while (pos < argc)
	{
		tokens.push_back(Token(argv[pos]));
		++pos;
	}
	tokens.shrink_to_fit();

	return tokens;
}


Token consume_as_symbol(const char * const token, const char * const next,
		const std::vector<std::pair<Option, OptionCode>>& supported, int &pos)
{
	// Determine Length of Option Symbol in Token
	auto sym_len = unsigned { 0 };
	while (token[sym_len + 2] && token[sym_len + 2] != '=') { ++sym_len; }

	// Position of identified Option in 'supported'
	auto option_pos = int { -1 };

	auto exact    = bool { false }; // Indicates Exact Match
	auto is_alias = bool { false }; // Indicates an Alias for an Other Option

	// Traverse Supported Options for a Match of the Symbol
	auto i = int { 0 };
	auto code = OptionCode { Option::NONE };
	for (const auto& entry : supported)
	{
		const auto& option { entry.first };

		if (std::strncmp(option.symbol().c_str(), &token[2], sym_len) == 0)
		{
			if (option.symbol().length() == sym_len)
			{
				option_pos = i;
				code = entry.second;
				exact = true;

				// Exact Match: Stop Search
				break;

			} else
			{
				// Substring Match:
				// 'token[2]' is a substring of 'option', but may be a valid
				// option itself.

				if (option_pos < 0)
				{
					// First Substring Match: memorize.
					// If no Second Substring Match follows, the name of the
					// option is just a substring of an other option name.
					option_pos = i;
				} else
				{
					// Second Substring Match: Check whether this is an Alias
					// for the Memorized First Match ("abbreviated option").
					is_alias = (option == supported[option_pos].first);
				}
			}
		}

		++i;
	}

	if (option_pos < 0)
	{
		std::ostringstream msg;
		msg << "Invalid option '--" << &token[2] << "'";
		throw CallSyntaxException(msg.str());
	}

	if (not exact and not is_alias)
	{
		std::ostringstream msg;
		msg << "Option '--" << &token[2] << "' is unknown, did you mean '--"
			<< supported[option_pos].first.symbol() << "'?";
		throw CallSyntaxException(msg.str());
	}

	// Move Token Pointer for Caller
	++pos;

	Token result_token { code };

	const auto& option { supported[option_pos].first };

	if (token[sym_len + 2]) // Expect syntax '--some-option=foo'
	{
		if (option.needs_value())
		{
			// Value required, but nothing after the '='

			if (!token[sym_len + 3])
			{
				std::ostringstream msg;
				msg << "Option '--" << option.symbol()
					<< "' requires an argument but none is passed";
				throw CallSyntaxException(msg.str());
			}
		} else
		{
			std::ostringstream msg;
			msg << "Option '--" << option.symbol();

			if (!token[sym_len + 3])
			{
				msg << "' has an unexpected trailing character '"
					<< token[sym_len + 2] << "'";
			} else
			{
				msg << "' is assigned an unexpected argument: '"
					<< &token[sym_len + 3] << "'";
			}

			throw CallSyntaxException(msg.str());
		}

		result_token.set_value(&token[sym_len + 3]);
	} else if (option.needs_value()) // Expect syntax '--foo bar'
	{
		if (!next or !next[0])
		{
			std::ostringstream msg;
			msg << "Option '" << token
				<< "' requires an argument but none is passed";
			throw CallSyntaxException(msg.str());
		}

		// Move Token Pointer for Caller
		++pos;

		result_token.set_value(next);
	}

	return result_token;
}


std::vector<Token> consume_as_shorthand(const char * const token,
		const char * const next,
		const std::vector<std::pair<Option, OptionCode>>& supported, int &pos)
{
	auto result_tokens = std::vector<Token> { };

	using unsigned_char = unsigned char;

	// Position Index of Character in Token
	auto cind  = int { 1 };

	// Position of identified Option in 'supported'
	auto option_pos = int { -1 };

	auto code = OptionCode { Option::NONE };

	// We may have concatenated options like '-lsbn':
	// Traverse all characters in token as separate options.
	while (cind > 0)
	{
		//const unsigned char c = token[cind]; // Consider Next Character
		const auto c = unsigned_char (token[cind]); // Consider Next Character
		option_pos = -1; // Flag "nothing found"

		if (c != 0)
		{
			auto i = int { 0 };
			for (const auto& option : supported)
			{
				if (c == option.first.shorthand_symbol())
				{
					option_pos = i;
					code = option.second;
					break;
				}
				++i;
			}
		}

		if (option_pos < 0)
		{
			std::ostringstream msg;
			msg << "Invalid option '-" << c << "'";
			throw CallSyntaxException(msg.str());
		}

		// Supported Option Represented by 'c' is 'supported[option_pos].first'

		result_tokens.emplace_back(code);

		++cind;

		if (token[cind] == 0) // Token is Completely Processed
		{
			cind = 0;

			// Move Token Pointer for Caller
			++pos;
		}

		if (supported[option_pos].first.needs_value())
		{
			if (cind > 0 and token[cind])
			{
				// Consume Trailing Part as Option Value

				result_tokens.back().set_value(&token[cind]);
			} else
			{
				// No trailing part, consider Next Token as Value

				if (!next or !next[0])
				{
					std::ostringstream msg;
					msg << "Option '-" << token
						<< "' requires an argument but none was passed";
					throw CallSyntaxException(msg.str());
				}

				result_tokens.back().set_value(next);
			}
			cind = 0;

			// Move Token Pointer for Caller
			++pos;
		}
	} // while

	return result_tokens;
}

} // namespace input


// CLITokens


CLITokens::CLITokens()
	: option_ {/* empty */}
	, tokens_ {/* empty */}
{
	// empty
}


CLITokens::~CLITokens() noexcept = default;


void CLITokens::register_callback(const input::option_callback f)
{
	option_ = f;
}


void CLITokens::parse(const int argc, const char* const * const argv,
			const std::vector<std::pair<Option, OptionCode>> &supported)
{
	tokens_ = input::get_tokens(argc, argv, supported);
}


//


CLITokens::CLITokens(const int argc, const char* const * const argv,
		const std::vector<std::pair<Option, OptionCode>> &supported)
	: option_ { /* empty */ }
	, tokens_ { input::get_tokens(argc, argv, supported) }
{
	// empty
}


const std::string& CLITokens::value(const OptionCode &option) const
{
	const auto element { lookup(option) };

	return element ? element->value() : Token::empty_value();
}


bool CLITokens::contains(const OptionCode &option) const noexcept
{
	return lookup(option);
}


OptionCode CLITokens::option_code(const size_type i) const
{
	return get(i) ? get(i)->code() : 0;
}


const std::string& CLITokens::option_value(const size_type i) const
{
	return get(i) ? get(i)->value() : Token::empty_value();
}


const std::string& CLITokens::argument(const size_type &i) const
{
	using std::begin;
	using std::end;

	if (i >= size())
	{
		return Token::empty_value();
	}

	const auto stop { end(tokens_) };
	auto counter = size_type { 0 };
	auto pos   { begin(tokens_) };
	auto token { pos };

	const auto is_argument {
		[](const Token &item) { return item.code() == Option::NONE; }
	};

	do
	{
		token = std::find_if(pos, stop, is_argument);

		if (stop == token)
		{
			return Token::empty_value();
		}

		++counter;
		pos = token + 1;

	} while (pos != stop && counter <= i);

	return counter > i ? token->value() : Token::empty_value() ;
}


const CLITokens::Token* CLITokens::lookup(const OptionCode &option) const
{
	using std::begin;
	using std::end;

	const auto element { std::find_if(begin(tokens_), end(tokens_),
			[option](const Token &i)
			{
				return i.code() == option;
			})
	};

	if (element != end(tokens_))
	{
		return &(*element);
	}

	return nullptr;
}


const CLITokens::Token* CLITokens::get(const size_type i) const
{
	if (i >= 0 && i < tokens_.size())
	{
		return &tokens_[i];
	}

	return nullptr;
}


CLITokens::size_type CLITokens::size() const
{
	return tokens_.size();
}


bool CLITokens::empty() const
{
	return tokens_.empty();
}


CLITokens::iterator CLITokens::begin()
{
	using std::begin;
	return begin(tokens_);
}


CLITokens::iterator CLITokens::end()
{
	using std::end;
	return end(tokens_);
}


CLITokens::const_iterator CLITokens::begin() const
{
	using std::begin;
	return begin(tokens_);
}


CLITokens::const_iterator CLITokens::end() const
{
	using std::end;
	return end(tokens_);
}


CLITokens::const_iterator CLITokens::cbegin() const
{
	using std::cbegin;
	return cbegin(tokens_);
}


CLITokens::const_iterator CLITokens::cend() const
{
	using std::cend;
	return cend(tokens_);
}

} // namespace arcsapp

