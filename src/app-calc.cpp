#ifndef __ARCSTOOLS_ARCALC_HPP__
#include "app-calc.hpp"
#endif

#include <array>                    // for array
#include <cstdlib>                  // for EXIT_SUCCESS
#include <fstream>                  // for operator<<, ostream, ofstream
#include <functional>               // for _Bind, bind, _1
#include <iostream>                 // for cout
#include <memory>                   // for unique_ptr, allocator, make_unique
#include <string>                   // for string, char_traits, operator<<
#include <tuple>                    // for tuple, tuple_element<>::type
#include <utility>                  // for move
#include <vector>                   // for vector

#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif
#ifndef __LIBARCSTK_MATCH_HPP__
#include <arcstk/match.hpp>         // for ARId, Checksums
#endif

#ifndef __LIBARCSDEC_AUDIOREADER_HPP__
#include <arcsdec/audioreader.hpp>  // for AudioReaderSelection
#endif
#ifndef __LIBARCSDEC_CALCULATORS_HPP__
#include <arcsdec/calculators.hpp>  // for ARCSCalculator, TOCParser
#endif
#ifndef __LIBARCSDEC_METAPARSER_HPP__
#include <arcsdec/metaparser.hpp>   // for MetadataParserSelection
#endif

#ifndef __ARCSTOOLS_APPREGISTRY_HPP__
#include "appregistry.hpp"                 // for RegisterApplicationType
#endif
#ifndef __ARCSTOOLS_CLITOKENS_HPP__
#include "clitokens.hpp"            // for CLITokens, __ARCSTOOLS_CLITOKENS_H...
#endif
#ifndef __ARCSTOOLS_CONFIG_HPP__
#include "config.hpp"
#endif
#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#include "layouts.hpp"               // for operator<<
#endif
#ifndef __ARCSTOOLS_PRINTERS_HPP__
#include "printers.hpp"
#endif
#ifndef __ARCSTOOLS_TOOLS_CALC_HPP__
#include "tools-calc.hpp"           // for ARCSMultifileAlbumCalculator
#endif
#ifndef __ARCSTOOLS_TOOLS_FS_HPP__
#include "tools-fs.hpp"             // for path
#endif
#ifndef __ARCSTOOLS_TOOLS_INFO_HPP__
#include "tools-info.hpp"           // for SupportedFormats
#endif

