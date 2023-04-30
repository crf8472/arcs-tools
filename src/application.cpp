#ifndef __ARCSTOOLS_APPLICATION_HPP__
#include "application.hpp"
#endif

#include <cstdlib>             // for EXIT_SUCCESS
#include <iostream>            // for cout
#include <memory>              // for unique_ptr
#include <stdexcept>           // for runtime_error

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
	: mutex_ {}
	, filename_ {}
	, append_ { false }
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


class Options;


// Application


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

	// FIXME: This may throw a CallSyntaxException
	auto options = this->setup_options(argc, argv);

	if (options->is_set(OPTION::HELP))
	{
		this->print_usage();

		return EXIT_SUCCESS;
	}

	if (options->is_set(OPTION::VERSION))
	{
		std::cout << this->name() << " " << ARCSTOOLS_VERSION << std::endl;

		return EXIT_SUCCESS;
	}

	return this->do_run(*options);
}


void Application::print_usage() const
{
	std::cout << "Usage:" << std::endl;

	// Print call syntax

	std::cout << this->do_name() << " " << this->do_call_syntax() << std::endl;
	std::cout << std::endl;

	// Print the options

	std::cout << "OPTIONS:" << std::endl;

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


std::unique_ptr<Options> Application::setup_options(int argc, char** argv) const
{
	auto configurator = this->create_configurator();

	return configurator->provide_options(argc, argv);
}


void Application::fatal_error(const std::string &message) const
{
	//ARCS_LOG_ERROR << message; // Commented out, just a reminder
	throw std::runtime_error(message);
}


void Application::output(std::unique_ptr<Result> result, const Options &options)
	const
{
	auto object { std::move(result) };

	if (!object)
	{
		return;
	}

	if (not options.value(OPTION::OUTFILE).empty())
	{
		// Save previous state of Output and restore it after use
		const auto filename { Output::instance().filename() };
		Output::instance().to_file(options.value(OPTION::OUTFILE));

		// Perform output
		Output::instance().output(*object);

		// Restore previous state
		Output::instance().to_file(filename);
	} else
	{
		Output::instance().output(*object);
	}
}


} //namespace arcsapp

