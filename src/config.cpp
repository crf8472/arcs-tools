#ifndef __ARCSTOOLS_CONFIG_HPP__
#include "config.hpp"
#endif

#include <cstdio>        // for stdout
#include <iterator>      // for ostream_iterator
#include <map>           // for map
#include <sstream>       // for ostringstream, operator<<
#include <string>        // for char_traits, operator<<, operator+
#include <type_traits>   // for add_const<>::type, __underlying_type_i...
#include <utility>       // for move
#include <vector>        // for vector

#include <iostream> // debug

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


// ConfigurationException


ConfigurationException::ConfigurationException(const std::string &what_arg)
	: std::runtime_error(what_arg)
{
	// empty
}


// Options


Options::Options()
	: options_   {}
	, arguments_ {}
{
	// empty
}


Options::~Options() noexcept = default;


bool Options::is_set(const OptionCode &option) const
{
	return options_.find(option) != options_.end();
}


void Options::set(const OptionCode &option)
{
	this->set(option, std::string{});
}


void Options::set(const OptionCode &option, const std::string &value)
{
	auto rc = options_.insert(std::make_pair(option, value));

	if (not rc.second) // Insertion failed
	{
		std::ostringstream msg;
		msg << "Option '" << option << "' with value '" << value
			<< "' could not be stored in configuration instance"
			<< ", prevented by option '" << rc.first->first
			<< "' with value '" << rc.first->second << "'." << std::endl;
		throw ConfigurationException(msg.str());
	}
}


void Options::unset(const OptionCode &option)
{
	auto o = options_.find(option);

	if (o != options_.end())
	{
		options_.erase(o);
	}
}


std::string Options::value(const OptionCode &option) const
{
	auto o = options_.find(option);

	if (o != options_.end())
	{
		return o->second;
	}

	return std::string();
}


void Options::put_argument(const std::string &argument)
{
	arguments_.push_back(argument);
}


std::vector<std::string> const Options::arguments() const
{
	return arguments_;
}


std::string const Options::argument(const std::size_t index) const
{
	if (index >= arguments_.size())
	{
		return std::string{};
	}

	return arguments_.at(index);
}


bool Options::no_arguments() const
{
	return arguments_.empty();
}


bool Options::empty() const
{
	return options_.empty() and arguments_.empty();
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

	out << "Options (w/o value):" << std::endl;
	for (const auto& entry : options.options_)
	{
		out << "Option: " << entry.first << " = " << entry.second << std::endl;
	}

	out << "Arguments:" << std::endl;
	auto i = int { 0 };
	for (const auto& arg : options.arguments())
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
	: logman_ { /* Define the default log level and the quiet log level */
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

	// The log options --logfile, --verbosity and --quiet are
	// immediately

	this->activate_logging(input);

	// Convert input to Options object

	auto options = std::make_unique<Options>();

	for (const auto& item : input)
	{
		if (Option::NONE == item.id())
		{
			options->put_argument(item.value());
		} else
		{
			options->set(item.id(), item.value());
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
	// TODO Find a way not to copy the statically stored options.
	// It's all internal data and should be accessible without copy.

	std::vector<std::pair<Option, OptionCode>> all_supported =
		supported_options();

	// Add global options to supported options

	for (auto i = std::size_t { 0 }; i < global_options_.size(); ++i)
	{
		auto option = std::make_pair(
				global(static_cast<OptionCode>(i + 1)), /* global option */
				global_id_[i] /* option code */);

		all_supported.push_back(option);
	}

	return all_supported;
}


void Configurator::activate_logging(const CLIInput &input) const
{
	// --logfile (or stdout)

	std::unique_ptr<Appender> appender;

	if (input.contains(OPTION::LOGFILE)
			and not input.value(OPTION::LOGFILE).empty())
	{
		appender = std::make_unique<Appender>(input.value(OPTION::LOGFILE));
	} else
	{
		appender = std::make_unique<Appender>("stdout", stdout);
	}

	Logging::instance().add_appender(std::move(appender));

	// --quiet, --verbosity

	if (input.contains(OPTION::QUIET))
	{
		Logging::instance().set_level(logman_.quiet_loglevel());

	} else if (input.contains(OPTION::VERBOSITY))
	{
		Logging::instance().set_level(logman_.get_loglevel(
					input.value(OPTION::VERBOSITY)));
	} else
	{
		Logging::instance().set_level(logman_.default_loglevel());
	}

	// TODO Make configurable
	Logging::instance().set_timestamps(false);
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

