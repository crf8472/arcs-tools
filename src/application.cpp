#ifndef __ARCSTOOLS_APPLICATION_HPP__
#include "application.hpp"
#endif

#include <cstdlib>     // for EXIT_SUCCESS
#include <iostream>    // for cout
#include <memory>      // for unique_ptr, make_unique
#include <mutex>       // for mutex, lock_guard
#include <stdexcept>   // for invalid_argument, out_of_range, runtime_error
#include <string>      // for string, stoi
#include <utility>     // for get, move

#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#ifndef __ARCSTOOLS_CONFIG_HPP__
#include "config.hpp"          // for Configurator
#endif
#ifndef __ARCSTOOLS_RESULT_HPP__
#include "result.hpp"          // for Result
#endif
#ifndef __ARCSTOOLS_TABLE_HPP__
#include "table.hpp"           // for StringTable
#endif
#ifndef __ARCSTOOLS_VERSION_HPP__
#include "version.hpp"         // for ARCSTOOLS_VERSION
#endif

namespace arcsapp
{

// Output


Output::Output()
	: mutex_    { }
	, filename_ { }
	, append_   { false }
{
	// empty
}


bool Output::is_appending() const
{
	return append_;
}


void Output::set_append(const bool append)
{
	const std::lock_guard<std::mutex> lock(mutex_);
	append_ = append;
}


const std::string& Output::filename() const
{
	return filename_;
}


void Output::to_file(const std::string &filename)
{
	const std::lock_guard<std::mutex> lock(mutex_);
	filename_ = filename;
}


Output& Output::instance()
{
	static Output instance;

	return instance;
}


// Logging


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


// Application


class Options;

Application::Application() = default;


Application::~Application() noexcept = default;


std::string Application::name() const
{
	return this->do_name();
}


int Application::run(int argc, char** argv)
{
	if (argc == 1)
	{
		this->print_usage();
		return EXIT_SUCCESS;
	}

	auto configurator = this->create_configurator();

	auto options = configurator->provide_options(argc, argv);

	if (options->is_set(OPTION::HELP))
	{
		this->print_usage();
		return EXIT_SUCCESS;
	}

	if (options->is_set(OPTION::VERSION))
	{
		std::cout << this->name() << " " << ARCSTOOLS_VERSION << '\n';
		return EXIT_SUCCESS;
	}

	// Logging

	this->setup_logging(*options);

	ARCS_LOG(DEBUG1) << *options;

	// Output

	if (not options->value(OPTION::OUTFILE).empty())
	{
		Output::instance().to_file(options->value(OPTION::OUTFILE));
	}

	// Load configuration

	// Specific subclass function

	auto app_configuration = configurator->load(std::move(options));

	return this->do_run(*app_configuration);
}


void Application::print_usage() const
{
	std::cout << "Usage:" << '\n';

	// Print call syntax

	std::cout << this->do_name() << " " << this->do_call_syntax() << '\n';
	std::cout << '\n';

	// Print the options

	std::cout << "OPTIONS:" << '\n';

	const auto& options { this->create_configurator()->supported_options() };
	// Reference points to static local member

	table::StringTable table { static_cast<int>(options.size()), 3 };

	table.set_col_label(0, "Option");
	table.set_col_label(1, "Default");
	table.set_col_label(2, "Description");

	int row = 0;

	for (const auto& entry : options)
	{
		// Use option, discard code
		auto& option = std::get<1>(entry);

		// Add row
		table(row, 0) = option.tokens_str();
		table(row, 1) = option.default_arg();
		table(row, 2) = option.description();

		++row;
	}

	std::cout << table;
}


std::unique_ptr<Configurator> Application::create_configurator() const
{
	return this->do_create_configurator();
}


void Application::setup_logging(const Options& options) const
{
	// Activate logging:
	// The log options --logfile, --verbosity and --quiet take immediate effect
	// to have logging available as soon as possible, if requested.

	using arcstk::Logging;
	using arcstk::LOGLEVEL;
	using arcstk::Appender;

	// --logfile (or stdout)

	std::unique_ptr<Appender> appender;
	if (options.is_set(OPTION::LOGFILE))
	{
		appender = std::make_unique<Appender>(
				options.value(OPTION::LOGFILE));
	} else
	{
		appender = std::make_unique<Appender>("stdout", stdout);
	}
	Logging::instance().add_appender(std::move(appender));

	// --quiet, --verbosity

	if (!options.is_set(OPTION::QUIET))
	{
		// Set actual loglevel to either requested verbosity or default
		auto actual_loglevel = options.is_set(OPTION::VERBOSITY)
			? to_loglevel(options.value(OPTION::VERBOSITY))
			: LOGLEVEL::WARNING;

		Logging::instance().set_level(actual_loglevel);
	}

	ARCS_LOG_DEBUG << "Logging activated";
}


void Application::fatal_error(const std::string &message) const
{
	throw std::runtime_error(message);
}


void Application::output(std::unique_ptr<Result> result) const
{
	if (result)
	{
		Output::instance().output(*result);
	} else
	{
		ARCS_LOG_WARNING << "Will not output null pointer";
	}
}

} //namespace arcsapp

