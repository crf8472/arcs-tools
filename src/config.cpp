#ifndef __ARCSTOOLS_CONFIG_HPP__
#include "config.hpp"
#endif

#include <cstdio>        // for stdout
#include <iterator>      // for ostream_iterator
#include <map>           // for map
#include <sstream>       // for operator<<, basic_ostream::operator<<
#include <string>        // for char_traits, operator<<, operator+
#include <type_traits>   // for add_const<>::type, __underlying_type_i...
#include <utility>       // for move
#include <vector>        // for vector

#include <iostream>

#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#ifndef __ARCSTOOLS_CLITOKENS_HPP__
#include "clitokens.hpp"
#endif

namespace arcsapp
{

using arcstk::Appender;


// LogManager


LogManager::LogManager(const LOGLEVEL default_lvl, const LOGLEVEL quiet_lvl)
	: default_loglevel_ { default_lvl }
	, quiet_loglevel_   { quiet_lvl   }
{
	// empty
}


LOGLEVEL LogManager::default_loglevel() const
{
	return default_loglevel_;
}


LOGLEVEL LogManager::quiet_loglevel() const
{
	return quiet_loglevel_;
}


LOGLEVEL LogManager::get_loglevel(const std::string &loglevel_arg)
{
	using arcstk::LOGLEVEL_MIN;
	using arcstk::LOGLEVEL_MAX;

	auto parsed_level = int { -1 };

	try {

		parsed_level = std::stoi(loglevel_arg);

	} catch (const std::invalid_argument &ia)
	{
		std::stringstream ss;

		ss << "Parsed LOGLEVEL is '" << loglevel_arg
			<< "' but must be a non-negative integer in the range "
			<< LOGLEVEL_MIN << "-"
			<< LOGLEVEL_MAX << ".";

		throw std::runtime_error(ss.str());

	} catch (const std::out_of_range &oor)
	{
		std::stringstream ss;

		ss << "Parsed LOGLEVEL is '" << loglevel_arg
			<< "' which is out of the valid range "
			<< LOGLEVEL_MIN << "-"
			<< LOGLEVEL_MAX << "";

		throw std::runtime_error(ss.str());
	}

	if (parsed_level < LOGLEVEL_MIN or parsed_level > LOGLEVEL_MAX)
	{
		std::stringstream ss;

		ss << "Parsed LOGLEVEL is '" << loglevel_arg
			<< "' which does not correspond to a valid loglevel ("
			<< LOGLEVEL_MIN << "-"
			<< LOGLEVEL_MAX << ").";

		throw std::runtime_error(ss.str());
	}

	// We could warn about -q overriding -v but we are quiet.

	auto log_level = LOGLEVEL::NONE;

	switch (parsed_level)
	{
		case 0: log_level = LOGLEVEL::NONE;    break;
		case 1: log_level = LOGLEVEL::ERROR;   break;
		case 2: log_level = LOGLEVEL::WARNING; break;
		case 3: log_level = LOGLEVEL::INFO;    break;
		case 4: log_level = LOGLEVEL::DEBUG;   break;
		case 5: log_level = LOGLEVEL::DEBUG1;  break;
		case 6: log_level = LOGLEVEL::DEBUG2;  break;
		case 7: log_level = LOGLEVEL::DEBUG3;  break;
		case 8: log_level = LOGLEVEL::DEBUG4;  break;
		default: {
			std::stringstream ss;

			ss << "Illegal value for log_level (must be in range "
				<< LOGLEVEL_MIN << "-"
				<< LOGLEVEL_MAX << ").";

			throw std::runtime_error(ss.str());
		}
	}

	return log_level;
}


// Options


Options::Options(const std::size_t size)
	: help_      { false }
	, version_   { false }
	, output_    {}
	, flags_     {}
	, values_    {}
	, arguments_ {}
{
	flags_.resize(size, false);
}


Options::~Options() noexcept = default;


void Options::set_help(const bool help)
{
	help_ = help;
}


bool Options::is_set_help() const
{
	return help_;
}


void Options::set_version(const bool version)
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


std::vector<bool> const Options::get_flags() const
{
	return flags_;
}


std::vector<std::string> const Options::get_arguments() const
{
	return arguments_;
}


std::string const Options::get_argument(const OptionCode &index) const
{
	if (index > arguments_.size())
	{
		return std::string();
	}

	return arguments_.at(index);
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


std::ostream& operator << (std::ostream& out, const Options &options)
{
	std::ios_base::fmtflags prev_settings = out.flags();

	out << "Options:" << std::endl;

	out << "Global: " << std::endl;
	out << "HELP:    " << std::boolalpha << options.is_set_help() << std::endl;
	out << "VERSION: " << std::boolalpha << options.is_set_version()
		<< std::endl;

	if (not options.output().empty())
	{
		out << "OUTPUT:  " << std::boolalpha << options.output();
	}

	out << "Options (w/o value):" << std::endl;
	auto opts = options.get_flags();
	for (auto i = std::size_t { 0 }; i < opts.size(); ++i)
	{
		out << std::setw(2) << i << ": "
			<< options.is_set(i) << std::endl;

		if (not options.get(i).empty())
		{
			out << "    = '" << options.get(i) << "'" << std::endl;
		}
	}

	out << "Arguments:" << std::endl;
	auto i = int { 0 };
	for (const auto& arg : options.get_arguments())
	{
		out << "Arg " << std::setw(2) << i << ": " << arg << std::endl;
		++i;
	}

	out.flags(prev_settings);

	return out;
}


// Configurator


const std::vector<Option> Configurator::global_options_ = {
	{ 'h', "help",      false, "FALSE", "Get help on usage" },
	{      "version",   false, "FALSE", "Print version and exit,"
	                                    " ignoring any other options." },
	{ 'v', "verbosity", true,  "2",     "Verbosity of output (loglevel 0-8)" },
	{ 'q', "quiet",     false, "FALSE", "Only output results, nothing else." },
	{ 'l', "logfile",   true,  "none",  "File for logging output" },
	{ 'o', "outfile",   true,  "none",  "File for result output" }
};


Configurator::Configurator()
	: logman_ {
		LogManager::get_loglevel(global(OPTION::VERBOSITY).default_arg()),
		LOGLEVEL::NONE /* quiet */
	  }
{
	// We do not know whether the application is required to run quiet,
	// so initial level is NOT default level but quiet level

	Logging::instance().set_level(logman_.quiet_loglevel());
	Logging::instance().set_timestamps(false);
}


Configurator::~Configurator() noexcept = default;


std::unique_ptr<Options> Configurator::provide_options(const int argc,
		const char* const * const argv)
{
	auto all_supported_options = this->all_supported();

	// Parse Input and Match Supported Options

	auto input = CLIInput { argc, argv, all_supported_options, true };

	auto options = this->process_global_options(input);

	// Now process the application specific (local) options

	for (const auto& item : input)
	{
		if (Option::NONE == item.id())
		{
			options->append(item.value()); // argument
		} else
		{
			options->set(item.id()); // option

			if (not item.value().empty())
			{
				options->put(item.id(), item.value());
			}
		}
	}

	ARCS_LOG_DEBUG << "Command line input successfully parsed";

	return this->do_configure_options(std::move(options));
}


const std::vector<Option>& Configurator::global_options()
{
	return Configurator::global_options_;
}


const std::vector<std::pair<Option, OptionCode>>&
	Configurator::supported_options() const
{
	return this->do_supported_options();
}


const Option& Configurator::global(const OptionCode c)
{
	return Configurator::global_options_[c - 1];
}


std::vector<std::pair<Option, OptionCode>> Configurator::all_supported() const
{
	std::vector<std::pair<Option, OptionCode>> all_supported =
		supported_options();

	// Add global options to supported options

	for (auto i = std::size_t { 0 }; i < global_options_.size(); ++i)
	{
		auto option = std::make_pair(
				global(static_cast<OptionCode>(i + 1)),  /* global option */
				global_id_[i] /* option code */);

		//std::cout << i << " - Global option --" << option.first.symbol()
		//	<< " is encoded with " << option.second << std::endl;

		all_supported.push_back(option);
	}

//	std::cout << "Supported options:" << std::endl;
//	for (auto i = std::size_t { 0 }; i < all_supported.size(); ++i)
//	{
//		std::cout << std::setw(2) << i << ": --" <<
//			all_supported[i].first.symbol()
//			<< " is encoded with "
//			<< all_supported[i].second << std::endl;
//	}

	return all_supported;
}


std::unique_ptr<Options> Configurator::process_global_options(
		const CLIInput &input) const
{
	auto options = std::make_unique<Options>();

	// --help

	if (input.contains(OPTION::HELP))
	{
		options->set(OPTION::HELP);
		return options;
	}

	// --version

	if (input.contains(OPTION::VERSION))
	{
		options->set(OPTION::VERSION);
		return options;
	}

	// --logfile (or stdout)

	std::unique_ptr<Appender> appender;
	auto appender_name = std::string {};

	if (input.contains(OPTION::LOGFILE))
	{
		auto logfile = input.value(OPTION::LOGFILE);
		appender = std::make_unique<Appender>(logfile);
		appender_name = appender->name();
	} else
	{
		appender = std::make_unique<Appender>("stdout", stdout);
		appender_name = appender->name();
	}

	Logging::instance().add_appender(std::move(appender));

	//std::cout << "Set log appender to " << appender_name << std::endl;

	// --verbosity

	if (input.contains(OPTION::VERBOSITY))
	{
		auto level = logman_.get_loglevel(
				input.value(OPTION::VERBOSITY));

		Logging::instance().set_level(level);
		Logging::instance().set_timestamps(false);

		//std::cout << "Set loglevel to "
		//	<< input.value(OPTION::VERBOSITY)
		//	<< std::endl;
	}

	// --quiet

	if (input.contains(OPTION::QUIET))
	{
		Logging::instance().set_level(logman_.quiet_loglevel());
		Logging::instance().set_timestamps(false);

		//std::cout << "Set quiet loglevel" << std::endl;
	}

	// --outfile,-o

	if (input.contains(OPTION::OUTFILE))
	{
		auto outfile = input.value(OPTION::OUTFILE);

		options->set(OPTION::OUTFILE);
	}

	return options;
}


std::unique_ptr<Options> Configurator::do_configure_options(
		std::unique_ptr<Options> options)
{
	// Default Implementation does Nothing

	return options;
}


// DefaultConfigurator


const std::vector<std::pair<Option, OptionCode>>&
	DefaultConfigurator::do_supported_options() const
{
	const static std::vector<std::pair<Option, OptionCode>> empty = {};
	return empty;
}

} // namespace arcsapp

