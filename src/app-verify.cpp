#ifndef __ARCSTOOLS_APPARVERIFY_HPP__
#include "app-verify.hpp"
#endif

#include <algorithm>                // for replace
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
#ifndef __ARCSTOOLS_CONFIG_HPP__
#include "config.hpp"               // for CallSyntaxException, Configurator
#endif
#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#include "layouts.hpp"              // for ARIdLayout
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
using arcstk::Checksum;
using arcstk::Checksums;
using arcstk::Logging;
using arcstk::DefaultContentHandler;
using arcstk::DefaultErrorHandler;
using arcstk::Matcher;
using arcstk::AlbumMatcher;
using arcstk::ListMatcher;
using arcstk::TracksetMatcher;

using arcsdec::ARCSCalculator;


// VERIFY


constexpr OptionCode VERIFY::NOFIRST;
constexpr OptionCode VERIFY::NOLAST;
constexpr OptionCode VERIFY::NOALBUM;
constexpr OptionCode VERIFY::RESPONSEFILE;
constexpr OptionCode VERIFY::REFVALUES;
constexpr OptionCode VERIFY::PRINTALL;
constexpr OptionCode VERIFY::BOOLEAN;
constexpr OptionCode VERIFY::NOOUTPUT;


/**
 * \brief Return reference checksums from block \c block in order of appearance.
 *
 * \param[in] response The ARResponse to get a checksum block of
 * \param[in] block    Index of the block to read off
 *
 * \return Checksums in block \c block
 */
std::vector<Checksum> sums_in_block(const ARResponse response, const int block)
{
	auto sums = std::vector<Checksum>{};
	sums.reserve(response.tracks_per_block());

	for (const auto& triplet : response[block])
	{
		sums.push_back(Checksum { triplet.arcs() });
	}

	return sums;
}


// ARVerifyConfigurator


