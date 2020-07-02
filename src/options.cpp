#ifndef __ARCSTOOLS_OPTIONS_HPP__
#include "options.hpp"
#endif

//#include <cstdint>  // for uint64_t
//#include <cmath>    // for pow
#include <map>      // for map, operator!=, _Rb_tree_const_iterator, _Rb_tre...
//#include <sstream>  // for ostringstream
#include <string>   // for string, basic_string
#include <utility>  // for pair, make_pair
#include <vector>   // for vector

namespace arcsapp
{

Options::Options()
	: version_   { false }
	, output_    {}
	, flags_     {}
	, values_    {}
	, arguments_ {}
{
	flags_.resize(64, false); // TODO Magic number
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


bool Options::is_set(const OptionCode &option) const
{
	return flags_[option];
}


void Options::set(const OptionCode &option)
{
	flags_[option] = true;
}


void Options::unset(const OptionCode &option)
{
	flags_[option] = false;
}


std::string Options::get(const OptionCode &option) const
{
	auto it = values_.find(option);

	if (it != values_.end())
	{
		return it->second;
	}

	return std::string();
}


void Options::put(const OptionCode &option, const std::string &value)
{
	values_.insert(std::make_pair(option, value));
}


std::string const Options::get_argument(const OptionCode &index) const
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
	return flags_.empty() and values_.empty() and arguments_.empty();
}


// Commented out but kept for reference

//OptionCode Options::leftmost_flag() const
//{
//	auto flags = config_;
//
//	if (flags == 0) { return 0; }
//
//	OptionCode count = 0;
//	while (flags > 1) { count++; flags >>= 1; }
//
//	return std::pow(2, count);
//}


//OptionCode Options::rightmost_flag() const
//{
//	return config_ & (~config_ + 1);
//}

} // namespace arcsapp

