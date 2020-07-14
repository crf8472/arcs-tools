#ifndef __ARCSTOOLS_CLITOKENS_HPP__
#include "clitokens.hpp"
#endif

#include <algorithm>  // for find, find_if
#include <cstring>    // for strncmp
#include <iterator>   // for ostream_iterator
#include <sstream>    // for ostringstream
#include <string>     // for string, allocator
#include <tuple>      // for tuple, get
#include <vector>     // for vector

#include <iostream>   // cout (for debugging)

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
	, default_     { default_arg }
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
	return default_;
}


std::string Option::description() const
{
	return description_;
}


std::vector<std::string> Option::tokens() const
{
	std::vector<std::string> tokens;
	tokens.reserve(2);

	auto symbol = this->symbol();
	if (not symbol.empty())
	{
		symbol.insert(0, "--");
		tokens.push_back(symbol);
	}

	if (auto shorthand_sym = this->shorthand_symbol(); shorthand_sym != '\0')
	{
		auto shorthand = std::string(1, shorthand_sym);
		shorthand.insert(0, "-");
		tokens.push_back(shorthand);
	}

	return tokens;
}


std::string Option::tokens_str() const
{
	std::ostringstream oss;
	auto tokens = this->tokens();

	if (!tokens.empty())
	{
		std::copy(tokens.begin(), tokens.end() - 1,
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
	: std::runtime_error(what_arg)
{
	// empty
}


// CLITokens


CLITokens::CLITokens(const int argc, const char* const * const argv,
		const std::vector<std::pair<Option, OptionCode>> &supported,
		const bool preserve_order)
	: tokens_ () // TODO reserve sensible capacity
{
	if (argc < 2 or !argv)
	{
		return; // No Options or Arguments Present
	}

	auto pos = int { 1 }; // Ignore argv[0]

	unsigned char first_ch  = 0;
	unsigned char second_ch = 0;

	auto non_options = std::vector<const char*>{}; // Tokens Not Options

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
						try { consume_as_symbol(token, next, supported, pos); }
						catch (const CallSyntaxException &e)
						{
							throw;
						}
					}
				} else
				{
					// Expected Syntax: -o
					try { consume_as_shorthand(token, next, supported, pos); }
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
			if (preserve_order)
			{
				// preserve order of occurrences
				tokens_.push_back(Token(argv[pos]));
			} else
			{
				// handle arguments separately
				non_options.push_back(argv[pos]);
			}
			pos++;
		}
	} // while

	for (auto i = std::size_t { 0 }; i < non_options.size(); ++i)
	{
		tokens_.push_back(Token(non_options[i]));
	}

	while (pos < argc)
	{
		tokens_.push_back(Token(argv[pos]));
		++pos;
	}
}


CLITokens::~CLITokens() noexcept = default;


OptionCode CLITokens::option_code(const int i) const
{
	if (i >= 0 && static_cast<decltype( tokens_.size() )>(i) < tokens_.size())
	{
		return tokens_[i].code();
	}

	return 0;
}


const std::string& CLITokens::option_value(const int i) const
{
	if (i >= 0 && static_cast<decltype( tokens_.size() )>(i) < tokens_.size())
	{
		return tokens_[i].value();
	}

	return empty_value();
}


bool CLITokens::empty() const
{
	return tokens_.empty();
}


bool CLITokens::contains(const OptionCode &option) const noexcept
{
	return lookup(option) ? true : false;
}


const std::string& CLITokens::value(const OptionCode &option) const
{
	auto element = lookup(option);

	return element ? element->value() : empty_value();
}


const std::string& CLITokens::argument(const std::size_t &i) const
{
	if (i >= size())
	{
		return empty_value();
	}

	std::size_t counter = 0;
	auto token = tokens_.begin();

	while (counter < i)
	{
		token = std::find_if(tokens_.begin() + counter, tokens_.end(),
				[i](const Token &item)
				{
					return item.code() == Option::NONE;
				});

		if (token == tokens_.end())
		{
			return empty_value();
		}

		++counter;
	}

	return token->value();
}


CLITokens::size_type CLITokens::size() const
{
	return tokens_.size();
}


void CLITokens::consume_as_symbol(const char * const token,
		const char * const next,
		const std::vector<std::pair<Option, OptionCode>> supported, int &pos)
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
		msg << "Invalid option --'" << &token[2] << "'";
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

	tokens_.emplace_back(code);

	const auto& option = supported[option_pos].first;

	if (token[sym_len + 2]) // Expect syntax '--some-option=foo'
	{
		if (option.needs_value())
		{
			// Value required, but nothing after the '='

			if (!token[sym_len + 3])
			{
				std::ostringstream msg;
				msg << "Option --" << option.symbol()
					<< " requires argument but none is passed";
				throw CallSyntaxException(msg.str());
			}
		} else
		{
			std::ostringstream msg;
			msg << "Option --" << option.symbol();

			if (!token[sym_len + 3])
			{
				msg << " has an unexpected trailing character '"
					<< token[sym_len + 2] << "'";
			} else
			{
				msg << " is assigned an unexpected argument: '"
					<< &token[sym_len + 3] << "'";
			}

			throw CallSyntaxException(msg.str());
		}

		tokens_.back().set_value(&token[sym_len + 3]);
	} else if (option.needs_value()) // Expect syntax '--foo bar'
	{
		if (!next or !next[0])
		{
			std::ostringstream msg;
			msg << "Option " << token
				<< " requires argument but none is passed";
			throw CallSyntaxException(msg.str());
		}

		// Move Token Pointer for Caller
		++pos;
		tokens_.back().set_value(next);
	}
}


void CLITokens::consume_as_shorthand(const char * const token,
		const char * const next,
		const std::vector<std::pair<Option, OptionCode>> supported, int &pos)
{
	// Position Index of Character in Token
	auto cind  = int { 1 };

	// Position of identified Option in 'supported'
	auto option_pos = int { -1 };

	auto code  = OptionCode { Option::NONE };

	// We may have concatenated options like '-lsbn':
	// Traverse all characters in token as separate options.
	while (cind > 0)
	{
		const unsigned char c = token[cind]; // Consider Next Character
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

		tokens_.emplace_back(code);

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

				tokens_.back().set_value(&token[cind]);
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

				tokens_.back().set_value(next);
			}
			cind = 0;

			// Move Token Pointer for Caller
			++pos;
		}
	} // while
}


CLITokens::iterator CLITokens::begin()
{
	return tokens_.begin();
}


CLITokens::iterator CLITokens::end()
{
	return tokens_.end();
}


CLITokens::const_iterator CLITokens::begin() const
{
	return tokens_.begin();
}


CLITokens::const_iterator CLITokens::end() const
{
	return tokens_.end();
}


CLITokens::const_iterator CLITokens::cbegin() const
{
	return tokens_.cbegin();
}


CLITokens::const_iterator CLITokens::cend() const
{
	return tokens_.cend();
}


const CLITokens::Token* CLITokens::lookup(const OptionCode &option) const
{
	auto element = std::find_if(tokens_.begin(), tokens_.end(),
			[option](const Token &i){ return i.code() == option; } );

	if (element != tokens_.end())
	{
		return &(*element);
	}

	return nullptr;
}


const std::string& CLITokens::empty_value() const noexcept
{
	static const auto empty_string = std::string{};
	return empty_string;
}

} // namespace arcsapp

