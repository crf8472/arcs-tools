#ifndef __ARCSTOOLS_APPARVERIFY_HPP__
#include "app-verify.hpp"
#endif

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <filesystem>
#include <memory>
#include <string>

#ifndef __LIBARCSTK_MATCH_HPP__
#include <arcstk/match.hpp>
#endif
#ifndef __LIBARCSTK_PARSE_HPP__
#include <arcstk/parse.hpp>
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#ifndef __LIBARCSDEC_CALCULATORS_HPP__
#include <arcsdec/calculators.hpp>
#endif

#ifndef __ARCSTOOLS_ARCALC_HPP__
#include "app-calc.hpp"
#endif
#ifndef __ARCSTOOLS_OUTPUTFORMATS_HPP__
#include "outputformats.hpp"
#endif
#ifndef __ARCSTOOLS_PARSEHANDLERS_HPP__
#include "parsehandlers.hpp"
#endif
#ifndef __ARCSTOOLS_CLIPARSE_HPP__
#include "cliparse.hpp"
#endif
#ifndef __ARCSTOOLS_OPTIONS_HPP__
#include "options.hpp"
#endif
#ifndef __ARCSTOOLS_TOOLS_CALC_HPP__
#include "tools_calc.hpp"
#endif
#ifndef __ARCSTOOLS_TOOLS_FS_HPP__
#include "tools_fs.hpp"
#endif


using arcstk::ARStreamParser;
using arcstk::ARFileParser;
using arcstk::ARId;
using arcstk::ARResponse;
using arcstk::ARStdinParser;
using arcstk::Checksums;
using arcstk::DefaultContentHandler;
using arcstk::DefaultErrorHandler;
using arcstk::Matcher;
using arcstk::TracksetMatcher;
using arcstk::AlbumMatcher;

using arcsdec::ARCSCalculator;


// ARVerifyOptions


constexpr uint16_t ARVerifyOptions::RESPONSEFILE;


// ARVerifyConfigurator


ARVerifyConfigurator::ARVerifyConfigurator(int argc, char** argv)
	: ARCalcConfigurator(argc, argv)
{
	// empty
}


std::unique_ptr<Options> ARVerifyConfigurator::do_configure_options(
		std::unique_ptr<Options> options)
{
	if (options->empty())
	{
		throw CallSyntaxException("No options or arguments");
	}

	// Does output file exist?

	{
		std::string outfile = options->get(ARVerifyOptions::OUT);

		if (not outfile.empty() and file::file_exists(outfile))
		{
			ARCS_LOG_WARNING << "Outfile " << outfile << " exists.";
		}
	}

	return options;
}


std::unique_ptr<Options> ARVerifyConfigurator::parse_options(CLIParser& cli)
{
	// A hack to reuse ARCalcConfigurator for the common subset of options:
	// parse_options() empties the cli and whatever options are not recognized
	// by ARCalcConfigurator will occur as arguments. To prevent this, we must
	// consume every legal option that is specific for the verify application,
	// BEFORE calling parse_options. Only thereafter we add the consumed options
	// to the Options object.

	// Response Filename

	const auto& [ found, response_file ] = cli.consume_valued_option("-r");

	if (found and response_file.empty())
	{
		throw CallSyntaxException("Option -r is passed without argument");
	}

	// Parse calc application options and arguments

	auto options = ARCalcConfigurator::parse_options(cli);

	// Add postponed options from the verify application

	if (not response_file.empty())
	{
		options->set(ARVerifyOptions::RESPONSEFILE);
		options->put(ARVerifyOptions::RESPONSEFILE, response_file);
	}

	return options;
}


// ARVerifyApplication


void ARVerifyApplication::log_matching_files(const Checksums &checksums,
		const Match &match, const uint32_t block,
		const bool version) const
{
	auto unmatched { checksums.size() };

	// Traverse checksums
	for (std::size_t t = 0; t < checksums.size() and unmatched > 0; ++t)
	{
		// Traverse specified block
		for (int track = 0; track < match.tracks_per_block(); ++track)
		{
			if (match.track(block, track, version))
			{
				ARCS_LOG_DEBUG << "Pos " << track << " matches track "
					<< (track+1) << " in block " << block;

				--unmatched;
			}
		}
	}
}


std::unique_ptr<Configurator> ARVerifyApplication::create_configurator(
		int argc, char** argv) const
{
	return std::make_unique<ARVerifyConfigurator>(argc, argv);
}


std::string ARVerifyApplication::do_name() const
{
	return "verfiy";
}


