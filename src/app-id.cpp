#ifndef __ARCSTOOLS_APPARID_HPP__
#include "app-id.hpp"
#endif

#include <cstdlib>                  // for EXIT_SUCCESS
#include <fstream>                  // for operator<<, ostream, ofstream
#include <iostream>                 // for cout
#include <memory>                   // for unique_ptr, make_unique, allocator
#include <string>                   // for string, char_traits, operator<<

#ifndef __LIBARCSTK_IDENTIFIER_HPP__
#include <arcstk/identifier.hpp>
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#ifndef __LIBARCSDEC_CALCULATE_HPP__
#include <arcsdec/calculators.hpp>
#endif

#ifndef __ARCSTOOLS_CLIPARSE_HPP__
#include "cliparse.hpp"
#endif
#ifndef __ARCSTOOLS_CONFIG_HPP__
#include "config.hpp"
#endif
#ifndef __ARCSTOOLS_OPTIONS_HPP__
#include "options.hpp"
#endif
#ifndef __ARCSTOOLS_OUTPUTFORMATS_HPP__
#include "outputformats.hpp"
#endif


using arcstk::ARId;

using arcsdec::ARIdCalculator;


// ARIdOptions


ARIdOptions::ARIdOptions() = default;


// ARIdConfigurator


ARIdConfigurator::ARIdConfigurator(int argc, char** argv)
	: Configurator(argc, argv)
{
	// empty
}


std::unique_ptr<Options> ARIdConfigurator::do_configure_options(
		std::unique_ptr<Options> options)
{
	if (options->empty())
	{
		return std::make_unique<Options>();
	}

	// Default function is URL

	/*
	if (    not options->is_set(ARIdOptions::CDDBID )
		and not options->is_set(ARIdOptions::URL    )
		and not options->is_set(ARIdOptions::DBID   )
		and not options->is_set(ARIdOptions::PROFILE))
	{
		options->set(ARIdOptions::URL);
	}
	*/

	// Infile specified?

	std::string infilename(options->get_argument(0));

	if (infilename.empty())
	{
		ARCS_LOG_ERROR << "No input file specified";
		return std::make_unique<Options>();
	}

	return options;
}


std::unique_ptr<Options> ARIdConfigurator::parse_options(CLIParser &cli)
{
	auto options = std::make_unique<ARIdOptions>();

	this->check_for_option("--cddb_id", ARIdOptions::CDDBID,
			cli, *options);
	this->check_for_option("--url", ARIdOptions::URL,
			cli, *options);
	this->check_for_option("--db_id", ARIdOptions::DBID,
			cli, *options);
	this->check_for_option("--profile", ARIdOptions::PROFILE,
			cli, *options);

	this->check_for_option_with_argument("-a", ARIdOptions::AUDIOFILE,
			cli, *options);
	this->check_for_option_with_argument("-o", ARIdOptions::OUT,
			cli, *options);
	this->check_for_option_with_argument("--url_prefix", ARIdOptions::PRE,
			cli, *options);

	// Input Metadata Filename

	if (std::string filename { cli.consume_argument() }; filename.empty())
	{
		throw CallSyntaxException(
				"Filename of a readable metadata file required");
	} else
	{
		options->push_back_argument(filename);
	}

	return options;
}


// ARIdApplication


std::unique_ptr<Configurator> ARIdApplication::create_configurator(
		int argc, char** argv) const
{
	return std::make_unique<ARIdConfigurator>(argc, argv);
}


std::string ARIdApplication::do_name() const
{
	return "id";
}


int ARIdApplication::do_run(const Options &options)
{
	// Compute requested values

	std::string metafilename  = options.get_argument(0);
	std::string audiofilename = options.get(ARIdOptions::AUDIOFILE);

	std::unique_ptr<ARId> id = nullptr;

	{
		ARIdCalculator calculator;
		id = calculator.calculate(audiofilename, metafilename);
	}

	if (!id) { this->fatal_error("Could not compute AccurateRip id."); }

	// Adjust format and print information
	std::unique_ptr<ARIdPrinter> format;

	if (options.is_set(ARIdOptions::PROFILE))
	{
		format = std::make_unique<ARIdTableFormat>(
				true, true, true, true, true, true);
	} else
	{
		format = std::make_unique<ARIdTableFormat>(
			options.is_set(ARIdOptions::URL),
			options.is_set(ARIdOptions::DBID),
			false /* no track count */,
			false /* no disc id 1 */,
			false /* no disc id 2 */,
			options.is_set(ARIdOptions::CDDBID)
		);
	}

	// Configure output stream

	std::streambuf *buf;
	std::ofstream out_file_stream;
	if (auto filename = options.get(ARIdOptions::OUT); filename.empty())
	{
		buf = std::cout.rdbuf();
	} else
	{
		out_file_stream.open(filename);
		buf = out_file_stream.rdbuf();
	}
	std::ostream out_stream(buf);

	format->out(out_stream, *id, options.get(ARIdOptions::PRE));

	return EXIT_SUCCESS;
}


void ARIdApplication::do_print_usage() const
{
	std::cout << this->name() << " [OPTIONS] FILENAME\n\n";
	std::cout << "Where FILENAME is the name of some metadata file "
		<< "(CUESheet)\n\n";
	std::cout << "Options:\n\n";
	std::cout << "--cddb_id       print the CDDB id\n\n";
	std::cout << "--url           print the AccurateRip URL\n\n";
	std::cout << "--db_id         print the AccurateRip DB ID (filename)\n\n";
	std::cout << "--profile       print all available meta information\n\n";
	std::cout << "--url_prefix    use the given prefix instead of the default\n";
	std::cout << "                'http://accuraterip.com/accuraterip/'\n\n";
	std::cout << "-a <filename>   specify audio file explicitly\n\n";
	std::cout << "-q              only output parsed content, nothing else\n\n";
	std::cout << "-v <i>          verbous output (loglevel 0-6)\n\n";
	std::cout << "-l <filename>   specify log file for output\n\n";
	std::cout << "--version       print version and exit\n";
	std::cout << "                (ignoring any other options)\n";
}

