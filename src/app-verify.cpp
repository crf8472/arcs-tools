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


// VERIFY


constexpr OptionValue VERIFY::NOFIRST;
constexpr OptionValue VERIFY::NOLAST;
constexpr OptionValue VERIFY::NOALBUM;
constexpr OptionValue VERIFY::RESPONSEFILE;
constexpr OptionValue VERIFY::REFVALUES;
constexpr OptionValue VERIFY::PRINTALL;
constexpr OptionValue VERIFY::BOOLEAN;
constexpr OptionValue VERIFY::NOOUTPUT;


// ARVerifyConfigurator


const std::vector<std::pair<Option, OptionValue>>&
	ARVerifyConfigurator::do_supported_options() const
{
	const static std::vector<std::pair<Option, OptionValue>> local_options =
	{
		// calculation input options

		{{      "no-first",    false, "~",
			"Do not treat first audio file as first track" },
			VERIFY::NOFIRST },
		{{      "no-last",     false, "~",
			"Do not treat last audio file as last track" },
			VERIFY::NOLAST },
		{{      "no-album",    false, "~",
			"Abbreviates --no-first --no-last" },
			VERIFY::NOALBUM },
		{{ 'm', "metafile", true, "none",
			"Specify metadata file (TOC) to use" },
			VERIFY::METAFILE },
		{{ 'r', "response", true, "none",
			"Specify AccurateRip response file" },
			VERIFY::RESPONSEFILE },
		{{      "refvalues", true, "none",
			"Specify AccurateRip reference values (as hex value list)" },
			VERIFY::REFVALUES}, // TODO

		// calculation output options

		{{      "no-track-nos",  false, "FALSE",
			"Do not print track numbers" },
			VERIFY::NOTRACKS},
		{{      "no-filenames",    false, "FALSE",
			"Do not print the filenames" },
			VERIFY::NOFILENAMES},
		{{      "no-offsets",    false, "FALSE",
			"Do not print track offsets" },
			VERIFY::NOOFFSETS},
		{{      "no-lengths",    false, "FALSE",
			"Do not print track lengths" },
			VERIFY::NOLENGTHS},
		{{      "no-labels",    false, "FALSE",
			"Do not print column or row labels" },
			VERIFY::NOLABELS},
		{{      "col-delim",    true, " ",
			"Specify column delimiter" },
			VERIFY::COLDELIM},
		{{      "print-id",    false, "FALSE",
			"Print the AccurateRip Id of the album" },
			VERIFY::PRINTID}, // TODO
		{{      "print-url",   false, "FALSE",
			"Print the AccurateRip URL of the album" },
			VERIFY::PRINTURL}, // TODO
		{{      "print-all-matches",   false, "FALSE",
			"Print verification results for all blocks" },
			VERIFY::PRINTALL}, // TODO
		{{ 'b', "boolean",   false, "FALSE",
			"Return number of differing tracks in best match" },
			VERIFY::BOOLEAN},
		{{ 'n', "no-output", false, "FALSE",
			"Do not print the result (implies --boolean)" },
			VERIFY::NOOUTPUT},

		// info output options

		{{      "list-toc-formats",  false,   "FALSE",
			"List all supported file formats for TOC metadata" },
			VERIFY::LIST_TOC_FORMATS },
		{{      "list-audio-formats",  false, "FALSE",
			"List all supported audio codec/container formats" },
			VERIFY::LIST_AUDIO_FORMATS }
	};

	return local_options;
}


std::unique_ptr<Options> ARVerifyConfigurator::do_configure_options(
		std::unique_ptr<Options> options)
{
	auto voptions = configure_calcbase_options(std::move(options));

	// Album mode

	if (voptions->is_set(VERIFY::NOALBUM))
	{
		ARCS_LOG(DEBUG1) << "Activate option NOFIRST due to NOALBUM";
		voptions->set(VERIFY::NOFIRST);

		ARCS_LOG(DEBUG1) << "Activate option NOLAST due to NOALBUM";
		voptions->set(VERIFY::NOLAST);
	} else
	{
		if(voptions->is_set(VERIFY::NOFIRST) and
				voptions->is_set(VERIFY::NOLAST))
		{
			ARCS_LOG(DEBUG1) <<
				"Activate option NOALBUM due to NOFIRST and NOLAST";
			voptions->set(VERIFY::NOALBUM);
		}
	}

	// NOOUTPUT implies BOOLEAN

	if (voptions->is_set(VERIFY::NOOUTPUT))
	{
		voptions->set(VERIFY::BOOLEAN);
	}

	return voptions;
}


