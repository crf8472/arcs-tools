#ifndef __ARCSTOOLS_OPTIONS_HPP__
#include "options.hpp"
#endif

#include <cmath>  // for pow
#include <map>    // from .h
#include <string> // from .h
#include <vector> // from .h


// Options


Options::Options()
	: version_(false)
	, config_(0)
	, option_map_()
	, arguments_()
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


bool Options::is_set(const uint16_t &option) const
{
	return config_ & option;
}


void Options::set(const uint16_t &option)
{
	config_ |= option;
}


void Options::unset(const uint16_t &option)
{
	config_ &= !option;
}


std::string Options::get(const uint16_t &option) const
{
	auto it = option_map_.find(option);

	if (it != option_map_.end())
	{
		return it->second;
	}

	return std::string();
}


void Options::put(const uint16_t &option, const std::string &value)
{
	option_map_.insert(std::make_pair(option, value));
}


std::string const Options::get_argument(const uint16_t &index) const
{
	if (index > arguments_.size())
	{
		return std::string();
	}

	return arguments_.at(index);
}


void Options::push_back_argument(const std::string &arg)
{
	arguments_.push_back(arg);
}


std::vector<std::string> const Options::get_arguments() const
{
	return arguments_;
}


bool Options::empty() const
{
	return config_ == 0 && arguments_.empty() && option_map_.empty();
}


uint16_t Options::leftmost_flag() const
{
	auto flags = config_;

	if (flags == 0) { return 0; }

	uint16_t count = 0;
	while (flags > 1) { count++; flags >>= 1; }

	return std::pow(2, count);
}


uint16_t Options::rightmost_flag() const
{
	return config_ & (~config_ + 1);
}

