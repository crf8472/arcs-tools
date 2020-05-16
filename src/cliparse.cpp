#ifndef __ARCSTOOLS_CLIPARSE_HPP__
#include "cliparse.hpp"
#endif

#include <algorithm>  // for find
#include <string>     // for string, allocator
#include <tuple>      // for tuple, get
#include <vector>     // for vector

#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif


// CLIParser


CLIParser::CLIParser (int argc, char **argv)
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


CLIParser::~CLIParser () noexcept = default;


const std::string CLIParser::consume_argument()
{
	if (tokens_.empty())
	{
		ARCS_LOG(DEBUG1) << "Argument requested, but no more tokens";

		return std::string();
	}

	auto argument = tokens_.front();
	tokens_.pop_front();

	ARCS_LOG(DEBUG1) << "Consume argument: " << argument;

	return argument;
}


std::tuple<bool, std::string> CLIParser::consume_valued_option(
		const std::string &option)
{
	if (tokens_.empty())
	{
		ARCS_LOG(DEBUG1) << "Valued option requested, but no more tokens";

		return std::make_tuple(false, std::string());
	}

	auto itr = std::find(tokens_.begin(), tokens_.end(), option);

	if (itr != tokens_.end())
	{
		++itr;

		if (itr != tokens_.end())
		{
			auto opt_val = std::make_tuple(true, std::string(*itr));

			auto option_itr = itr - 1;
			tokens_.erase(option_itr); // erase option
			tokens_.erase(itr); // erase value

			ARCS_LOG(DEBUG1) << "Consume option " << option << ": "
				<< std::get<1>(opt_val);

			return opt_val;
		} else
		{
			// Valued option without value

			--itr;
			tokens_.erase(itr); // erase option

			ARCS_LOG(DEBUG1) << "Consume option " << option
				<< " that unexpectedly has no value";

			return std::make_tuple(true, std::string());
		}
	}

	return std::make_tuple(false, std::string());
}


bool CLIParser::option_present(const std::string &option)
{
	auto itr = std::find(tokens_.begin(), tokens_.end(), option);

	ARCS_LOG(DEBUG1) << "Recognized option: " << option;

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
		ARCS_LOG(DEBUG1) << "Consume option switch: " << option;

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

