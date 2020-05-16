#ifndef __ARCSTOOLS_APPLICATION_HPP__
#include "application.hpp"
#endif

#include <cstdlib>             // for EXIT_SUCCESS
#include <fstream>             // for operator<<, basic_ostream, char_traits
#include <iostream>            // for cout
#include <memory>              // for unique_ptr
#include <set>                 // for set
#include <stdexcept>           // for runtime_error
#include <vector>              // for vector

#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#ifndef __ARCSTOOLS_APPLICATION_HPP__
#include "application.hpp"
#endif
#ifndef __ARCSTOOLS_CLIPARSE_HPP__
#include "cliparse.hpp"        // for __ARCSTOOLS_CLIPARSE_HPP__
#endif
#ifndef __ARCSTOOLS_CONFIG_HPP__
#include "config.hpp"          // for Configurator, CallSyntaxException
#endif
#ifndef __ARCSTOOLS_VERSION_HPP__
#include "version.hpp"         // for ARCSTOOLS_VERSION_INFO
#endif

class Options;

using arcsdec::FileFormat;


// FormatCollector


FormatCollector::FormatCollector()
	: info_ {}
{
	// empty
}


void FormatCollector::add(const FileReaderDescriptor &descriptor)
{
	auto name = descriptor.name();

	auto formats = descriptor.formats();

	std::stringstream desc;
	for (const auto& f : formats)
	{
		desc << arcsdec::name(f) << ",";
	}

	info_.push_back( { name, desc.str(), "-", "-" } );
}


std::vector<std::array<std::string, 4>> FormatCollector::info() const
{
	return info_;
}


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

