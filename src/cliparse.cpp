#ifndef __ARCSTOOLS_CLIPARSE_HPP__
#include "cliparse.hpp"
#endif

#include <algorithm>  // for find
#include <string>     // for string, allocator
#include <vector>     // for vector


// CLIParser


CLIParser::CLIParser (int argc, char **argv)
	: tokens_()
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


CLIParser::~CLIParser () noexcept = default;


const std::string CLIParser::consume_argument()
{
	if (tokens_.empty())
	{
		return std::string();
	}

	std::string argument(tokens_.front());
	tokens_.pop_front();

	return argument;
}


std::tuple<bool, std::string> CLIParser::consume_valued_option(
		const std::string &option)
{
	if (tokens_.empty())
	{
		return std::make_tuple(false, std::string());
	}

	auto itr = std::find(tokens_.begin(), tokens_.end(), option);

	if (itr != tokens_.end())
	{
		++itr;

		if (itr != tokens_.end())
		{
			std::string opt_value = std::string(*itr);

			tokens_.erase(itr); // erase value
			--itr;
			tokens_.erase(itr); // erase option

			return std::make_tuple(true, opt_value);
		}

		// Valued option without value

		--itr;
		tokens_.erase(itr); // erase option

		return std::make_tuple(true, std::string());
	}

	return std::make_tuple(false, std::string());
}


bool CLIParser::option_present(const std::string &option)
{
	auto itr = std::find(tokens_.begin(), tokens_.end(), option);

	return itr != tokens_.end();
}


bool CLIParser::consume_option(const std::string &option)
{
	if (tokens_.empty())
	{
		return false;
	}

	auto itr = std::find(tokens_.begin(), tokens_.end(), option);

	if (itr != tokens_.end())
	{
		tokens_.erase(itr);
		return true;
	}

	return false;
}


bool CLIParser::tokens_left()
{
	return !tokens_.empty();
}


const std::vector<std::string> CLIParser::get_unconsumed_tokens()
{
	return std::vector<std::string>(tokens_.begin(), tokens_.end());
}

