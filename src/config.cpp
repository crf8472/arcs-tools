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


Configurator::Configurator(const int argc, const char* const * const argv)
	: logman_ {
		LogManager::get_loglevel(global(CONFIG::VERBOSITY).default_arg()),
		LOGLEVEL::NONE /* quiet */
	  }
	, tokens_(argc, argv)
{
	// We do not know whether the application is required to run quiet,
	// so initial level is NOT default level but quiet level

	Logging::instance().set_level(logman_.quiet_loglevel());
	Logging::instance().set_timestamps(false);
}


Configurator::~Configurator() noexcept = default;


void Configurator::configure_logging()
{
	auto level    = this->configure_loglevel();
	auto appender = this->configure_logappender();

	// All supported options regarding logging are now consumed and won't
	// occurr in the result of parse_options()

	ARCS_LOG(DEBUG1) << "Set loglevel to "
		<< Log::to_string(Logging::instance().level())
		<< " (=" << level << ") thereby consuming option "
		<< global(CONFIG::VERBOSITY).tokens_str();
}


std::unique_ptr<Options> Configurator::provide_options()
{
	auto options = this->parse_input(tokens_);

	ARCS_LOG_DEBUG << "Command line input successfully parsed";

	return this->do_configure_options(std::move(options));
}


std::unique_ptr<Options> Configurator::provide_options(const int argc,
		const char* const * const argv)
{
	// TODO Implement all_supported_options()

	std::vector<std::pair<Option, OptionCode>> all_supported =
		supported_options();

	// Add global options to supported options
	using config_t = std::underlying_type_t<Configurator::CONFIG>;

	for (auto i = config_t { 0 };
		i < static_cast<config_t>(Configurator::BASE_CODE()); ++i)
	{
		all_supported.emplace_back(
			std::make_pair(
				global(static_cast<CONFIG>(i)) /* global option i */,
				static_cast<config_t>(global_id[i]) /* code for option i */ ));
	}

	// Parse Input and Match Supported Options

	auto input = CLIInput { argc, argv, all_supported, true };

	auto options = std::make_unique<Options>();

	// Process global options
	// TODO Implement Options process_global_options(const CLIInput &input)

	if (input.contains(static_cast<config_t>(CONFIG::HELP)))
	{
		options->set_help(true);
		return options;
	}

	if (input.contains(static_cast<config_t>(CONFIG::VERSION)))
	{
		options->set_version(true);
		return options;
	}

	if (input.contains(static_cast<config_t>(CONFIG::VERBOSITY)))
	{
		auto level = logman_.get_loglevel(
				input.value(static_cast<config_t>(CONFIG::VERBOSITY)));

		Logging::instance().set_level(level);
		Logging::instance().set_timestamps(false);
	}

	if (input.contains(static_cast<config_t>(CONFIG::QUIET)))
	{
		Logging::instance().set_level(logman_.quiet_loglevel());
		Logging::instance().set_timestamps(false);
	}

	if (input.contains(static_cast<config_t>(CONFIG::LOGFILE)))
	{
		auto logfile = input.value(static_cast<config_t>(CONFIG::LOGFILE));

		std::unique_ptr<Appender> appender;

		if (logfile.empty())
		{
			// This defines the default!
			appender = std::make_unique<Appender>("stdout", stdout);
		} else
		{
			appender = std::make_unique<Appender>(logfile);
		}

		auto appender_name = appender->name();

		Logging::instance().add_appender(std::move(appender));

		ARCS_LOG(DEBUG1) << "Set log appender to " << appender_name
			<< " thereby consuming option "
			<<  global(CONFIG::LOGFILE).tokens_str();
	}

	if (input.contains(static_cast<config_t>(CONFIG::OUTFILE)))
	{
		auto outfile = input.value(static_cast<config_t>(CONFIG::OUTFILE));

		options->set_output(outfile);
	}

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


std::pair<bool, std::string> Configurator::option(CLITokens &tokens,
		const Option &option) const
{
	for (const auto& token : option.tokens())
	{
		if (const auto& [ found, value ] =
			tokens.consume(token, option.needs_value()); found)
		{
			if (option.needs_value() and value.empty())
			{
				throw CallSyntaxException(
						"Option " + token + " was passed without argument");
			}

			return std::make_pair(found, value);
		}
	}

	return std::make_pair(false, tokens.empty_value());
}


std::string Configurator::argument(CLITokens &tokens) const
{
	return tokens.consume();
}


std::vector<std::string> Configurator::arguments(CLITokens &tokens) const
{
	if (tokens.empty())
	{
		return std::vector<std::string>{};
	}

	auto arguments = std::vector<std::string> {};
	arguments.reserve(tokens.unconsumed().size());

	while (not tokens.empty())
	{
		arguments.push_back(this->argument(tokens));
	}

	return arguments;
}


int Configurator::configure_loglevel()
{
	// current loglevel is NONE (since QUIET could have been requested)

	const auto& [ is_quiet, noval ] =
		this->option(tokens_, global(CONFIG::QUIET));

	const auto& [ verbosity_defined, parsed_level ] =
		this->option(tokens_, global(CONFIG::VERBOSITY));
	// quiet overrides verbosity, but verbosity must be consumed, however

	if (is_quiet)
	{
		Logging::instance().set_level(logman_.quiet_loglevel());
	} else if (verbosity_defined)
	{
		Logging::instance().set_level(logman_.get_loglevel(parsed_level));
	} else
	{
		Logging::instance().set_level(logman_.default_loglevel());
	}

	Logging::instance().set_timestamps(false);

	return static_cast<std::underlying_type<LOGLEVEL>::type>(
				Logging::instance().level());
}


std::tuple<std::string, std::string> Configurator::configure_logappender()
{
	const auto& [ found, logfile ] = this->option(tokens_, global(CONFIG::LOGFILE));

	std::unique_ptr<Appender> appender;

	if (logfile.empty())
	{
		// This defines the default!
		appender = std::make_unique<Appender>("stdout", stdout);
	} else
	{
		appender = std::make_unique<Appender>(logfile);
	}

	auto appender_name = appender->name();

	Logging::instance().add_appender(std::move(appender));

	ARCS_LOG(DEBUG1) << "Set log appender to " << appender_name
			<< " thereby consuming option "
			<<  global(CONFIG::LOGFILE).tokens_str();

	return std::make_tuple(appender_name, logfile);
}


std::unique_ptr<Options> Configurator::parse_options(CLITokens& tokens)
{
	// The --version flag is magic, the parsing can be stopped because it is
	// known at this point what the application has to do.

	if (const auto& [ found, value ] =
			this->option(tokens, global(CONFIG::VERSION)); found)
	{
		Options options;
		options.set_version(true);

		return std::make_unique<Options>(options);
	}

	std::unique_ptr<Options> parsed_options = std::make_unique<Options>();

	// Consume any supported option

	for (const auto& entry : supported_options())
	{
		auto& option = std::get<0>(entry);
		auto& id     = std::get<1>(entry);

		ARCS_LOG(DEBUG2) << "Check for option: " << option.tokens_str();

		// Try to consume supported options und assign their ids

		if (const auto& [ found, value ] = this->option(tokens, option); found)
		{
			parsed_options->set(id);

			if (option.needs_value())
			{
				parsed_options->put(id, value);
			}
		}

		if (Logging::instance().has_level(LOGLEVEL::DEBUG))
		{
			auto print_fragment = std::ostringstream { };

			if (auto utokens = tokens.unconsumed(); not utokens.empty())
			{
				std::copy (utokens.begin(), utokens.end() - 1,
					std::ostream_iterator<std::string>(print_fragment, ",")
				);
				print_fragment << utokens.back();
			} // XXX This trick is duplicated in options.cpp/Options::token_str

			ARCS_LOG(DEBUG2) << "Tokens left: " << print_fragment.str();
		}
	}

	// Add builtin option: outfile

	if (const auto& [ found, outfile ] =
		this->option(tokens, global(CONFIG::OUTFILE)); found)
	{
		if (outfile.empty())
		{
			throw CallSyntaxException("Option "
				+ global(CONFIG::OUTFILE).tokens_str() +
				" requires the name of an outfile");
		}

		parsed_options->set_output(outfile);
	}

	return parsed_options;
}


int Configurator::parse_arguments(CLITokens& tokens, Options &options) const
{
	return this->do_parse_arguments(tokens, options);
}


std::unique_ptr<Options> Configurator::parse_input(CLITokens& tokens)
{
	auto options = this->parse_options(tokens);

	this->parse_arguments(tokens, *options);

	// Finish Processing of the Command Line Input

	if (not tokens.empty())
	{
		std::stringstream ss;

		ss << "Unrecognized command line tokens: ";

		for (const auto& token : tokens.unconsumed())
		{
			ss << "'" << token << "' ";
		}

		throw CallSyntaxException(ss.str());
	}

	return options;
}


const Option& Configurator::global(const CONFIG c)
{
	return Configurator::global_options_[std::underlying_type_t<CONFIG>(c)];
}



int Configurator::do_parse_arguments(CLITokens& tokens, Options &options) const
{
	// allow only single argument

	auto arg = this->argument(tokens);

	if (arg.empty())
	{
		throw CallSyntaxException("Argument expected");
	}

	options.append(arg);

	return 1;
}


std::unique_ptr<Options> Configurator::do_configure_options(
		std::unique_ptr<Options> options)
{
	// default implementation does nothing

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