namespace arcsapp
{

namespace registered
{
const auto calc = RegisterApplicationType<ARCalcApplication>("calc");
}

class Options;

using arcstk::ARId;
using arcstk::Checksums;
using arcstk::TOC;
using arcstk::make_empty_arid;

using arcsdec::ARCSCalculator;
using arcsdec::TOCParser;


// ARCalcOptions


constexpr OptionValue ARCalcOptions::NOV1;
constexpr OptionValue ARCalcOptions::NOV2;
constexpr OptionValue ARCalcOptions::ALBUM;
constexpr OptionValue ARCalcOptions::FIRST;
constexpr OptionValue ARCalcOptions::LAST;
constexpr OptionValue ARCalcOptions::METAFILE;
constexpr OptionValue ARCalcOptions::LIST_TOC_FORMATS;
constexpr OptionValue ARCalcOptions::LIST_AUDIO_FORMATS;
constexpr OptionValue ARCalcOptions::METAFILEPATH;


// ARCalcConfigurator


ARCalcConfigurator::ARCalcConfigurator(int argc, char** argv)
	: Configurator(argc, argv)
{
	// empty
}


ARCalcConfigurator::~ARCalcConfigurator() noexcept
= default;


bool ARCalcConfigurator::calculation_requested(const Options &options) const
{
	return options.is_set(ARCalcOptions::METAFILE)
		or not options.no_arguments();
}


std::unique_ptr<Options> ARCalcConfigurator::configure_calc_options(
		std::unique_ptr<Options> options) const
{
	//const auto calculation_request = bool { calculation_requested(options) };

	if (not calculation_requested(*options))
	{
		return options; // Only no-calc options present
	}

	if (options->is_set(ARCalcOptions::LIST_TOC_FORMATS))
	{
		ARCS_LOG_WARNING << "Option LIST_TOC_FORMATS is ignored";
		options->unset(ARCalcOptions::LIST_TOC_FORMATS);
	}

	if (options->is_set(ARCalcOptions::LIST_AUDIO_FORMATS))
	{
		ARCS_LOG_WARNING << "Option LIST_AUDIO_FORMATS is ignored";
		options->unset(ARCalcOptions::LIST_AUDIO_FORMATS);
	}

	// Metafile: get path + activate album mode

	if (options->is_set(ARCalcOptions::METAFILE))
	{
		auto metafilename(options->get(ARCalcOptions::METAFILE));

		if (metafilename.empty())
		{
			// no metadata file specified ...

			 // TODO Better: check for some "ARCalcOptions::AUDIOFILES" list
			if (options->no_arguments())
			{
				ARCS_LOG_ERROR << "No metafile and no audiofile specified";
				return std::make_unique<Options>();
			}
		}

		// Provide Path of the Metafile as Searchpath

		options->put(ARCalcOptions::METAFILEPATH,
			file::path(options->get(ARCalcOptions::METAFILE)));

		// Activate album mode

		if (options->is_set(ARCalcOptions::FIRST))
		{
			ARCS_LOG_INFO <<
				"Option FIRST is redundant when metafile is passed";
		} else
		{
			ARCS_LOG(DEBUG1) << "Activate option FIRST due to METAFILE";
			options->set(ARCalcOptions::FIRST);
		}

		if (options->is_set(ARCalcOptions::LAST))
		{
			ARCS_LOG_INFO <<
				"Option LAST is redundant when metafile is passed";
		} else
		{
			ARCS_LOG(DEBUG1) << "Activate option LAST due to METAFILE";
			options->set(ARCalcOptions::LAST);
		}

		ARCS_LOG(DEBUG1) << "Activate option ALBUM due to METAFILE";
		options->set(ARCalcOptions::ALBUM);
	}

	// Album or not

	if (options->is_set(ARCalcOptions::ALBUM))
	{
		if (not options->is_set(ARCalcOptions::FIRST))
		{
			ARCS_LOG(DEBUG1) << "Activate option FIRST due to ALBUM";

			options->set(ARCalcOptions::FIRST);
		} else
		{
			ARCS_LOG_INFO <<
				"Option FIRST is redundant when ALBUM is passed";
		}

		if (not options->is_set(ARCalcOptions::LAST))
		{
			ARCS_LOG(DEBUG1) << "Activate option LAST due to ALBUM";

			options->set(ARCalcOptions::LAST);
		} else
		{
			ARCS_LOG_INFO <<
				"Option LAST is redundant when ALBUM is passed";
		}
	} else
	{
		if (options->is_set(ARCalcOptions::FIRST)
			and options->is_set(ARCalcOptions::LAST))
		{
			ARCS_LOG(DEBUG1) << "Activate option ALBUM due to FIRST and LAST";
			options->set(ARCalcOptions::ALBUM);
		}
	}

	// Printing options

	if (options->is_set(ARCalcOptions::SUMSONLY))
	{
		options->set(ARCalcOptions::NOTRACKS);
		options->set(ARCalcOptions::NOFILENAMES);
		options->set(ARCalcOptions::NOOFFSETS);
		options->set(ARCalcOptions::NOLENGTHS);
		options->set(ARCalcOptions::NOLABELS); // Multiple Checksum types?
	}

	return options;
}


const std::vector<std::pair<Option, OptionValue>>&
	ARCalcConfigurator::do_supported_options() const
{
	const static std::vector<std::pair<Option, OptionValue>> local_options =
	{
		// calculation input options

		{{      "first",  false, "~", "Treat first audio file as first track" },
			ARCalcOptions::FIRST },
		{{      "last",   false, "~", "Treat last audio file as last track" },
			ARCalcOptions::LAST },
		{{      "album",  false, "~", "Abbreviates --first --last" },
			ARCalcOptions::ALBUM },
		{{ 'm', "metafile", true, "none", "Specify toc metadata file to use" },
			ARCalcOptions::METAFILE },

		// calculation output options

		{{  "no-v1", false, "FALSE", "Do not provide ARCSv1" },
			ARCalcOptions::NOV1 },
		{{  "no-v2", false, "FALSE", "Do not provide ARCSv2" },
			ARCalcOptions::NOV2 },
		{{  "no-track-nos", false, "FALSE", "Do not print track numbers" },
			ARCalcOptions::NOTRACKS},
		{{  "no-filenames", false, "FALSE", "Do not print the filenames" },
			ARCalcOptions::NOFILENAMES},
		{{  "no-offsets", false, "FALSE", "Do not print track offsets" },
			ARCalcOptions::NOOFFSETS},
		{{  "no-lengths", false, "FALSE", "Do not print track lengths" },
			ARCalcOptions::NOLENGTHS},
		{{  "no-labels", false, "FALSE", "Do not print column or row labels" },
			ARCalcOptions::NOLABELS},
		{{  "print-sums-only", false, "FALSE", "Print only checksums" },
			ARCalcOptions::SUMSONLY},
		{{  "tracks-as-cols", false, "FALSE", "Print tracks as columns" },
			ARCalcOptions::TRACKSASCOLS},
		{{  "col-delim", true, " ", "Specify column delimiter" },
			ARCalcOptions::COLDELIM},
		{{  "print-id", false, "FALSE", "Print AccurateRip Id of the album" },
			ARCalcOptions::PRINTID},
		{{  "print-url", false, "FALSE", "Print AccurateRip URL of the album" },
			ARCalcOptions::PRINTURL},

		// info output options

		{{  "list-toc-formats", false, "FALSE",
				"List all supported file formats for TOC metadata" },
			ARCalcOptions::LIST_TOC_FORMATS },
		{{  "list-audio-formats", false, "FALSE",
				"List all supported audio codec/container formats" },
			ARCalcOptions::LIST_AUDIO_FORMATS }
	};

	return local_options;
}


int ARCalcConfigurator::do_parse_arguments(CLITokens& cli, Options &options)
		const
{
	// Respect multiple arguments

	if (auto args = this->arguments(cli); !args.empty())
	{
		for (const auto& arg : args)
		{
			options.append(arg);
		}

		return static_cast<int>(args.size());
	}

	return 0;
}


std::unique_ptr<Options> ARCalcConfigurator::do_configure_options(
		std::unique_ptr<Options> options)
{
	return this->configure_calc_options(std::move(options));
}


// ARCalcApplication


std::tuple<Checksums, ARId, std::unique_ptr<TOC>> ARCalcApplication::calculate(
		const Options &options, const std::set<arcstk::checksum::type> &types)
{
	using ChecksumType = arcstk::checksum::type;

	// XXX Determine whether to request ARCS2+1 or ARCS1-only
	ChecksumType checksum_type = types.find(ChecksumType::ARCS2) != types.end()
		? ChecksumType::ARCS2
		: ChecksumType::ARCS1;

	std::vector<std::string> audiofilenames = options.get_arguments();
	std::string metafilename = options.get(ARCalcOptions::METAFILE);

	if (metafilename.empty())
	{
		// No Offsets => No ARId => No TOC
		// => No Album information, but may be requested as album by options
		// (since we have no offsets, we cannot offer to compute the ARId)

		ARCSCalculator c { checksum_type };

		// Checksums for a list of files (no tracks known)
		auto checksums = c.calculate(audiofilenames,
				options.is_set(ARCalcOptions::FIRST),
				options.is_set(ARCalcOptions::LAST));

		return std::tuple<Checksums, ARId, std::unique_ptr<TOC>>(
				checksums, *make_empty_arid(), nullptr);

	} else
	{
		// With Offsets, ARId and Result
		// => Album

		const auto audiofilenames = options.get_arguments();
		auto metafilepath         = options.get(ARCalcOptions::METAFILEPATH);

		calc::ARCSMultifileAlbumCalculator c { checksum_type };

		// TODO Resolve the audiofilenames HERE instead to do this within
		// the calculator class. Because the existence and validity
		// checks are done on THIS level, not in the delegate.
		// Create a class that concatenates audiofilenames from TOC to a
		// searchpath AND check for existence of the resulting filename.

		// TODO And sanitize the METAFILEPATH if necessery, it has to end
		// with a file separator

		auto [ checksums, arid, toc ] = audiofilenames.empty()
			? c.calculate(metafilename, metafilepath)
			: c.calculate(audiofilenames, metafilename);

		return std::tuple<Checksums, ARId, std::unique_ptr<TOC>>(
				checksums, arid, std::move(toc));
	}
}


std::unique_ptr<ChecksumsResultPrinter> ARCalcApplication::configure_format(
		const Options &options) const
{
	const bool has_toc = !options.get(ARCalcOptions::METAFILE).empty();

	// Print track number if they are not forbidden and a TOC is present
	const bool prints_tracks = options.is_set(ARCalcOptions::NOTRACKS)
		? false
		: has_toc;

	// Print offsets if they are not forbidden and a TOC is present
	const bool prints_offsets = options.is_set(ARCalcOptions::NOOFFSETS)
		? false
		: has_toc;

	// Print lengths if they are not forbidden
	const bool prints_lengths = not options.is_set(ARCalcOptions::NOLENGTHS);

	// Print filenames if they are not forbidden and explicitly requested
	const bool prints_filenames = options.is_set(ARCalcOptions::NOFILENAMES)
		? false
		: not has_toc;

	// Set column delimiter
	const std::string coldelim = options.is_set(ARCalcOptions::COLDELIM)
		? options.get(ARCalcOptions::COLDELIM)
		: " ";

	// Decide which implementation

	std::unique_ptr<ChecksumsResultPrinter> format;

	if (options.is_set(ARCalcOptions::TRACKSASCOLS))
	{
		format = std::make_unique<AlbumTracksTableFormat>(
			not options.is_set(ARCalcOptions::NOLABELS),
			prints_tracks, prints_offsets, prints_lengths, prints_filenames,
			coldelim);
	} else
	{
		format = std::make_unique<AlbumChecksumsTableFormat>(
			not options.is_set(ARCalcOptions::NOLABELS),
			prints_tracks, prints_offsets, prints_lengths, prints_filenames,
			coldelim);
	}

	return format;
}


std::set<arcstk::checksum::type> ARCalcApplication::requested_types(
		const Options &options) const
{
	// Select the checksum::type(s) to print

	std::set<arcstk::checksum::type> types = {};

	if (not options.is_set(ARCalcOptions::NOV1))
	{
		types.insert(arcstk::checksum::type::ARCS1);
	}
	if (not options.is_set(ARCalcOptions::NOV2))
	{
		types.insert(arcstk::checksum::type::ARCS2);
	}

	return types;
}


int ARCalcApplication::run_info(const Options &options)
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


int ARCalcApplication::run_calculation(const Options &options)
{
	// Determine the explicitly requested types

	auto requested_types = this->requested_types(options);

	if (requested_types.empty()) // No types requested? No calculation required!
	{
		ARCS_LOG_WARNING << "No checksum types requested. Done.";

		return EXIT_SUCCESS;
	}

	// Let calculate() determine the types to _calculate_, which are allowed
	// to differ from the explicitly requested types (since e.g. ARCS1 is
	// a byproduct of ARCS2 and the type-to-calculate ARCS2 hence represents
	// both the types-requested ARCS1 as well as ARCS2).

	auto [ checksums, arid, toc ] = this->calculate(options, requested_types);

	if (checksums.size() == 0)
	{
		this->fatal_error("Calculation returned no checksums");
	}

	// A hack: for printing, remove all types from the result that not
	// have been requested explicitly (implements e.g. --no-v1)

	for (auto& track : checksums)
	{
		for (const auto& type : track.types())
		{
			if (requested_types.find(type) == requested_types.end())
			{
				track.erase(type);
			}
		}
	}

	// Print formatted results to output stream

	auto format = configure_format(options);

	auto filenames = toc
		? arcstk::toc::get_filenames(toc)
		: options.get_arguments();

	format->use(&checksums, std::move(filenames), toc.get(), std::move(arid),
			options.is_set(ARCalcOptions::ALBUM));
	output(*format);

	return EXIT_SUCCESS;
}


std::string ARCalcApplication::do_name() const
{
	return "calc";
}


std::string ARCalcApplication::do_call_syntax() const
{
	return "[OPTIONS] <filename1>, <filename2>, ...";
}


std::unique_ptr<Configurator> ARCalcApplication::create_configurator(
		int argc, char** argv) const
{
	return std::make_unique<ARCalcConfigurator>(argc, argv);
}


int ARCalcApplication::do_run(const Options &options)
{
	// If only info options are present, handle info request

	if (options.is_set(ARCalcOptions::LIST_TOC_FORMATS)
		or options.is_set(ARCalcOptions::LIST_AUDIO_FORMATS))
	{
		return this->run_info(options);
	}

	// Else: Handle calculation request

	return this->run_calculation(options);
}

} // namespace arcsapp

