#ifndef __ARCSTOOLS_APPLICATION_HPP__
#include "application.hpp"
#endif

#include <algorithm>           // for copy
#include <cstdlib>             // for EXIT_SUCCESS
#include <iterator>            // for ostream_iterator
#include <iostream>            // for cout
#include <memory>              // for unique_ptr
#include <set>                 // for set
#include <sstream>             // for ostringstream
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
#include "format.hpp"          // for StringTable
#endif
#ifndef __ARCSTOOLS_VERSION_HPP__
#include "version.hpp"         // for ARCSTOOLS_VERSION_INFO
#endif

namespace arcsapp
{

class Options;

using arcsdec::FileFormat;


// FormatCollector


FormatCollector::FormatCollector()
	: table_ { 0, 4 }
{
	table_.set_title(0, "Name");
	table_.set_width(0, table_.title(0).length() + 6);
	table_.set_alignment(0, true);

	table_.set_title(1, "Short Desc.");
	table_.set_width(1, table_.title(1).length() + 4);
	table_.set_alignment(1, true);

	table_.set_title(2, "Lib");
	table_.set_width(2, table_.title(2).length() + 5);
	table_.set_alignment(2, true);

	table_.set_title(3, "Version");
	table_.set_width(3, table_.title(3).length());
	table_.set_alignment(3, true);
}


void FormatCollector::add(const FileReaderDescriptor &descriptor)
{
	// FIXME Implement this (currently a mess, just a sketch)

	auto name = descriptor.name();

	// Description: for now, just concatenate the format names
	std::ostringstream desc;
	const auto& formats = descriptor.formats();
	if (!formats.empty())
	{
		std::transform(formats.begin(), formats.rbegin().base(),
			std::ostream_iterator<std::string>(desc, ","),
			[](const arcsdec::FileFormat &format) -> std::string
			{
				return arcsdec::name(format);
			});
		desc << arcsdec::name(*formats.rbegin());
	}

	// FIXME Get the name of the library used at runtime from libarcsdec

	// FIXME Get the version of the library used at runtime from libarcsdec

	int row = table_.current_row();

	// Add row
	table_.update_cell(row, 0, name);
	table_.update_cell(row, 1, desc.str());
	table_.update_cell(row, 2, "-");
	table_.update_cell(row, 3, "-");

	// Adjust col width to optimal width after each row
	for (std::size_t col = 0; col < table_.columns(); ++col)
	{
		if (static_cast<std::size_t>(table_.width(col))
			< table_(row, col).length())
		{
			table_.set_width(col, table_(row, col).length());
		}
	}

	++row;
}


StringTable FormatCollector::info() const
{
	return table_;
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
	if (argc == 1)
	{
		this->print_usage();

		return EXIT_SUCCESS;
	}

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
	std::cout << "Usage:" << std::endl;

	// Print call syntax

	std::cout << this->do_name() << " " << this->do_call_syntax() << std::endl;
	std::cout << std::endl;

	// Print the options

	std::cout << "OPTIONS:" << std::endl;

	const auto& goptions { Configurator::global_options() };
	const auto& loptions {
		this->create_configurator(0, nullptr)->supported_options() };

	StringTable table { static_cast<int>(goptions.size() + loptions.size()),
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

		// Adjust col width to optimal width after each row
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

	// Print app specific options
	for (const auto& entry : loptions)
	{
		auto& option = std::get<0>(entry);

		// Add row
		table.update_cell(row, 0, option.tokens_str());
		table.update_cell(row, 1, option.default_arg());
		table.update_cell(row, 2, option.description());

		// Adjust col width to optimal width after each row
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

} //namespace arcsapp

