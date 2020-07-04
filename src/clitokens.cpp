#ifndef __ARCSTOOLS_CLITOKENS_HPP__
#include "clitokens.hpp"
#endif

#include <algorithm>  // for find
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


// CallSyntaxException


CallSyntaxException::CallSyntaxException(const std::string &what_arg)
	: std::runtime_error(what_arg)
{
	// empty
}


// CLIInput


CLIInput::CLIInput(const int argc, const char* const * const argv,
		const std::vector<std::pair<Option, OptionCode>> &supported,
		const bool preserve_order)
	: items_ () // TODO reserve sensible capacity
{
	if (argc < 2 or !argv)
	{
		return; // No Options or Arguments Present
	}

	auto pos = int { 1 }; // Ignore argv[0]

	unsigned char first_ch  = 0;
	unsigned char second_ch = 0;

	auto non_options = std::vector<const char*>{}; // Tokens but Not Options

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
				items_.push_back(InputItem(argv[pos]));
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
		items_.push_back(InputItem(non_options[i]));
	}

	while (pos < argc)
	{
		items_.push_back(InputItem(argv[pos]));
		++pos;
	}
}


CLIInput::~CLIInput() noexcept = default;


OptionCode CLIInput::option_code(const int i) const
{
	if (i >= 0 && static_cast<decltype( items_.size() )>(i) < items_.size())
	{
		return items_[i].id();
	}

	return 0;
}


const std::string& CLIInput::option_value(const int i) const
{
	if (i >= 0 && static_cast<decltype( items_.size() )>(i) < items_.size())
	{
		return items_[i].value();
	}

	return empty_value();
}


bool CLIInput::empty() const
{
	return items_.empty();
}


bool CLIInput::contains(const OptionCode &option) const noexcept
{
	return lookup(option) ? true : false;
}


const std::string& CLIInput::value(const OptionCode &option) const
{
	auto element = lookup(option);

	return element ? element->value() : empty_value();
}


CLIInput::size_type CLIInput::size() const
{
	return items_.size();
}


void CLIInput::consume_as_symbol(const char * const token,
		const char * const next,
		const std::vector<std::pair<Option, OptionCode>> supported, int &pos)
{
	std::cout << "--- Pos: " << pos << "  [symbol]" << std::endl;
	std::cout << "Token: " << token << std::endl;

	auto name_len = unsigned { 0 };

	// determine length of option name
	while (token[name_len + 2] && token[name_len + 2] != '=') { ++name_len; }

	auto index = int { -1 };
	bool exact = false;
	bool is_substring = false;

	// Test supported options for a match
	auto i = int { 0 };
	auto code = OptionCode { Option::NONE };
	for (const auto& entry : supported)
	{
		const auto& option { entry.first };

		if (std::strncmp(option.symbol().c_str(), &token[2], name_len) == 0)
		{
			if (option.symbol().length() == name_len)
			{
				index = i;
				code = entry.second;
				exact = true;
				break;
			} else if (index < 0)
			{
				index = i;
			} else
			{
				const auto& s_option = supported[index].first;

				is_substring = s_option.symbol() != option.symbol()
					   || s_option.needs_value() != option.needs_value();
			}
		}

		++i;
	}

	if (is_substring && not exact)
	{
		std::ostringstream msg;
		msg << "Option " << token << " is ambigous";
		throw CallSyntaxException(msg.str());
	}

	if (index < 0)
	{
		std::ostringstream msg;
		msg << "Invalid option '" << token << "'";
		throw CallSyntaxException(msg.str());
	}

	++pos;
	items_.emplace_back(code);

	const auto& option = supported[index].first;

	if (token[name_len + 2]) // Expect syntax '--some-option=foo'
	{
		if (option.needs_value())
		{
			if (!token[name_len + 3])
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

			if (!token[name_len + 3])
			{
				msg << " has an unexpected trailing character '"
					<< token[name_len + 2] << "'";
			} else
			{
				msg << " is assigned an unexpected argument: '"
					<< &token[name_len + 3] << "'";
			}

			throw CallSyntaxException(msg.str());
		}

		std::cout << "Value : " << &token[name_len + 3] << std::endl;
		items_.back().set_value(&token[name_len + 3]);
	} else if (option.needs_value()) // Expect syntax '--foo bar'
	{
		if (!next or !next[0])
		{
			std::ostringstream msg;
			msg << "Option " << token
				<< " requires argument but none is passed";
			throw CallSyntaxException(msg.str());
		}

		++pos;
		std::cout << "Value : " << next << std::endl;
		items_.back().set_value(next);
	}
}


