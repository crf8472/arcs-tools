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


Options::Options()
	: options_   {}
	, arguments_ {}
{
	// empty
}


Options::~Options() noexcept = default;


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

	// Reset default Loglevel to value defined by option
	const auto& options { common_options() };
	if (options.size() < OPTION::VERBOSITY)
	{
		const auto& verbosity {
			std::get<0>(common_options()[OPTION::VERBOSITY])
		};
		auto lvl = to_loglevel(verbosity.default_arg());
		Logging::instance().set_level(lvl);
	}

	Logging::instance().set_timestamps(false);
}


Configurator::~Configurator() noexcept = default;


std::unique_ptr<Options> Configurator::provide_options(const int argc,
		const char* const * const argv) const
{
	// Parse Input and Match Supported Options

	auto cli_options = std::make_unique<Options>();
	{
		const auto add_option =
			[&cli_options](const OptionCode c, const std::string& v)
			{
				if (Option::NONE == c)
				{
					cli_options->put_argument(v);
				} else
				{
					cli_options->set(c, v);
				}
			};
		auto cli_input = CLITokens {};
		cli_input.register_callback(add_option);
		cli_input.parse(argc, argv, supported_options());
	}
	//

	const auto& supported_options = this->supported_options();

	// Parse Input and Match Supported Options

	auto input = CLITokens { argc, argv, supported_options };
	auto options = std::make_unique<Options>();

	// Activate logging:
	// The log options --logfile, --verbosity and --quiet take immediate effect
	// to have logging available as soon as possible, if requested.
	{
		// --logfile (or stdout)

		std::unique_ptr<Appender> appender;

		if (input.contains(OPTION::LOGFILE))
		{
			appender = std::make_unique<Appender>(input.value(OPTION::LOGFILE));
		} else
		{
			appender = std::make_unique<Appender>("stdout", stdout);
		}

		Logging::instance().add_appender(std::move(appender));

		// --quiet, --verbosity

		if (not input.contains(OPTION::QUIET))
		{
			if (input.contains(OPTION::VERBOSITY))
			{
				Logging::instance().set_level(to_loglevel(
							input.value(OPTION::VERBOSITY)));
			}
		} // else do nothing since LOGLEVEL::NONE i.e. --quiet is default

		// TODO Make configurable
		//Logging::instance().set_timestamps(false);
	}

	// Convert input to Options object

	for (const auto& token : input)
	{
		if (Option::NONE == token.code())
		{
			options->put_argument(token.value());
		} else
		{
			options->set(token.code(), token.value());
		}
	}

	ARCS_LOG_DEBUG << "Command line input successfully parsed";

	// Reset verbosity to actual log level
	// XXX WHY THIS??
	{
		const auto actual_level = static_cast<std::underlying_type_t<LOGLEVEL>>(
				Logging::instance().level());

		options->set(OPTION::VERBOSITY, std::to_string(actual_level));
	}

	return this->do_configure_options(std::move(options));
}


const std::vector<std::pair<Option, OptionCode>>&
	Configurator::supported_options() const
{
	return this->do_supported_options();
}


const Configurator::OptionRegistry& Configurator::common_options()
{
	const static OptionRegistry common_options = {
		{{ 'h', "help",      false, "FALSE", "Get help on usage" },
			OPTION::HELP },

		{{      "version",   false, "FALSE", "Print version and exit,"
											" ignoring any other options." },
			OPTION::VERSION },

		{{ 'v', "verbosity", true,  "2", "Verbosity of output (loglevel 0-8)" },
			OPTION::VERBOSITY },

		{{ 'q', "quiet",     false, "FALSE", "Only output results, "
			"nothing else." },
			OPTION::QUIET },

		{{ 'l', "logfile",   true,  "none",  "File for logging output" },
			OPTION::LOGFILE },

		{{ 'o', "outfile",   true,  "none",  "File for result output" },
			OPTION::OUTFILE }
	};

	return common_options;
}


void Configurator::flush_common_options_to(OptionRegistry& r) const
{
	using std::begin;
	using std::end;

	std::copy(
		begin(Configurator::common_options()),
		end(Configurator::common_options()),
		std::back_inserter(r)
	);
}


std::unique_ptr<Options> Configurator::do_configure_options(
		std::unique_ptr<Options> options) const
{
	// Default Implementation does nothing

	return options;
}


// DefaultConfigurator


const std::vector<std::pair<Option, OptionCode>>&
	DefaultConfigurator::do_supported_options() const
{
	return Configurator::common_options();
}


// FORMATBASE


constexpr OptionCode FORMATBASE::LIST_TOC_FORMATS;
constexpr OptionCode FORMATBASE::LIST_AUDIO_FORMATS;
constexpr OptionCode FORMATBASE::READERID;
constexpr OptionCode FORMATBASE::PARSERID;

constexpr OptionCode FORMATBASE::SUBCLASS_BASE;

} // namespace arcsapp

