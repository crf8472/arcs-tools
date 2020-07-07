#ifndef __ARCSTOOLS_CONFIG_HPP__
#include "config.hpp"
#endif

#include <cstdio>              // for stdout
#include <iterator>            // for ostream_iterator
#include <sstream>             // for operator<<, basic_ostream::operator<<
#include <string>              // for char_traits, operator<<, operator+
#include <type_traits>         // for add_const<>::type, __underlying_type_i...
#include <utility>             // for move
#include <vector>              // for vector

#include <iostream>

#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#ifndef __ARCSTOOLS_CLITOKENS_HPP__
#include "clitokens.hpp"
#endif
#ifndef __ARCSTOOLS_OPTIONS_HPP__
#include "options.hpp"         // for Options, __ARCSTOOLS_OPTIONS_HPP__
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


const Option& Configurator::global(const OPTION c)
{
	return Configurator::global_options_[std::underlying_type_t<OPTION>(c) - 1];
}


std::vector<std::pair<Option, OptionCode>> Configurator::all_supported() const
{
	std::vector<std::pair<Option, OptionCode>> all_supported =
		supported_options();

	// Add global options to supported options
	using config_t = std::underlying_type_t<Configurator::OPTION>;

	for (auto i = std::size_t { 0 }; i < global_options_.size(); ++i)
	{
		auto option = std::make_pair(
				global(static_cast<OPTION>(i + 1)),  /* global option */
				static_cast<config_t>(global_id_[i]) /* option code */);

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
	using config_t = std::underlying_type_t<Configurator::OPTION>;

	// --help

	if (input.contains(static_cast<config_t>(OPTION::HELP)))
	{
		std::cout << "Help" << std::endl;
		options->set_help(true);
		return options;
	}

	// --version

	if (input.contains(static_cast<config_t>(OPTION::VERSION)))
	{
		std::cout << "Version" << std::endl;
		options->set_version(true);
		return options;
	}

	// --logfile (or stdout)

	std::unique_ptr<Appender> appender;
	auto appender_name = std::string {};

	if (input.contains(static_cast<config_t>(OPTION::LOGFILE)))
	{
		auto logfile = input.value(static_cast<config_t>(OPTION::LOGFILE));
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

	if (input.contains(static_cast<config_t>(OPTION::VERBOSITY)))
	{
		auto level = logman_.get_loglevel(
				input.value(static_cast<config_t>(OPTION::VERBOSITY)));

		Logging::instance().set_level(level);
		Logging::instance().set_timestamps(false);

		//std::cout << "Set loglevel to "
		//	<< input.value(static_cast<config_t>(OPTION::VERBOSITY))
		//	<< std::endl;
	}

	// --quiet

	if (input.contains(static_cast<config_t>(OPTION::QUIET)))
	{
		Logging::instance().set_level(logman_.quiet_loglevel());
		Logging::instance().set_timestamps(false);

		//std::cout << "Set quiet loglevel" << std::endl;
	}

	// --outfile,-o

	if (input.contains(static_cast<config_t>(OPTION::OUTFILE)))
	{
		auto outfile = input.value(static_cast<config_t>(OPTION::OUTFILE));

		options->set_output(outfile);
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

