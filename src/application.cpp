#ifndef __ARCSTOOLS_APPLICATION_HPP__
#include "application.hpp"
#endif

#include <iostream>
#include <fstream>
#include <memory>

#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#ifndef __ARCSTOOLS_CLIPARSE_HPP__
#include "cliparse.hpp"
#endif
#ifndef __ARCSTOOLS_OPTIONS_HPP__
#include "options.hpp"
#endif
#ifndef __ARCSTOOLS_VERSION_HPP__
#include "version.hpp"
#endif


// ARApplication


ARApplication::ARApplication() = default;


ARApplication::~ARApplication() noexcept = default;


std::string ARApplication::name() const
{
	return this->do_name();
}


int ARApplication::run(int argc, char** argv)
{
	std::unique_ptr<Options> options;

	// Print error message and usage if application call line is ill-formed
	try
	{
		options = this->setup_options(argc, argv);

	} catch (const CallSyntaxException &e)
	{
		this->print_usage();

		throw e;
	}

	if (options->is_set_version())
	{
		std::cout << this->name() << " " << ARCSTOOLS_VERSION_INFO << std::endl;

		return EXIT_SUCCESS;
	}

	return this->do_run(*options);
}


void ARApplication::print_usage() const
{
	std::cout << "Usage:\n";

	this->do_print_usage();
}


std::unique_ptr<Options> ARApplication::setup_options(int argc, char** argv)
	const
{
	auto configurator = this->create_configurator(argc, argv);

	configurator->configure_logging();

	return configurator->configure_options();
}


void ARApplication::fatal_error(const std::string &message) const
{
	throw std::runtime_error(message);
}


void ARApplication::output(const StringTable &table) const
{
	// TODO table empty
	//if (table.empty())
	//{
	//	return;
	//}

	std::cout << table;
}


void ARApplication::output(const StringTable &table, const std::string &filename)
	const
{
	if (filename.empty())
	{
		output(table);
		return;
	}

	std::ofstream out_file_stream;
	out_file_stream.open(filename);
	out_file_stream << table;
}