// ARVerifyApplication


ARResponse ARVerifyApplication::parse_response(const Options &options) const
{
	// Parse the AccurateRip response

	std::string responsefile(options.get(VERIFY::RESPONSEFILE));

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
	const bool with_toc = !options.get(VERIFY::METAFILE).empty();

	// Print track number if they are not forbidden and a TOC is present
	const bool prints_tracks = options.is_set(VERIFY::NOTRACKS)
		? false
		: with_toc;

	// Print offsets if they are not forbidden and a TOC is present
	const bool prints_offsets = options.is_set(VERIFY::NOOFFSETS)
		? false
		: with_toc;

	// Print lengths if they are not forbidden
	const bool prints_lengths = not options.is_set(VERIFY::NOLENGTHS);

	// Print filenames if they are not forbidden and explicitly requested
	const bool prints_filenames = options.is_set(VERIFY::NOFILENAMES)
		? false
		: with_filenames;

	// Set column delimiter
	const std::string coldelim = options.is_set(CALC::COLDELIM)
		? options.get(CALC::COLDELIM)
		: " ";

	auto format = std::make_unique<AlbumMatchTableFormat>(
			not options.is_set(VERIFY::NOLABELS),
			prints_tracks, prints_offsets, prints_lengths, prints_filenames,
			coldelim);

	return format;
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


int ARVerifyApplication::run_calculation(const Options &options)
{
	// Calculate the actual ARCSs from input files

	auto [ checksums, arid, toc ] = ARCalcApplication::calculate(
			options.get(CALC::METAFILE),
			options.get_arguments(),
			not options.is_set(VERIFY::NOFIRST),
			not options.is_set(VERIFY::NOLAST),
			{ arcstk::checksum::type::ARCS2 } /* force ARCSv1 + ARCSv2 */);

	if (checksums.size() == 0)
	{
		this->fatal_error("Calculation returned no checksums.");
	}

	// Parse reference ARCSs from AccurateRip

	auto response = parse_response(options);

	// Perform match

	std::unique_ptr<const Matcher> diff;

	const bool album_requested = not options.is_set(VERIFY::NOALBUM);

	bool print_filenames = true;

	if (not album_requested)
	{
		// No Offsets => No ARId => No TOC

		diff = std::make_unique<TracksetMatcher>(checksums, response);

		if (Logging::instance().has_level(arcstk::LOGLEVEL::DEBUG))
		{
			log_matching_files(checksums, *diff->match(), 1, true);
		}
	} else
	{
		// Do verification for Offsets, ARId and TOC

		if (!toc)
		{
			this->fatal_error(
					"Album requested, but calculation returned no TOC.");
		}

		if (arid.empty())
		{
			this->fatal_error(
					"Album requested, but calculation returned an empty ARId.");
		}

		// Verify pairwise distinct audio files

		const auto& [ single_audio_file, pairwse_distinct_files ] =
			calc::audiofile_layout(*toc);

		if (not single_audio_file and not pairwse_distinct_files)
		{
			throw std::runtime_error("Images with audio files that contain"
				" some but not all tracks are currently unsupported");
		}

		print_filenames = not single_audio_file;

		diff = std::make_unique<AlbumMatcher>(checksums, arid, response);
	}

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

	if (options.is_set(VERIFY::NOOUTPUT)) // implies BOOLEAN
	{
		return diff->best_difference(); // 0 on accurate match, else > 0
	}

	// Print match results

	auto filenames = not options.no_arguments()
		? options.get_arguments()
		: (toc ? arcstk::toc::get_filenames(toc) : std::vector<std::string>{} );

	Match *match = const_cast<Match*>(diff->match()); // FIXME catastrophic

	auto format = configure_format(options, print_filenames);

	format->use(&checksums, std::move(filenames), std::move(response),
		std::move(match), diff->best_match(), diff->matches_v2(),
		toc.get(), std::move(arid));

	output(*format);

	return options.is_set(VERIFY::BOOLEAN)
		? diff->best_difference()
		: EXIT_SUCCESS;
}


int ARVerifyApplication::do_run(const Options &options)
{
	if (ARCalcConfiguratorBase::calculation_requested(options))
	{
		return this->run_calculation(options);
	}

	// If only info options are present, handle info request

	if (options.is_set(VERIFY::LIST_TOC_FORMATS))
	{
		output(SupportedFormats::toc(), options.output());
	}

	if (options.is_set(VERIFY::LIST_AUDIO_FORMATS))
	{
		output(SupportedFormats::audio(), options.output());
	}

	return EXIT_SUCCESS;
}

} // namespace arcsapp

