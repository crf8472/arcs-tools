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

#ifndef __ARCSTOOLS_APPLICATION_HPP__
#include "application.hpp"
#endif
#ifndef __ARCSTOOLS_CONFIG_HPP__
#include "config.hpp"          // for Configurator, CallSyntaxException
#endif
#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#include "layouts.hpp"          // for StringTable
#endif
#ifndef __ARCSTOOLS_VERSION_HPP__
#include "version.hpp"         // for ARCSTOOLS_VERSION_INFO
#endif

namespace arcsapp
{

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

	// This may throw a CallSyntaxException
	auto options = this->setup_options(argc, argv);

	if (options->is_set(OPTION::HELP))
	{
		this->print_usage();

		return EXIT_SUCCESS;
	}

	if (options->is_set(OPTION::VERSION))
	{
		std::cout << this->name() << " " << ARCSTOOLS_VERSION_INFO << std::endl;

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

	const auto& goptions { Configurator::global_options() };
	const auto& soptions { this->create_configurator()->supported_options() };

	StringTable table { static_cast<int>(goptions.size() + soptions.size()),
		3 };

	table.set_title(0, "Option");
	table.set_width(0, table.title(0).length());
	table.set_alignment(0, true);

	table.set_title(1, "Default");
	table.set_width(1, table.title(1).length());
	table.set_alignment(1, true);

	table.set_title(2, "Description");
	table.set_width(2, table.title(2).length());
	table.set_alignment(2, true);

	int row = 0;

	// Print global options
	for (const auto& option : goptions)
	{
		// Add row
		table.update_cell(row, 0, option.tokens_str());
		table.update_cell(row, 1, option.default_arg());
		table.update_cell(row, 2, option.description());

		++row;
	}

	// Print app specific options
	for (const auto& entry : soptions)
	{
		auto& option = std::get<0>(entry);

		// Add row
		table.update_cell(row, 0, option.tokens_str());
		table.update_cell(row, 1, option.default_arg());
		table.update_cell(row, 2, option.description());

		++row;
	}

	std::cout << table;
}


std::unique_ptr<Options> Application::setup_options(int argc, char** argv)
	const
{
	auto configurator = this->create_configurator();

	return configurator->provide_options(argc, argv);
}


void Application::fatal_error(const std::string &message) const
{
	//ARCS_LOG_ERROR << message;
	throw std::runtime_error(message);
}


} //namespace arcsapp

