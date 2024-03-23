#ifndef __ARCSTOOLS_CONFIG_HPP__
#include "config.hpp"
#endif

#include <algorithm>     // for replace
#include <cstddef>       // for size_t
#include <iomanip>       // for setw
#include <memory>        // for unique_ptr, make_unique
#include <ostream>       // for ostream, endl, operator<<
#include <stdexcept>     // for runtime_error
#include <string>        // for string
#include <utility>       // for make_pair, move
#include <vector>        // for vector

#ifndef __ARCSTOOLS_CLITOKENS_HPP__
#include "clitokens.hpp"    // for parse
#endif

namespace arcsapp
{
inline namespace v_1_0_0
{

// ConfigurationException


ConfigurationException::ConfigurationException(const std::string &what_arg)
	: std::runtime_error(what_arg)
{
	// empty
}


// log_cli_input


void log_cli_input(const Options& options, const OptionRegistry& registry)
{
	ARCS_LOG(DEBUG1) << "Command line options:";

	for (const auto& [code, value] : options.options_)
	{
		for (const auto& p : registry)
		{
			if (code == p.first)
			{
				ARCS_LOG(DEBUG1) << "--" << p.second.symbol()
					<< " (" << code << ")"
					<< " = "
					<< (value.empty() ? "TRUE" : "'" + value + "'");
				break;
			}
		}
	}

	ARCS_LOG(DEBUG1) << "Command line arguments:";

	auto i = int { 0 };
	for (const auto& arg : *options.arguments())
	{
		ARCS_LOG(DEBUG1) << "Arg " << std::setw(2) << i << ": '" << arg << "'";
		++i;
	}
}


// Options


bool Options::is_set(const OptionCode &option) const
{
	using std::end;
	return options_.find(option) != end(options_);
}


void Options::set(const OptionCode &option)
{
	this->set(option, std::string{}); // TODO Use a constant
}


void Options::set(const OptionCode &option, const std::string &value)
{
	if (OPTION::NONE == option)
	{
		throw ConfigurationException("Cannot set OPTION::NONE");
	}

	const auto& [pos, done] { options_.insert(std::make_pair(option, value)) };

	if (not done) // Insertion failed, but option value can be updated
	{
		pos->second = value;
	}
}


void Options::unset(const OptionCode &option)
{
	const auto o { options_.find(option) };

	using std::end;

	if (o != end(options_))
	{
		options_.erase(o);
	}
}


std::string Options::value(const OptionCode &option) const
{
	const auto o { options_.find(option) };

	using std::end;

	if (o != end(options_))
	{
		return o->second/* value */;
	}

	return std::string{}; // TODO Use a constant
}


void Options::put_argument(const std::string &argument)
{
	arguments_.push_back(argument);
}


const std::vector<std::string>* Options::arguments() const
{
	return &arguments_;
}


std::string Options::argument(const std::size_t index) const
{
	if (index >= arguments_.size())
	{
		return std::string{}; // TODO Use a constant
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


// StringParser


std::any StringParser::parse(const std::string& s) const
{
	ARCS_LOG(DEBUG1) << "=> " << start_message();
	return this->do_parse(s);
}


// Configurator


Configurator::~Configurator() noexcept = default;


std::unique_ptr<Options> Configurator::read_options(const int argc,
		const char* const * const argv) const
{
	// Logging is not yet possible here

	auto options = std::make_unique<Options>();
	{
		const auto add_option =
			[&options](const OptionCode c, const std::string& v)
			{
				// Discard dashes
				if (input::DASH == c || input::DDASH == c) { return; }

				if (input::ARGUMENT == c)
				{
					options->put_argument(v);
				} else
				{
					options->set(c, v);
				}
			};

		input::parse(argc, argv, supported_options(), add_option);
	}

	// --quiet is just an alias for -v 0.

	if (options->is_set(OPTION::QUIET))
	{
		options->set(OPTION::VERBOSITY, "0");
	}

	return options;
}


std::unique_ptr<Options> Configurator::configure_options(
		std::unique_ptr<Options> options) const
{
	options = this->do_configure_options(std::move(options));
	do_validate(*options);
	return options;
}


OptionRegistry Configurator::supported_options() const
{
	auto options { common_options() };
	this->do_flush_local_options(options);
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


void Configurator::do_validate(const Options& o) const
{
	// Default implementation will just validate the options
	// without any checks
}


OptionParsers Configurator::do_parser_list() const
{
	// Default Implementation does not specify parseable options

	return {};
}


std::unique_ptr<Options> Configurator::do_configure_options(
		std::unique_ptr<Options> options) const
{
	// Default Implementation does nothing

	return options;
}


std::unique_ptr<Configuration> Configurator::create(
		std::unique_ptr<Options> options) const
{
	auto config = std::make_unique<Configuration>(std::move(options));

	apply_parsers(*config);
	do_validate(*config);

	return config;
}


void Configurator::apply_parsers(Configuration& config) const
{
	// Parse input strings to objects

	for (const auto& [option, load] : do_parser_list())
	{
		if (config.is_set(option))
		{
			ARCS_LOG_DEBUG << "Parse input string for option " << option;

			config.put(option, load()->parse(config.value(option)));

			ARCS_LOG_DEBUG << "Successfully parsed input string for option "
				<< option;
		}
	}
}


void Configurator::do_validate(const Configuration& c) const
{
	// Default implementation will just validate the configuration
	// without any checks
}


// Configuration


Configuration::Configuration(std::unique_ptr<Options> options)
	: options_ { std::move(options) }
	, objects_ { /* empty */ }
{
	// empty
}


void Configuration::put(const OptionCode &option, const std::any& object)
{
	objects_[option] = object;
}


bool Configuration::is_set(const OptionCode &option) const
{
	return options_->is_set(option);
}


std::string Configuration::value(const OptionCode &option) const
{
	return options_->value(option);
}


std::string Configuration::argument(const std::size_t i) const
{
	return options_->argument(i);
}


const std::vector<std::string>* Configuration::arguments() const
{
	return options_->arguments();
}


bool Configuration::no_arguments() const
{
	return options_->no_arguments();
}


// DefaultConfigurator


void DefaultConfigurator::do_flush_local_options(OptionRegistry& r) const
{
	// empty
}


// FORMATBASE


constexpr OptionCode FORMATBASE::LIST_TOC_FORMATS;
constexpr OptionCode FORMATBASE::LIST_AUDIO_FORMATS;
constexpr OptionCode FORMATBASE::READERID;
constexpr OptionCode FORMATBASE::PARSERID;

constexpr OptionCode FORMATBASE::SUBCLASS_BASE;


//


void parse_list(const std::string& list, const char delim,
		std::function<void(const std::string& s)> value_hook)
{
	if (list.empty())
	{
		return;
	}

	auto in { list }; // copy

	// replace delimiters by spaces
	if (delim != ' ')
	{
		using std::begin;
		using std::end;
		std::replace(begin(in), end(in), delim, ' ');
	}
	// FIXME If the cli input list contains spaces, parsing will break
	// Like "a:b,c:d e,f:g" (with quotes containing spaces)?
	// Parsed as: a:b,c:d,e,f:g
	// In this example, "c" will not have the value "d e" but "d" and "e" will
	// be a name instead of a value having no value by itself.

	auto input = std::istringstream { in };
	auto value = std::string {};

	while (input >> value)
	{
		value_hook(value);
		value.clear();
	}
}

} // namespace v_1_0_0
} // namespace arcsapp

