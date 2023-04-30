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

#ifndef __LIBARCSDEC_SELECTION_HPP__
#include <arcsdec/selection.hpp>        // for FileReaderSelection
										//     FileReaderPreferenceSelection
#endif

#ifndef __ARCSTOOLS_CLITOKENS_HPP__
#include "clitokens.hpp"                // for CLITokens
#endif

namespace arcsapp
{

using arcstk::Appender;
using arcstk::Logging;
using arcstk::LOGLEVEL;


/**
 * \brief LOGLEVEL from a string representation.
 *
 * The string is expected to consist of digit symbols.
 *
 * \param[in] lvl_str  A string
 *
 * \throw ConfigurationException Conversion to loglevel failed
 *
 * \return The deduced log level
 */
LOGLEVEL to_loglevel(const std::string &lvl_str);


LOGLEVEL to_loglevel(const std::string &lvl_str)
{
	using arcstk::LOGLEVEL_MIN;
	using arcstk::LOGLEVEL_MAX;

	auto parsed_level = int { -1 };

	try {

		parsed_level = std::stoi(lvl_str);

	} catch (const std::invalid_argument &ia)
	{
		std::ostringstream ss;

		ss << "Parsed LOGLEVEL is '" << lvl_str
			<< "' but must be a non-negative integer in the range "
			<< LOGLEVEL_MIN << "-"
			<< LOGLEVEL_MAX << ".";

		throw ConfigurationException(ss.str());

	} catch (const std::out_of_range &oor)
	{
		std::ostringstream ss;

		ss << "Parsed LOGLEVEL is '" << lvl_str
			<< "' which is out of the valid range "
			<< LOGLEVEL_MIN << "-"
			<< LOGLEVEL_MAX << "";

		throw ConfigurationException(ss.str());
	}

	if (parsed_level < LOGLEVEL_MIN or parsed_level > LOGLEVEL_MAX)
	{
		std::ostringstream ss;

		ss << "Parsed LOGLEVEL is '" << lvl_str
			<< "' which does not correspond to a valid loglevel ("
			<< LOGLEVEL_MIN << "-"
			<< LOGLEVEL_MAX << ").";

		throw ConfigurationException(ss.str());
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
			std::ostringstream ss;

			ss << "Illegal value for log_level (must be in range "
				<< LOGLEVEL_MIN << "-"
				<< LOGLEVEL_MAX << ").";

			throw ConfigurationException(ss.str());
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


bool Options::is_set(const OptionCode &option) const
{
	using std::end;
	return options_.find(option) != end(options_);
}


void Options::set(const OptionCode &option)
{
	this->set(option, std::string{});
}


void Options::set(const OptionCode &option, const std::string &value)
{
	if (OPTION::NONE == option)
	{
		throw ConfigurationException("Cannot set OPTION::NONE");
	}

	auto rc { options_.insert(std::make_pair(option, value)) };

	if (not rc.second) // Insertion failed, but option value can be updated
	{
		rc.first->second = value;
	}
}


void Options::unset(const OptionCode &option)
{
	auto o { options_.find(option) };

	if (o != options_.end())
	{
		options_.erase(o);
	}
}


std::string Options::value(const OptionCode &option) const
{
	auto o { options_.find(option) };

	if (o != options_.end())
	{
		return o->second;
	}

	return std::string{};
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


Configurator::Configurator()
{
	// We do not know whether the application is required to run quiet,
	// so initial level is NOT default level but quiet level
	Logging::instance().set_level(LOGLEVEL::NONE);

	// TODO Make configurable
	Logging::instance().set_timestamps(false);
}


Configurator::~Configurator() noexcept = default;


std::unique_ptr<Options> Configurator::provide_options(const int argc,
		const char* const * const argv) const
{
	// Parse Commandline Input and Match Supported Options

	auto cli_options = std::make_unique<Options>();
	{
		const auto add_option =
			[&cli_options](const OptionCode c, const std::string& v)
			{
				if (OPTION::NONE == c)
				{
					cli_options->put_argument(v);
				} else
				{
					cli_options->set(c, v);
				}
			};

		input::parse(argc, argv, supported_options(), add_option);

		ARCS_LOG_DEBUG << "Command line input successfully parsed";
	}


	// Activate logging:
	// The log options --logfile, --verbosity and --quiet take immediate effect
	// to have logging available as soon as possible, if requested.
	{
		// --logfile (or stdout)

		std::unique_ptr<Appender> appender;

		if (cli_options->is_set(OPTION::LOGFILE))
		{
			appender = std::make_unique<Appender>(
					cli_options->value(OPTION::LOGFILE));
		} else
		{
			appender = std::make_unique<Appender>("stdout", stdout);
		}

		Logging::instance().add_appender(std::move(appender));

		// --quiet, --verbosity

		if (cli_options->is_set(OPTION::QUIET))
		{
			// Let verbosity reflect the --quiet request
			cli_options->set(OPTION::VERBOSITY, "0");

			// Since initial loglevel is NONE, Logging::instance() is fine
		} else
		{
			// Set actual loglevel to either requested verbosity or default
			auto actual_loglevel = cli_options->is_set(OPTION::VERBOSITY)
				? to_loglevel(cli_options->value(OPTION::VERBOSITY))
				: LOGLEVEL::WARNING;

			Logging::instance().set_level(actual_loglevel);
		}

		ARCS_LOG_DEBUG << "Activate Logging";
	}

	return this->do_configure_options(std::move(cli_options));
}


OptionRegistry Configurator::supported_options() const
{
	auto options { common_options() };
	this->flush_local_options(options);
	return options;
}


OptionRegistry Configurator::common_options() const
{
	return {
		{ OPTION::HELP,
			{ 'h', "help", false, "FALSE", "Get help on usage" } },

		{ OPTION::VERSION,
			{   "version", false, "FALSE", "Print version and exit,"
			" ignoring any other options." } },

		{ OPTION::VERBOSITY,
			{ 'v', "verbosity", true,  "2", "Verbosity of output (loglevel 0-8)"
			}} ,

		{ OPTION::QUIET,
			{ 'q', "quiet", false, "FALSE", "Only output results, "
			"nothing else." }},

		{ OPTION::LOGFILE,
			{ 'l', "logfile",   true,  "none",  "File for logging output" }},

		{ OPTION::OUTFILE,
			{ 'o', "outfile",   true,  "none",  "File for result output" }}
	};
}


std::unique_ptr<Options> Configurator::do_configure_options(
		std::unique_ptr<Options> options) const
{
	// Default Implementation does nothing

	return options;
}


// DefaultConfigurator


void DefaultConfigurator::flush_local_options(OptionRegistry& r) const
{
	// empty
}


// FORMATBASE


constexpr OptionCode FORMATBASE::LIST_TOC_FORMATS;
constexpr OptionCode FORMATBASE::LIST_AUDIO_FORMATS;
constexpr OptionCode FORMATBASE::READERID;
constexpr OptionCode FORMATBASE::PARSERID;

constexpr OptionCode FORMATBASE::SUBCLASS_BASE;

} // namespace arcsapp

