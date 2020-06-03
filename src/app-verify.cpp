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

#ifndef __ARCSTOOLS_APPREGISTRY_HPP__
#include "appregistry.hpp"          // for RegisterApplicationType
#endif
#ifndef __ARCSTOOLS_CLITOKENS_HPP__
#include "clitokens.hpp"            // for CLITokens, __ARCSTOOLS_CLITOKENS_H...
#endif
#ifndef __ARCSTOOLS_CONFIG_HPP__
#include "config.hpp"               // for CallSyntaxException, Configurator
#endif
#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#include "layouts.hpp"               // for ARId
#endif
#ifndef __ARCSTOOLS_OPTIONS_HPP__
#include "options.hpp"              // for Options, __ARCSTOOLS_OPTIONS_HPP__
#endif
#ifndef __ARCSTOOLS_TOOLS_PARSE_HPP__
#include "tools-parse.hpp"          // for ContentHandler
#endif
#ifndef __ARCSTOOLS_TOOLS_CALC_HPP__
#include "tools-calc.hpp"           // for audiofile_layout
#endif
#ifndef __ARCSTOOLS_TOOLS_FS_HPP__
#include "tools-fs.hpp"             // for file_exists
#endif
#ifndef __ARCSTOOLS_TOOLS_INFO_HPP__
#include "tools-info.hpp"           // for SupportedFormats
#endif

namespace arcsapp
{

namespace registered
{
const auto verify = RegisterApplicationType<ARVerifyApplication>("verify");
}

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


constexpr OptionValue ARVerifyOptions::RESPONSEFILE;
constexpr OptionValue ARVerifyOptions::REFVALUES;
constexpr OptionValue ARVerifyOptions::BOOLEAN;
constexpr OptionValue ARVerifyOptions::PRINTALL;


// ARVerifyConfigurator


ARVerifyConfigurator::ARVerifyConfigurator(int argc, char** argv)
	: ARCalcConfigurator(argc, argv)
{
	// empty
}


const std::vector<std::pair<Option, OptionValue>>&
	ARVerifyConfigurator::do_supported_options() const
{
	const static std::vector<std::pair<Option, OptionValue>> local_options =
	{
		// calculation input options

		{{      "first",    false, "~",
			"Treat first audio file as first track" },
			ARVerifyOptions::FIRST },
		{{      "last",     false, "~",
			"Treat last audio file as last track" },
			ARVerifyOptions::LAST },
		{{      "album",    false, "~",
			"Abbreviates --first --last" },
			ARVerifyOptions::ALBUM },
		{{ 'm', "metafile", true, "none",
			"Specify metadata file (TOC) to use" },
			ARVerifyOptions::METAFILE },
		{{ 'r', "response", true, "none",
			"Specify AccurateRip response file" },
			ARVerifyOptions::RESPONSEFILE },
		{{      "refvalues", true, "none",
			"Specify AccurateRip reference values (as hex value list)" },
			ARVerifyOptions::REFVALUES},

		// calculation output options

		{{      "no-v1",    false, "FALSE",
			"Do not provide ARCSv1" },
			ARVerifyOptions::NOV1 },
		{{      "no-v2",    false, "FALSE",
			"Do not provide ARCSv2" },
			ARVerifyOptions::NOV2 },
		{{      "no-track-nos",  false, "FALSE",
			"Do not print track numbers" },
			ARVerifyOptions::NOTRACKS},
		{{      "no-offsets",    false, "FALSE",
			"Do not print track offsets" },
			ARVerifyOptions::NOOFFSETS},
		{{      "no-lengths",    false, "FALSE",
			"Do not print track lengths" },
			ARVerifyOptions::NOLENGTHS},
		{{      "no-col-headers",    false, "FALSE",
			"Do not print column headers" },
			ARVerifyOptions::NOCOLHEADERS},
		{{      "print-sums-only",    false, "FALSE",
			"Print only the checksums" },
			ARVerifyOptions::SUMSONLY},
		{{      "tracks-as-cols",    false, "FALSE",
			"Print result with tracks as columns" },
			ARVerifyOptions::TRACKSASCOLS},
		{{      "col-delim",    true, " ",
			"Specify column delimiter" },
			ARVerifyOptions::COLDELIM},
		{{      "print-id",    false, "FALSE",
			"Print the AccurateRip Id of the album" },
			ARVerifyOptions::PRINTID},
		{{      "print-url",   false, "FALSE",
			"Print the AccurateRip URL of the album" },
			ARVerifyOptions::PRINTURL},
		{{      "print-all-matches",   false, "FALSE",
			"Print verification results for all blocks" },
			ARVerifyOptions::PRINTALL},
		{{ 'b', "boolean",   false, "FALSE",
			"Return number of differing tracks in best match" },
			ARVerifyOptions::BOOLEAN},
		{{ 'n', "no-output", false, "FALSE",
			"Do not print the result (implies --boolean)" },
			ARVerifyOptions::NOOUTPUT},

		// info output options

		{{      "list-toc-formats",  false,   "FALSE",
			"List all supported file formats for TOC metadata" },
			ARVerifyOptions::LIST_TOC_FORMATS },
		{{      "list-audio-formats",  false, "FALSE",
			"List all supported audio codec/container formats" },
			ARVerifyOptions::LIST_AUDIO_FORMATS }
	};

	return local_options;
}


std::unique_ptr<Options> ARVerifyConfigurator::do_configure_options(
		std::unique_ptr<Options> options)
{
	return this->configure_calc_options(std::move(options));
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


int ARVerifyApplication::run_info(const Options &options)
{
	if (options.is_set(ARCalcOptions::LIST_TOC_FORMATS))
	{
		output(SupportedFormats::toc(), options.output());
	}

	if (options.is_set(ARCalcOptions::LIST_AUDIO_FORMATS))
	{
		output(SupportedFormats::audio(), options.output());
	}

	return EXIT_SUCCESS;
}


int ARVerifyApplication::run_calculation(const Options &options)
{
	// Calculate the actual ARCSs from input files (force ARCSv1 + ARCSv2)

	auto [ checksums, arid, toc ] = ARCalcApplication::calculate(options,
			{ arcstk::checksum::type::ARCS2 });

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


int ARVerifyApplication::do_run(const Options &options)
{
	// If only info options are present, handle info request

	if (options.is_set(ARCalcOptions::LIST_TOC_FORMATS)
		or options.is_set(ARCalcOptions::LIST_AUDIO_FORMATS))
	{
		return this->run_info(options);
	}

	return this->run_calculation(options);
}

} // namespace arcsapp

