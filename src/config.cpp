#ifndef __ARCSTOOLS_CONFIG_HPP__
#include "config.hpp"
#endif

#include <cstdio>              // for stdout
#include <sstream>             // for operator<<, basic_ostream::operator<<
#include <string>              // for char_traits, operator<<, operator+
#include <type_traits>         // for add_const<>::type, __underlying_type_i...
#include <utility>             // for move
#include <vector>              // for vector

#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#ifndef __ARCSTOOLS_CLIPARSE_HPP__
#include "cliparse.hpp"
#endif
#ifndef __ARCSTOOLS_OPTIONS_HPP__
#include "options.hpp"         // for Options, __ARCSTOOLS_OPTIONS_HPP__
#endif


using arcstk::Appender;
using arcstk::Logging;
using arcstk::Log;
using arcstk::LOGLEVEL;

#include <iostream> // TODO remove this

class LogSettingsParser
{
public:

	LOGLEVEL default_loglevel();

	LOGLEVEL get_loglevel(CLIParser &cli);

	std::string get_logfile(CLIParser &cli);

private:

	const LOGLEVEL default_loglevel_ = LOGLEVEL::WARNING;
};


LOGLEVEL LogSettingsParser::default_loglevel()
{
	return default_loglevel_;
}


LOGLEVEL LogSettingsParser::get_loglevel(CLIParser &cli)
{
	using arcstk::LOGLEVEL_MIN;
	using arcstk::LOGLEVEL_MAX;

	const auto& [ v_found, loglevel_arg ] = cli.consume_valued_option("-v");

	if (not v_found)
	{
		return default_loglevel();
	}

	if (loglevel_arg.empty())
	{
		throw CallSyntaxException(
				"Option -v was passed without argument");
	}

	int level = -1;

	try {

		level = std::stoi(loglevel_arg);

	} catch (const std::invalid_argument &ia)
	{
		std::stringstream ss;

		ss << "Argument of -v is '" << loglevel_arg
			<< "' but must be a non-negative integer in the range "
			<< LOGLEVEL_MIN << "-"
			<< LOGLEVEL_MAX << ".";

		throw std::runtime_error(ss.str());

	} catch (const std::out_of_range &oor)
	{
		std::stringstream ss;

		ss << "Argument of -v is '" << loglevel_arg
			<< "' which is out of the valid range "
			<< LOGLEVEL_MIN << "-"
			<< LOGLEVEL_MAX << "";

		throw std::runtime_error(ss.str());
	}

	if (level < LOGLEVEL_MIN or level > LOGLEVEL_MAX)
	{
		std::stringstream ss;

		ss << "Argument of -v is '" << loglevel_arg
			<< "' which does not correspond to a valid loglevel ("
			<< LOGLEVEL_MIN << "-"
			<< LOGLEVEL_MAX << ").";

		throw std::runtime_error(ss.str());
	}

	// We could warn about -q overriding -v but we are quiet.

	LOGLEVEL log_level = default_loglevel();

	switch (level)
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

			ss << "Illegal value for log_level"
				<< LOGLEVEL_MIN << "-"
				<< LOGLEVEL_MAX << ".";

			throw std::runtime_error(ss.str());
		}
	}

	return log_level;
}


std::string LogSettingsParser::get_logfile(CLIParser &cli)
{
	const auto& [ l_found, logfile_arg ]  = cli.consume_valued_option("-l");

	if (l_found)
	{
		if (logfile_arg.empty())
		{
			throw CallSyntaxException(
				"Option -l was passed without argument");
		}
	}

	return logfile_arg;
}


// CallSyntaxException


CallSyntaxException::CallSyntaxException(const std::string &what_arg)
	: std::runtime_error(what_arg)
{
	// empty
}


// Configurator


Configurator::Configurator(int argc, char** argv)
	: cli_(argc, argv)
{
	Logging::instance().set_level(LOGLEVEL::NONE);
	Logging::instance().set_timestamps(false);
}


Configurator::~Configurator() noexcept = default;


void Configurator::configure_logging()
{
	this->do_configure_logging();
}


void Configurator::check_for_option(const std::string cli_option,
		const uint8_t option, CLIParser &cli, Options &options) const
{
	if (cli.consume_option(cli_option))
	{
		options.set(option);
	}
}


void Configurator::check_for_option_with_argument(const std::string cli_option,
		const uint8_t option, CLIParser &cli, Options &options)
{
	if (const auto& [ found, value ] =
			cli.consume_valued_option(cli_option); found)
	{
		if (value.empty())
		{
			throw CallSyntaxException(
					"Option " + cli_option + " was passed without argument");
		}

		options.set(option);
		options.put(option, value);
	}
}


int Configurator::configure_loglevel()
{
	// current level is NONE

	LogSettingsParser log_parser;

	auto log_level = log_parser.get_loglevel(cli_);

	if (not cli_.consume_option("-q"))
	{
		Logging::instance().set_level(log_level);
		Logging::instance().set_timestamps(false);
	}

	return static_cast<std::underlying_type<LOGLEVEL>::type>(
				Logging::instance().level());
}


std::tuple<std::string, std::string> Configurator::configure_logappender()
{
	LogSettingsParser log_parser;

	auto logfile = log_parser.get_logfile(cli_);

	std::unique_ptr<Appender> appender;

	if (logfile.empty())
	{
		appender = std::make_unique<Appender>("stdout", stdout);
	} else
	{
		appender = std::make_unique<Appender>(logfile);
	}

	auto appender_name = appender->name();

	Logging::instance().add_appender(std::move(appender));

	return std::make_tuple(appender_name, logfile);
}


void Configurator::do_configure_logging()
{
	auto level = this->configure_loglevel();

	auto appender = this->configure_logappender();

	ARCS_LOG_DEBUG << "Set loglevel: "
		<< Log::to_string(Logging::instance().level())
		<< " thereby consuming option -v";

	ARCS_LOG_DEBUG << "Set first log appender thereby consuming option -l "
		<< std::get<1>(appender);
}


std::unique_ptr<Options> Configurator::configure_options()
{
	// The --version flag is magic, the parsing can be stopped because it is
	// known at this point what the application has to do.

	if (cli_.consume_option("--version"))
	{
		Options options;
		options.set_version(true);

		return std::make_unique<Options>(options);
	}

	auto options = this->parse_options(cli_);

	// Finish Processing of the Command Line Input

	if (cli_.tokens_left())
	{
		auto tokens = cli_.get_unconsumed_tokens();

		std::stringstream ss;

		ss << "Unrecognized command line tokens: ";

		for (const auto& token : tokens)
		{
			ss << "'" << token << "' ";
		}

		throw CallSyntaxException(ss.str());
	}

	return this->do_configure_options(std::move(options));
}

