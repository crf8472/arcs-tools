#ifndef __ARCSTOOLS_CONFIG_HPP__
#include "config.hpp"
#endif

#include <cstddef>       // for size_t
#include <iomanip>       // for setw
#include <ios>           // for fmtflags
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
	this->set(option, std::string{}); // TODO Use a constant
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
		return o->second;
	}

	return std::string{}; // TODO Use a constant
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


std::ostream& operator << (std::ostream& out, const Options &options)
{
	std::ios_base::fmtflags prev_settings = out.flags();

	out << "Options:" << std::endl;

	out << "Options (w/o value):" << std::endl;
	for (const auto& entry : options.options_)
	{
		out << std::setw(2) << entry.first;
		if (!entry.second.empty())
		{
			out << " = '" << entry.second << "'" << std::endl;
		} else
		{
			out << " is set" << std::endl;
		}
	}

	out << "Arguments:" << std::endl;
	auto i = int { 0 };
	for (const auto& arg : options.arguments())
	{
		out << "Arg " << std::setw(2) << i << ": '" << arg << "'" << std::endl;
		++i;
	}

	out.flags(prev_settings);

	return out;
}


// Configurator


Configurator::~Configurator() noexcept = default;


std::unique_ptr<Options> Configurator::provide_options(const int argc,
		const char* const * const argv) const
{
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

	// Let verbosity reflect the --quiet request

	if (options->is_set(OPTION::QUIET))
	{
		// Overwrite value for --verbosity, if any
		options->set(OPTION::VERBOSITY, "0");
	}

	return this->do_configure_options(std::move(options));
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

