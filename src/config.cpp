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
using arcstk::LOGLEVEL;


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
	// empty
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


void Configurator::do_configure_logging()
{
	LOGLEVEL log_level = LOGLEVEL::WARNING; // default

	bool is_quiet = false;

	if (cli_.consume_option("-q"))
	{
		log_level = LOGLEVEL::NONE;

		is_quiet = true;
	}

	// Loglevel
	{
		// -q takes precedence over -v, but -v must nonetheless be parsed

		const auto& [ found, loglevel_arg ] = cli_.consume_valued_option("-v");

		if (found)
		{
			if (loglevel_arg.empty())
			{
				throw CallSyntaxException(
						"Option -v was passed without argument");
			}

			const int min_log_level = 0;
			const int max_log_level = 6;

			int level = -1;
			try {

				level = std::stoi(loglevel_arg);

			} catch (const std::invalid_argument &ia)
			{
				std::stringstream ss;

				ss << "Argument of -v is '" << loglevel_arg
					<< "' but must be a non-negative integer in the range "
					<< min_log_level << "-"
					<< max_log_level << ".";

				throw std::runtime_error(ss.str());

			} catch (const std::out_of_range &oor)
			{
				std::stringstream ss;

				ss << "Argument of -v is '" << loglevel_arg
					<< "' which is out of the valid range "
					<< min_log_level << "-"
					<< max_log_level << "";

				throw std::runtime_error(ss.str());
			}

			if (level < min_log_level or level > max_log_level)
			{
				std::stringstream ss;

				ss << "Argument of -v is '" << loglevel_arg
					<< "' which does not correspond to a valid loglevel ("
					<< min_log_level << "-"
					<< max_log_level << ").";

				throw std::runtime_error(ss.str());
			}

			// loglevel index validates, assign the corresponding level

			if (not is_quiet)
			{
				switch (level)
				{
					case 0: log_level = LOGLEVEL::NONE; break;
					case 1: log_level = LOGLEVEL::ERROR; break;
					case 2: log_level = LOGLEVEL::WARNING; break;
					case 3: log_level = LOGLEVEL::INFO; break;
					case 4: log_level = LOGLEVEL::DEBUG; break;
					case 5: log_level = LOGLEVEL::DEBUG1; break;
					case 6: log_level = LOGLEVEL::DEBUG2; break;
					default: {
						std::stringstream ss;

						ss << "Illegal value for log_level"
							<< min_log_level << "-"
							<< max_log_level << ".";

						throw std::runtime_error(ss.str());
					}
				}
			}
			// We could warn about -q overriding -v but we are quiet.
		}
	} //Loglevel

	// Logfile
	{
		std::unique_ptr<Appender> appender;
		const auto& [ found, logfile ] = cli_.consume_valued_option("-l");

		if (found)
		{
			if (logfile.empty())
			{
				throw CallSyntaxException(
						"Option -l was passed without argument");
			}

			appender = std::make_unique<Appender>(logfile);

		} else
		{
			appender = std::make_unique<Appender>("stdout", stdout);

		}

		Logging::instance().add_appender(std::move(appender));
	} //Logfile

	Logging::instance().set_level(log_level);
	Logging::instance().set_timestamps(false);

	ARCS_LOG_DEBUG << "Loglevel " <<
		static_cast<std::underlying_type<LOGLEVEL>::type>(
				Logging::instance().level());
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

