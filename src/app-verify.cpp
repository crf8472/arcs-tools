#ifndef __ARCSTOOLS_APPARVERIFY_HPP__
#include "app-verify.hpp"
#endif

#include <cstdlib>                  // for EXIT_SUCCESS
#include <fstream>                  // for basic_ofstream<>::__filebuf_type
#include <iostream>                 // for operator<<, ostream, cout, basic_...
#include <memory>                   // for unique_ptr, make_unique, allocator
#include <stdexcept>                // for runtime_error
#include <string>                   // for char_traits, operator<<, string
#include <tuple>                    // for tuple_element<>::type
#include <type_traits>              // for add_const<>::type
#include <utility>                  // for move

#ifndef __LIBARCSTK_MATCH_HPP__
#include <arcstk/match.hpp>
#endif
#ifndef __LIBARCSTK_PARSE_HPP__
#include <arcstk/parse.hpp>
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>       // for ARCS_LOG_DEBUG, ARCS_LOG_ERROR
#endif

#ifndef __LIBARCSDEC_CALCULATORS_HPP__
#include <arcsdec/calculators.hpp>
#endif

#ifndef __ARCSTOOLS_CLIPARSE_HPP__
#include "cliparse.hpp"             // for CLITokens, __ARCSTOOLS_CLIPARSE_H...
#endif
#ifndef __ARCSTOOLS_CONFIG_HPP__
#include "config.hpp"               // for CallSyntaxException, Configurator
#endif
#ifndef __ARCSTOOLS_FORMAT_HPP__
#include "format.hpp"               // for ARId
#endif
#ifndef __ARCSTOOLS_OPTIONS_HPP__
#include "options.hpp"              // for Options, __ARCSTOOLS_OPTIONS_HPP__
#endif
#ifndef __ARCSTOOLS_PARSEHANDLERS_HPP__
#include "parsehandlers.hpp"        // for ContentHandler
#endif
#ifndef __ARCSTOOLS_TOOLS_CALC_HPP__
#include "tools_calc.hpp"           // for audiofile_layout
#endif
#ifndef __ARCSTOOLS_TOOLS_FS_HPP__
#include "tools_fs.hpp"             // for file_exists
#endif

namespace arcsapp
{

using arcstk::ARStreamParser;
using arcstk::ARFileParser;
using arcstk::ARId;
using arcstk::ARResponse;
using arcstk::ARStdinParser;
using arcstk::Checksums;
using arcstk::Logging;
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


const std::vector<std::pair<Option, uint32_t>>&
	ARVerifyConfigurator::do_supported_options() const
{
	const static std::vector<std::pair<Option, uint32_t>> local_options = {
		{{      "no-v1",    false, "FALSE",
			"do not verify ARCSv1" },
			ARVerifyOptions::NOV1 },
		{{      "no-v2",    false, "FALSE",
			"do not verify ARCSv2" },
			ARVerifyOptions::NOV2 },
		{{      "album",    false, "~",
			"abbreviates --first --last" },
			ARVerifyOptions::ALBUM },
		{{      "first",    false, "~",
			"treat first audio file as first track" },
			ARVerifyOptions::FIRST },
		{{      "last",     false, "~",
			"treat last audio file as last track" },
			ARVerifyOptions::LAST },
		{{ 'm', "metafile", true, "none",
			"specify metadata file (CUE) to use" },
			ARVerifyOptions::METAFILE },
		{{ 'r', "response", true, "none",
			"specify AccurateRip response file" },
			ARVerifyOptions::RESPONSEFILE },
		{{      "list-toc-formats",  false,   "FALSE",
			"list all supported file formats for TOC metadata" },
			ARVerifyOptions::LIST_TOC_FORMATS },
		{{      "list-audio-formats",  false, "FALSE",
			"list all supported audio codec/container formats" },
			ARVerifyOptions::LIST_AUDIO_FORMATS }
	};

	return local_options;
}


// ARVerifyApplication


ARResponse ARVerifyApplication::parse_response(const Options &options) const
{
	// Parse the AccurateRip response

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

	ARResponse response;
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

	return response;
}


std::unique_ptr<MatchResultPrinter> ARVerifyApplication::configure_format(
		const Options &options, const bool with_filenames) const
{
	const bool with_toc = !options.get(ARCalcOptions::METAFILE).empty();

	// show track + offset only when toc is requested
	// show filenames otherwise
	// show length in every case
	return std::make_unique<AlbumMatchTableFormat>(
				0, with_toc, with_toc, true, with_filenames);
}


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


std::string ARVerifyApplication::do_name() const
{
	return "verify";
}


std::string ARVerifyApplication::do_call_syntax() const
{
	return "[OPTIONS] <filename1> [ <filename2> ... ]";
}


std::unique_ptr<Configurator> ARVerifyApplication::create_configurator(
		int argc, char** argv) const
{
	return std::make_unique<ARVerifyConfigurator>(argc, argv);
}


int ARVerifyApplication::do_run(const Options &options)
{
	// Calculate the actual ARCSs from input files

	auto [ checksums, arid, toc ] = ARCalcApplication::calculate(options);

	if (checksums.size() == 0)
	{
		this->fatal_error("Calculation returned no checksums.");
	}

	// Parse reference ARCSs from AccurateRip

	auto response = parse_response(options);

	// Perform match

	std::unique_ptr<const Matcher> diff;

	const bool album_requested = !options.get(ARCalcOptions::METAFILE).empty();
	bool with_filenames = true;

	if (album_requested)
	{
		// With Offsets, ARId and Result

		if (!toc)
		{
			ARCS_LOG_ERROR << "Calculation returned no TOC.";
			// TODO throw;
		}

		if (arid.empty())
		{
			ARCS_LOG_ERROR << "Calculation returned no identifier.";
		}

		// Verify pairwise distinct audio files

		const auto& [ single_audio_file, pw_distinct ] =
			calc::audiofile_layout(*toc);

		if (not single_audio_file and not pw_distinct)
		{
			throw std::runtime_error("Images with audio files that contain"
				" some but not all tracks are currently unsupported");
		}

		with_filenames = not single_audio_file;

		diff = std::make_unique<AlbumMatcher>(checksums, arid, response);
	} else
	{
		// No Offsets => No ARId => No TOC

		diff = std::make_unique<TracksetMatcher>(checksums, response);

		if (Logging::instance().has_level(arcstk::LOGLEVEL::DEBUG))
		{
			log_matching_files(checksums, *diff->match(), 1, true);
		}
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

	// Print results

	auto filenames = toc
		? arcstk::toc::get_filenames(toc)
		: options.get_arguments();
	Match *match = const_cast<Match*>(diff->match()); // FIXME catastrophic

	auto format = configure_format(options, with_filenames);

	format->use(&checksums, std::move(filenames), std::move(response),
		std::move(match), diff->best_match(), diff->matches_v2(),
		toc.get(), std::move(arid));
	output(*format);

	return EXIT_SUCCESS;
}

} // namespace arcsapp