int ARVerifyApplication::do_run(const Options &options)
{
	// Parse the AccurateRip response

	ARResponse response; // Response object to be build be content handler

	// Parse reference data to response object
	{
		std::string responsefile(options.get(ARVerifyOptions::RESPONSEFILE));

		std::unique_ptr<ARStreamParser> parser;

		if (responsefile.empty())
		{
			ARCS_LOG_DEBUG << "Parse response from stdin";

			parser = std::make_unique<ARStdinParser>();
		} else
		{
			ARCS_LOG_DEBUG << "Parse response from file " << responsefile;

			auto file_parser { std::make_unique<ARFileParser>() };
			file_parser->set_file(responsefile);

			parser = std::move(file_parser);
		}

		auto c_handler { std::make_unique<DefaultContentHandler>() };
		c_handler->set_object(response);
		parser->set_content_handler(std::move(c_handler));
		parser->set_error_handler(std::make_unique<DefaultErrorHandler>());

		try
		{
			if (parser->parse() == 0)
			{
				this->fatal_error("No bytes parsed, exit");
			}
		} catch (const std::exception& e)
		{
			this->fatal_error(e.what());
		}

		ARCS_LOG_DEBUG << "Response object created";
	}


	std::unique_ptr<const Matcher> diff;
	std::unique_ptr<AlbumMatchTableFormat> format;

	// Calculate the actual ARCSs from input files

	auto [ checksums, arid, toc ] = ARCalcApplication::calculate(options);

	if (checksums.size() == 0)
	{
		this->fatal_error("Calculation returned no checksums.");
	}


	if (const auto metafilename = options.get(ARCalcOptions::METAFILE);
			metafilename.empty())
	{
		// No Offsets => No ARId => No TOC
		// These files may or may not form an album. If it is an album, it must
		// have been requested as such from cli. However, we have no metadata
		// and will therefore only print filename, length and checksum.

		auto audiofilenames = options.get_arguments();

		diff = std::make_unique<TracksetMatcher>(checksums, response);

		this->log_matching_files(checksums, *diff->match(), 1, true);

		const int rows = std::max(checksums.size(),
				static_cast<std::size_t>(response.tracks_per_block()));

		format = std::make_unique<AlbumMatchTableFormat>(
				rows, false, false, true, true);

		format->format(checksums, response, *diff->match(),
				diff->best_match(), diff->matches_v2(), audiofilenames);
	} else
	{
		// With Offsets, ARId and Result

		if (arid.empty())
		{
			ARCS_LOG_ERROR << "Calculation returned no identifier.";
		}
		if (!toc)
		{
			ARCS_LOG_ERROR << "Calculation returned no TOC.";
		}

		const auto& [ single_audio_file, pw_distinct ] = audiofile_layout(*toc);

		diff = std::make_unique<AlbumMatcher>(checksums, arid, response);

		format = std::make_unique<AlbumMatchTableFormat>(
				checksums.size(), true, true, true, !single_audio_file);

		format->format(checksums, response, *diff->match(),
				diff->best_match(), diff->matches_v2(), arid, std::move(toc));
	}


	// Print match results

	if (diff->matches())
	{
		ARCS_LOG_INFO << "Response contains a total match (v"
			<< (diff->matches_v2() + 1) << ") to the input tracks in block "
			<< diff->best_match();
	} else
	{
		ARCS_LOG_INFO << "Best match was block "  << diff->best_match()
			<< " in response, having difference " << diff->best_difference();
	}

	this->print(*format->lines(), options.get(ARCalcOptions::OUT));

	return EXIT_SUCCESS;
}


void ARVerifyApplication::do_print_usage() const
{
	std::cout << this->name();
	std::cout << " [OPTIONS] <filename1> [ <filename2> ... ]\n\n";

	std::cout << "Options:\n\n";

	std::cout << "-q            only output result, nothing else\n\n";
	std::cout << "-v <i>        verbous output (loglevel 0-6)\n\n";
	std::cout << "-l <filename> specify log file for output\n\n";
	std::cout << "-m <filename> specify metadata file (CUE) to use\n\n";
	std::cout << "-o <filename> specify output file\n\n";
	std::cout << "-r <filename> specify response file to match against\n\n";
	std::cout << "--v1          only compare ARCS v1\n\n";
	std::cout << "--v2          only compare ARCS v2\n\n";
	std::cout << "--first       treat first audio file as first track\n";
	std::cout << "              (implied if -m is specified)\n\n";
	std::cout << "--last        treat last audio file as last track\n";
	std::cout << "              (implied if -m is specified)\n\n";
	std::cout << "--album       short for --first --last\n";
	std::cout << "              (implied if -m is specified)\n\n";
	std::cout << "--version     print version and exit\n";
	std::cout << "              (ignoring any other options)\n\n";
}