void CLIInput::consume_as_shorthand(const char * const token,
		const char * const next,
		const std::vector<std::pair<Option, OptionCode>> supported, int &pos)
{

	std::cout << "--- Pos: " << pos << "  [shorthand]" << std::endl;
	std::cout << "Token: " << token << std::endl;

	auto cind  = int { 1 };  // Position Index of Character in Token
	auto index = int { -1 }; // Index of Identified Option in 'supported'
	auto code  = OptionCode { Option::NONE };

	while (cind > 0)
	{
		const unsigned char c = token[cind];

		index = -1;
		if (c != 0)
		{
			auto i = int { 0 };
			for (const auto& option : supported)
			{
				if (c == option.first.shorthand_symbol())
				{
					index = i;
					code = option.second;
					break;
				}
				++i;
			}
		}

		if (index < 0)
		{
			std::ostringstream msg;
			msg << "Invalid option '-" << c << "'";
			throw CallSyntaxException(msg.str());
		}

		// Supported Option Represented by 'c' is 'supported[index]'

		std::cout << "Option '-" << supported[index].first.shorthand_symbol()
			<< "' recognized" << std::endl;
		items_.emplace_back(code);

		++cind;

		if (token[cind] == 0)
		{
			++pos;
			cind = 0; // Token is Completely Processed
		}

		if (supported[index].first.needs_value())
		{
			if (cind > 0 && token[cind])
			{
				// Option Value is Present in Token

				std::cout << "Value : " << &token[cind] << std::endl;
				items_.back().set_value(&token[cind]);
			} else
			{
				// No Value in Token found, consider Next Token

				if (!next || !next[0])
				{
					std::ostringstream msg;
					msg << "Option '-" << token
						<< "' requires an argument but none was passed";
					throw CallSyntaxException(msg.str());
				}

				std::cout << "Value : " << next << std::endl;
				items_.back().set_value(next);
			}

			++pos;
			cind = 0;
		}
	}
}


CLIInput::iterator CLIInput::begin()
{
	return items_.begin();
}


CLIInput::iterator CLIInput::end()
{
	return items_.end();
}


CLIInput::const_iterator CLIInput::begin() const
{
	return items_.begin();
}


CLIInput::const_iterator CLIInput::end() const
{
	return items_.end();
}


CLIInput::const_iterator CLIInput::cbegin() const
{
	return items_.cbegin();
}


CLIInput::const_iterator CLIInput::cend() const
{
	return items_.cend();
}


const CLIInput::InputItem* CLIInput::lookup(const OptionCode &option) const
{
	auto element = std::find_if(items_.begin(), items_.end(),
			[option](const InputItem &i){ return i.id() == option; } );

	if (element != items_.end())
	{
		return &(*element);
	}

	return nullptr;
}


const std::string& CLIInput::empty_value() const noexcept
{
	static const auto empty_string = std::string{};
	return empty_string;
}


// CLITokens


CLITokens::CLITokens (const int argc, const char* const * const argv)
	: tokens_ {}
{
	if (!argv or !*argv)
	{
		return;
	}

	for (auto i = int { 1 }; i < argc; ++i)
	{
		this->tokens_.push_back(std::string(argv[i]));
	}
}


CLITokens::~CLITokens () noexcept = default;


std::tuple<bool, std::string> CLITokens::consume(const std::string &token,
		const bool value_requested) noexcept
{
	if (tokens_.empty())
	{
		return std::make_tuple(false, empty_value());
	}

	auto itr = std::find(tokens_.begin(), tokens_.end(), token);

	if (itr != tokens_.end()) // token found
	{
		if (not value_requested)
		{
			ARCS_LOG(DEBUG1) << "Consume token '" << token << "'";

			tokens_.erase(itr);
			return std::make_tuple(true, empty_value());
		}

		++itr;

		if (itr != tokens_.end())
		{
			//  next token is considered option value

			auto ret_val = std::make_tuple(true, std::string(*itr));

			ARCS_LOG(DEBUG1) << "Consume token '" << token << "', value '"
				<< std::get<1>(ret_val) << "'";

			--itr;
			itr = tokens_.erase(itr); // erase option switch
			tokens_.erase(itr);       // erase value

			return ret_val;
		}

		// value_requested but not present

		--itr;
		tokens_.erase(itr); // erase token

		ARCS_LOG_WARNING << "Consume token '" << token << "', value missing";

		return std::make_tuple(true, empty_value());
	}

	return std::make_tuple(false, empty_value());
}


const std::string CLITokens::consume() noexcept
{
	if (tokens_.empty())
	{
		return empty_value();
	}

	auto token = tokens_.front();
	tokens_.pop_front();

	ARCS_LOG(DEBUG1) << "Consume token: '" << token << "'";

	return token;
}


bool CLITokens::contains(const std::string &token) const noexcept
{
	auto itr = std::find(tokens_.begin(), tokens_.end(), token);

	ARCS_LOG(DEBUG1) << "Token '" << token << "' is present";

	return itr != tokens_.end();
}


bool CLITokens::empty() noexcept
{
	return tokens_.empty();
}


const std::vector<std::string> CLITokens::unconsumed() noexcept
{
	return std::vector<std::string>(tokens_.begin(), tokens_.end());
}


const std::string& CLITokens::empty_value() noexcept
{
	static const auto empty_string = std::string{};
	return empty_string;
}

} // namespace arcsapp

