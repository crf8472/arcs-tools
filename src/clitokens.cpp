#ifndef __ARCSTOOLS_CLITOKENS_HPP__
#include "clitokens.hpp"
#endif

#include <algorithm>  // for find
#include <iterator>   // for ostream_iterator
#include <sstream>    // for ostringstream
#include <string>     // for string, allocator
#include <tuple>      // for tuple, get
#include <vector>     // for vector

#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

namespace arcsapp
{

// Option


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

