#ifndef __ARCSTOOLS_CLITOKENS_HPP__
#include "clitokens.hpp"
#endif

#include <algorithm>  // for find
#include <string>     // for string, allocator
#include <tuple>      // for tuple, get
#include <vector>     // for vector

#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

namespace arcsapp
{

// CLITokens


CLITokens::CLITokens (int argc, char **argv)
	: tokens_ {}
{
	if (!argv or !*argv)
	{
		return;
	}

	for (int i = 1; i < argc; ++i)
	{
		this->tokens_.push_back(std::string(argv[i]));
	}
}


CLITokens::~CLITokens () noexcept = default;


std::tuple<bool, std::string> CLITokens::consume_option_token(
		const std::string &token, const bool value_requested) noexcept
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


const std::string CLITokens::consume_argument() noexcept
{
	if (tokens_.empty())
	{
		return empty_value();
	}

	auto argument = tokens_.front();
	tokens_.pop_front();

	ARCS_LOG(DEBUG1) << "Consume argument: '" << argument << "'";

	return argument;
}


bool CLITokens::token_present(const std::string &token) noexcept
{
	auto itr = std::find(tokens_.begin(), tokens_.end(), token);

	ARCS_LOG(DEBUG1) << "Token '" << token << "' is present";

	return itr != tokens_.end();
}


bool CLITokens::tokens_left() noexcept
{
	return !tokens_.empty();
}


const std::vector<std::string> CLITokens::unconsumed_tokens() noexcept
{
	return std::vector<std::string>(tokens_.begin(), tokens_.end());
}


const std::string& CLITokens::empty_value() noexcept
{
	static const auto empty_string = std::string{};
	return empty_string;
}

} // namespace arcsapp