const std::vector<std::pair<Option, OptionCode>>&
	ARVerifyConfigurator::do_supported_options() const
{
	const static std::vector<std::pair<Option, OptionCode>> local_options =
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
			VERIFY::REFVALUES},

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
			VERIFY::PRINTID},
		{{      "print-url",   false, "FALSE",
			"Print the AccurateRip URL of the album" },
			VERIFY::PRINTURL},
		{{      "print-all-matches",   false, "FALSE",
			"Print verification results for all blocks" },
			VERIFY::PRINTALL},
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

	// Print ID only in case we have one from the reference data

	if (voptions->is_set(VERIFY::REFVALUES))
	{
		if (voptions->is_set(VERIFY::PRINTID))
		{
			ARCS_LOG_WARNING <<
				"Ignore option PRINTID since option REFVALUES is active "
				"and reference values do not provide an ID to print";
			voptions->unset(VERIFY::PRINTID);
		}
		if (voptions->is_set(VERIFY::PRINTURL))
		{
			ARCS_LOG_WARNING <<
				"Ignore option PRINTURL since option REFVALUES is active "
				"and reference values do not provide an URL to print";
			voptions->unset(VERIFY::PRINTURL);
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


std::unique_ptr<VerifyResultLayout> ARVerifyApplication::configure_layout(
		const Options &options, const bool with_filenames) const
{
	const bool with_toc = !options.value(VERIFY::METAFILE).empty();

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
		? options.value(CALC::COLDELIM)
		: " ";

	auto layout = std::make_unique<VerifyTableLayout>(
			not options.is_set(VERIFY::NOLABELS),
			prints_tracks, prints_offsets, prints_lengths, prints_filenames,
			coldelim);

	return layout;
}


std::tuple<ARResponse, std::vector<Checksum>>
	ARVerifyApplication::get_reference_checksums(const Options &options) const
{
	auto response = ARResponse {};
	auto refsums  = std::vector<Checksum> {};

	if (options.is_set(VERIFY::REFVALUES))
	{
		// use referene checksums passed on command line
		refsums = parse_refvalues(options);
	} else
	{
		// stdin or accuraterip .bin-file
		response = parse_response(options);
	}

	return std::make_tuple(response, refsums);
}


ARResponse ARVerifyApplication::parse_response(const Options &options) const
{
	// Parse the AccurateRip response

	std::unique_ptr<ARStreamParser> parser;

	std::string responsefile { options.value(VERIFY::RESPONSEFILE) };

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


std::vector<Checksum> ARVerifyApplication::parse_refvalues(
		const Options &options) const
{
	auto refvals = parse_refvalues_sequence(options.value(VERIFY::REFVALUES));

	// Log the parsing result

	std::ostringstream outlist;
	for (const auto& v : refvals) { outlist << v << " "; }
	ARCS_LOG_DEBUG << "Option --refvals was passed the following values: "
			<< outlist.str();

	return refvals;
}


std::vector<Checksum> ARVerifyApplication::parse_refvalues_sequence(
		const std::string &input) const
{
	if (input.empty())
	{
		return std::vector<Checksum>{};
	}

	const char delim = ',';

	auto in { input };
	std::replace(in.begin(), in.end(), delim, ' ');
	auto refvals = std::istringstream { in };
	auto refsum = uint32_t { 0 };

	std::vector<Checksum> refsums = {};
	{
		auto value = std::string {};

		int t = 1;
		while (refvals >> value)
		{
			refsum = std::stoul(value, 0, 16);

			ARCS_LOG_DEBUG << "Input reference sum " << value
				<< " for track/file " << std::setw(2) << t
				<< " is parsed as " << refsum;

			refsums.push_back(Checksum { refsum });
			++t;
		}
	}

	return refsums;
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
				ARCS_LOG_DEBUG << "Pos " << std::to_string(track)
					<< " matches track " << std::to_string(track + 1)
					<< " in block " << std::to_string(block);

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


std::unique_ptr<Configurator> ARVerifyApplication::create_configurator() const
{
	return std::make_unique<ARVerifyConfigurator>();
}


int ARVerifyApplication::run_calculation(const Options &options)
{
	// Parse reference ARCSs from AccurateRip

	auto reference_sums = this->get_reference_checksums(options);

	// Calculate the actual ARCSs from input files

	auto [ checksums, arid, toc ] = ARCalcApplication::calculate(
			options.value(VERIFY::METAFILE),
			options.arguments(),
			not options.is_set(VERIFY::NOFIRST),
			not options.is_set(VERIFY::NOLAST),
			{ arcstk::checksum::type::ARCS2 } /* force ARCSv1 + ARCSv2 */);

	if (checksums.size() == 0)
	{
		this->fatal_error("Calculation returned no checksums.");
	}

	// Perform match

	const bool album_requested = not options.is_set(VERIFY::NOALBUM);

	bool print_filenames = true;

	std::unique_ptr<const Matcher> diff;

	if (options.is_set(VERIFY::REFVALUES))
	{
		diff = std::make_unique<ListMatcher>(checksums,
				std::get<1>(reference_sums) /* refvals */);
	}

	if (not album_requested)
	{
		// No Offsets => No ARId => No TOC

		if (!diff) // No ListMatcher for some refvals previously set
		{
			diff = std::make_unique<TracksetMatcher>(checksums,
					std::get<0>(reference_sums) /* ARResponse */);
		}

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

		const auto& [ single_audio_file, pairwse_distinct_files, audiofiles ] =
			calc::audiofile_layout(*toc);

		if (not single_audio_file and not pairwse_distinct_files)
		{
			throw std::runtime_error("Images with audio files that contain"
				" some but not all tracks are currently unsupported");
		}

		print_filenames = not single_audio_file;

		if (!diff) // No ListMatcher for some refvals previously set
		{
			diff = std::make_unique<AlbumMatcher>(checksums, arid,
				std::get<0>(reference_sums) /* ARResponse */);
		}
	} // if album_requested


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

	print_result(options, checksums, reference_sums, toc.get(), arid, *diff,
			print_filenames);

	return options.is_set(VERIFY::BOOLEAN)
		? diff->best_difference()
		: EXIT_SUCCESS;
}


void ARVerifyApplication::print_result(const Options &options,
			const Checksums &actual_sums,
			const std::tuple<ARResponse, std::vector<Checksum>> &reference_sums,
			const TOC *toc, const ARId &arid, const arcstk::Matcher &diff,
			const bool print_filenames) const
{
	const auto filenames = not options.no_arguments()
		? options.arguments()
		: (toc ? arcstk::toc::get_filenames(*toc) : std::vector<std::string>{});

	auto dont_overwrite = bool { true };

	if (options.is_set(VERIFY::PRINTID) or options.is_set(VERIFY::PRINTURL))
	{
		// Do we have an ARId for "Theirs"? (Otherwise, forget about the actual
		// ARId computed locally)
		if (auto response = &std::get<0>(reference_sums); response)
		{
			const std::unique_ptr<ARIdLayout> layout =
				std::make_unique<ARIdTableLayout>(
					options.is_set(CALC::PRINTID),
					options.is_set(CALC::PRINTURL),
					false, /* no filenames */
					false, /* no tracks */
					false, /* no id 1 */
					false, /* no id 2 */
					false /* no cddb id */
				);

			auto r_arid = response->at(diff.best_match()).id();
			auto result = layout->format(r_arid, std::string{});

			output(result, options.value(OPTION::OUTFILE)); // overwrites
			dont_overwrite = false;
		}
	}

	const auto layout = configure_layout(options, print_filenames);

	if (options.is_set(VERIFY::PRINTALL))
	{
		const bool print_v1_and_v2 = true;

		if (options.is_set(VERIFY::REFVALUES)) // Use refvals
		{
			const int only_block = 0;

			auto result = layout->format(actual_sums, filenames,
					std::get<1>(reference_sums) /* refvals */,
					diff.match(), only_block, print_v1_and_v2, toc, arid);

			output(result, options.value(OPTION::OUTFILE), dont_overwrite);

		} else // Use ARResponse
		{
			auto& response = std::get<0>(reference_sums);

			int curr_block = 0; // convert block counter to int
			for (ARResponse::size_type b = 0; b < response.size(); ++b)
			{
				curr_block = b;
				auto block_sums = sums_in_block(response, curr_block);

				auto result = layout->format(actual_sums, filenames,
						block_sums, diff.match(), curr_block,
						print_v1_and_v2, toc, arid);

				output(result, options.value(OPTION::OUTFILE), dont_overwrite);
			}
		}
	} else // print only best match
	{
		const auto best_block       = diff.best_match();
		const auto matching_version = diff.matches_v2();

		if (options.is_set(VERIFY::REFVALUES)) // Use refvals
		{
			auto result = layout->format(actual_sums, filenames,
					std::get<1>(reference_sums) /* refvals */,
					diff.match(), best_block, matching_version, toc, arid);

			output(result, options.value(OPTION::OUTFILE), dont_overwrite);
		} else // Use ARResponse
		{
			const auto ref_sums = sums_in_block(std::get<0>(reference_sums),
					diff.best_match());

			auto result = layout->format(actual_sums, filenames, ref_sums,
					diff.match(), best_block, matching_version, toc, arid);

			output(result, options.value(OPTION::OUTFILE), dont_overwrite);
			// &ref_sums must be in scope
		}
	}
}


int ARVerifyApplication::do_run(const Options &options)
{
	if (ARCalcConfiguratorBase::calculation_requested(options))
	{
		return this->run_calculation(options);
	}

	// If only info options are present, handle info request

	auto dont_overwrite = bool { true };

	if (options.is_set(VERIFY::LIST_TOC_FORMATS))
	{
		output(SupportedFormats::toc(), options.value(OPTION::OUTFILE));
		dont_overwrite = false;
	}

	if (options.is_set(VERIFY::LIST_AUDIO_FORMATS))
	{
		output(SupportedFormats::audio(), options.value(OPTION::OUTFILE),
				dont_overwrite);
	}

	return EXIT_SUCCESS;
}

} // namespace arcsapp
