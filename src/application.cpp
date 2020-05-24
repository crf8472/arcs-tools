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
#ifndef __ARCSTOOLS_FORMAT_HPP__
#include "format.hpp"
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

	if (options->empty())
	{
		this->print_usage();

		return EXIT_SUCCESS;
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
	std::cout << "Usage:" << std::endl;
	std::cout << this->do_name() << " " << this->do_call_syntax() << std::endl;
	std::cout << std::endl;

	std::cout << "Options:" << std::endl;

	auto options { this->create_configurator(0, nullptr)->supported() };

	// Print the options

	StringTable table { static_cast<int>(options.size()), 3 };

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
	for (const auto& option : options)
	{
		table.update_cell(row, 0, option.tokens_str());
		table.update_cell(row, 1, option.default_arg());
		table.update_cell(row, 2, option.description());

		for (std::size_t col = 0; col < table.columns(); ++col)
		{
			if (static_cast<std::size_t>(table.width(col))
				< table(row, col).length())
			{
				table.set_width(col, table(row, col).length());
			}
		}

		++row;
	}

	std::cout << table;
}


std::unique_ptr<Options> ARApplication::setup_options(int argc, char** argv)
	const
{
	auto configurator = this->create_configurator(argc, argv);

	configurator->configure_logging();

	return configurator->provide_options();
}


void ARApplication::fatal_error(const std::string &message) const
{
	throw std::runtime_error(message);
}
