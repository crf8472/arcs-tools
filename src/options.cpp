#ifndef __ARCSTOOLS_OPTIONS_HPP__
#include "options.hpp"
#endif

//#include <cstdint>  // for uint64_t
#include <cmath>    // for pow
#include <iterator> // for ostream_iterator
#include <map>      // for map, operator!=, _Rb_tree_const_iterator, _Rb_tre...
#include <sstream>  // for ostringstream
#include <string>   // for string, basic_string
#include <utility>  // for pair, make_pair
#include <vector>   // for vector

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


// Options


Options::Options()
	: version_ { false }
	, output_  {}
	, config_  { 0 }
	, option_map_ {}
	, arguments_ {}
{
	// empty
}


Options::~Options() noexcept = default;


void Options::set_version(const bool &version)
{
	version_ = version;
}


bool Options::is_set_version() const
{
	return version_;
}


void Options::set_output(const std::string &output)
{
	output_ = output;
}


std::string Options::output() const
{
	return output_;
}


bool Options::is_set(const OptionValue &option) const
{
	return config_ & option;
}


void Options::set(const OptionValue &option)
{
	config_ |= option;
}


void Options::unset(const OptionValue &option)
{
	config_ &= !option;
}


std::string Options::get(const OptionValue &option) const
{
	auto it = option_map_.find(option);

	if (it != option_map_.end())
	{
		return it->second;
	}

	return std::string();
}


void Options::put(const OptionValue &option, const std::string &value)
{
	option_map_.insert(std::make_pair(option, value));
}


std::string const Options::get_argument(const OptionValue &index) const
{
	if (index > arguments_.size())
	{
		return std::string();
	}

	return arguments_.at(index);
}


std::vector<std::string> const Options::get_arguments() const
{
	return arguments_;
}


bool Options::no_arguments() const
{
	return arguments_.empty();
}


void Options::append(const std::string &arg)
{
	arguments_.push_back(arg);
}


bool Options::empty() const
{
	return config_ == 0 && arguments_.empty() && option_map_.empty();
}


// Commented out but kept for reference

//OptionValue Options::leftmost_flag() const
//{
//	auto flags = config_;
//
//	if (flags == 0) { return 0; }
//
//	OptionValue count = 0;
//	while (flags > 1) { count++; flags >>= 1; }
//
//	return std::pow(2, count);
//}


//OptionValue Options::rightmost_flag() const
//{
//	return config_ & (~config_ + 1);
//}

} // namespace arcsapp

