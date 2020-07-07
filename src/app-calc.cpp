#include <arcstk/identifier.hpp>
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


// CALCBASE

constexpr OptionCode CALCBASE::LIST_TOC_FORMATS;
constexpr OptionCode CALCBASE::LIST_AUDIO_FORMATS;
constexpr OptionCode CALCBASE::METAFILE;
constexpr OptionCode CALCBASE::NOTRACKS;
constexpr OptionCode CALCBASE::NOFILENAMES;
constexpr OptionCode CALCBASE::NOOFFSETS;
constexpr OptionCode CALCBASE::NOLENGTHS;
constexpr OptionCode CALCBASE::NOLABELS;
constexpr OptionCode CALCBASE::COLDELIM;
constexpr OptionCode CALCBASE::PRINTID;
constexpr OptionCode CALCBASE::PRINTURL;

constexpr OptionCode CALCBASE::MAX_CONSTANT;


// CALC

constexpr OptionCode CALC::FIRST;
constexpr OptionCode CALC::LAST;
constexpr OptionCode CALC::ALBUM;
constexpr OptionCode CALC::NOV1;
constexpr OptionCode CALC::NOV2;
constexpr OptionCode CALC::SUMSONLY;
constexpr OptionCode CALC::TRACKSASCOLS;


// ARCalcConfiguratorBase


bool ARCalcConfiguratorBase::calculation_requested(const Options &options)
{
	return options.is_set(CALCBASE::METAFILE) or not options.no_arguments();
}


std::unique_ptr<Options> ARCalcConfiguratorBase::configure_calcbase_options(
		std::unique_ptr<Options> options) const
{
	if (not calculation_requested(*options))
	{
		ARCS_LOG_INFO <<
			"No calculation task requested, no configuring required";
		return options;
	}

	// Info Options

	if (options->is_set(CALCBASE::LIST_TOC_FORMATS))
	{
		ARCS_LOG_WARNING <<
			"Option LIST_TOC_FORMATS is ignored due to calculation task";
		options->unset(CALCBASE::LIST_TOC_FORMATS);
	}

	if (options->is_set(CALCBASE::LIST_AUDIO_FORMATS))
	{
		ARCS_LOG_WARNING <<
			"Option LIST_AUDIO_FORMATS is ignored due to calculation task";
		options->unset(CALCBASE::LIST_AUDIO_FORMATS);
	}

	// Metafile: Get Path + Activate Album Mode

	if (options->is_set(CALCBASE::METAFILE))
	{
		if (options->get(CALCBASE::METAFILE).empty())
		{
			// No Metadata File Specified

			if (options->no_arguments())
			{
				ARCS_LOG_ERROR << "No metafile and no audiofile specified";
				return std::make_unique<Options>();
			}
		}
	}

	return options;
}


// ARCalcConfigurator


const std::vector<std::pair<Option, OptionCode>>&
	ARCalcConfigurator::do_supported_options() const
{
	const static std::vector<std::pair<Option, OptionCode>> local_options =
	{
		// Calculation Input Options

		{{      "first",  false, "~", "Treat first audio file as first track" },
			CALC::FIRST },
		{{      "last",   false, "~", "Treat last audio file as last track" },
			CALC::LAST },
		{{      "album",  false, "~", "Abbreviates --first --last" },
			CALC::ALBUM },
		{{ 'm', "metafile", true, "none", "Specify toc metadata file to use" },
			CALC::METAFILE },

		// Calculation Output Options

		{{  "no-v1", false, "FALSE", "Do not provide ARCSv1" },
			CALC::NOV1 },
		{{  "no-v2", false, "FALSE", "Do not provide ARCSv2" },
			CALC::NOV2 },
		{{  "print-sums-only", false, "FALSE", "Print only checksums" },
			CALC::SUMSONLY},
		{{  "tracks-as-cols", false, "FALSE", "Print tracks as columns" },
			CALC::TRACKSASCOLS},

		// from CALCBASE
		{{  "no-track-nos", false, "FALSE", "Do not print track numbers" },
			CALC::NOTRACKS},
		{{  "no-filenames", false, "FALSE", "Do not print the filenames" },
			CALC::NOFILENAMES},
		{{  "no-offsets", false, "FALSE", "Do not print track offsets" },
			CALC::NOOFFSETS},
		{{  "no-lengths", false, "FALSE", "Do not print track lengths" },
			CALC::NOLENGTHS},
		{{  "no-labels", false, "FALSE", "Do not print column or row labels" },
			CALC::NOLABELS},
		{{  "col-delim", true, "<blank>", "Specify column delimiter" },
			CALC::COLDELIM},
		{{  "print-id", false, "FALSE", "Print AccurateRip Id of the album" },
			CALC::PRINTID},
		{{  "print-url", false, "FALSE", "Print AccurateRip URL of the album" },
			CALC::PRINTURL},

		// Info Output Options

		{{  "list-toc-formats", false, "FALSE",
				"List all supported file formats for TOC metadata" },
			CALC::LIST_TOC_FORMATS },
		{{  "list-audio-formats", false, "FALSE",
				"List all supported audio codec/container formats" },
			CALC::LIST_AUDIO_FORMATS }
	};

	return local_options;
}


std::unique_ptr<Options> ARCalcConfigurator::do_configure_options(
		std::unique_ptr<Options> coptions)
{
	auto options = this->configure_calcbase_options(std::move(coptions));

	// Determine whether to set ALBUM mode

	if (options->is_set(CALC::METAFILE))
	{
		// Activate Album Mode

		if (options->is_set(CALC::ALBUM))
		{
			ARCS_LOG_INFO <<
				"Option ALBUM is redundant when METAFILE is passed";
		} else
		{
			ARCS_LOG(DEBUG1) << "Activate option ALBUM due to METAFILE";
			options->set(CALC::ALBUM);
		}

		if (options->is_set(CALC::FIRST))
		{
			ARCS_LOG_INFO <<
				"Option FIRST is redundant when METAFILE is passed";
		} else
		{
			ARCS_LOG(DEBUG1) << "Activate option FIRST due to METAFILE";
			options->set(CALC::FIRST);
		}

		if (options->is_set(CALC::LAST))
		{
			ARCS_LOG_INFO <<
				"Option LAST is redundant when METAFILE is passed";
		} else
		{
			ARCS_LOG(DEBUG1) << "Activate option LAST due to METAFILE";
			options->set(CALC::LAST);
		}
	} else
	{
		// No metafile: Album Mode or Not?

		if (options->is_set(CALC::ALBUM))
		{
			if (not options->is_set(CALC::FIRST))
			{
				ARCS_LOG(DEBUG1) << "Activate option FIRST due to ALBUM";

				options->set(CALC::FIRST);
			} else
			{
				ARCS_LOG_INFO <<
					"Option FIRST is redundant when ALBUM is passed";
			}

			if (not options->is_set(CALC::LAST))
			{
				ARCS_LOG(DEBUG1) << "Activate option LAST due to ALBUM";

				options->set(CALC::LAST);
			} else
			{
				ARCS_LOG_INFO <<
					"Option LAST is redundant when ALBUM is passed";
			}
		} else
		{
			if (options->is_set(CALC::FIRST)
				and options->is_set(CALC::LAST))
			{
				ARCS_LOG(DEBUG1) <<
					"Activate option ALBUM due to FIRST and LAST";
				options->set(CALC::ALBUM);
			}
		}
	}

	// Printing options

	if (options->is_set(CALC::SUMSONLY))
	{
		options->set(CALC::NOTRACKS);
		options->set(CALC::NOFILENAMES);
		options->set(CALC::NOOFFSETS);
		options->set(CALC::NOLENGTHS);
		options->set(CALC::NOLABELS); // Multiple Checksum types?
	}

	return options;
}


// ARCalcApplication


std::tuple<Checksums, ARId, std::unique_ptr<TOC>> ARCalcApplication::calculate(
	const std::string &metafilename,
	const std::vector<std::string> &audiofilenames,
	const bool as_first,
	const bool as_last,
	const std::set<arcstk::checksum::type> &types)
{
	using ChecksumType = arcstk::checksum::type;

	// XXX Determine whether to request ARCS2+1 or ARCS1-only
	ChecksumType checksum_type = types.find(ChecksumType::ARCS2) != types.end()
		? ChecksumType::ARCS2
		: ChecksumType::ARCS1;

	calc::ARCSMultifileAlbumCalculator c { checksum_type };

	auto [ checksums, arid, toc ] = metafilename.empty()
		? c.calculate(audiofilenames, as_first, as_last) //Tracks/Album w/o TOC
		: c.calculate(audiofilenames, metafilename);     //Album: with TOC

	return std::make_tuple(checksums, arid, std::move(toc));
}


std::unique_ptr<ChecksumsResultPrinter> ARCalcApplication::configure_format(
		const Options &options) const
{
	const bool has_toc = !options.get(CALC::METAFILE).empty();

	// Print track number if they are not forbidden and a TOC is present
	const bool prints_tracks = options.is_set(CALC::NOTRACKS)
		? false
		: has_toc;

	// Print filenames if they are not forbidden and a TOC is _not_ present
	const bool prints_filenames = options.is_set(CALC::NOFILENAMES)
		? false
		: not has_toc;

	// Print offsets if they are not forbidden and a TOC is present
	const bool prints_offsets = options.is_set(CALC::NOOFFSETS)
		? false
		: has_toc;

	// Print lengths if they are not forbidden
	const bool prints_lengths = not options.is_set(CALC::NOLENGTHS);

	// Set column delimiter
	const std::string coldelim = options.is_set(CALC::COLDELIM)
		? options.get(CALC::COLDELIM)
		: " ";

	// Decide which implementation

	std::unique_ptr<ChecksumsResultPrinter> format;

	if (options.is_set(CALC::TRACKSASCOLS))
	{
		format = std::make_unique<AlbumTracksTableFormat>(
			not options.is_set(CALC::NOLABELS),
			prints_tracks, prints_offsets, prints_lengths, prints_filenames,
			coldelim);
	} else
	{
		format = std::make_unique<AlbumChecksumsTableFormat>(
			not options.is_set(CALC::NOLABELS),
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

	if (not options.is_set(CALC::NOV1))
	{
		types.insert(arcstk::checksum::type::ARCS1);
	}
	if (not options.is_set(CALC::NOV2))
	{
		types.insert(arcstk::checksum::type::ARCS2);
	}

	return types;
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

	auto [ checksums, arid, toc ] = this->calculate(
			options.get(CALC::METAFILE), options.get_arguments(),
			options.is_set(CALC::FIRST), options.is_set(CALC::LAST),
			requested_types);

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

	if (options.is_set(CALC::PRINTID) or options.is_set(CALC::PRINTURL))
	{
		const std::unique_ptr<ARIdTableFormat> idformat =
			std::make_unique<ARIdTableFormat>();

		idformat->set_id(options.is_set(CALC::PRINTID));
		idformat->set_url(options.is_set(CALC::PRINTURL));

		idformat->use(std::make_tuple(&arid, nullptr));

		output(*idformat);
	}

	auto format = configure_format(options);

	auto filenames = toc
		? arcstk::toc::get_filenames(toc)
		: options.get_arguments();

	auto album_mode = options.is_set(CALC::ALBUM);

	format->use(std::make_tuple(&checksums, &filenames, toc.get(), &arid,
				&album_mode));
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


std::unique_ptr<Configurator> ARCalcApplication::create_configurator() const
{
	return std::make_unique<ARCalcConfigurator>();
}


int ARCalcApplication::do_run(const Options &options)
{
	if (ARCalcConfiguratorBase::calculation_requested(options))
	{
		return this->run_calculation(options);
	}

	// If only info options are present, handle info request

	if (options.is_set(CALC::LIST_TOC_FORMATS))
	{
		output(SupportedFormats::toc(), options.output());
	}

	if (options.is_set(CALC::LIST_AUDIO_FORMATS))
	{
		output(SupportedFormats::audio(), options.output());
	}

	return EXIT_SUCCESS;
}

} // namespace arcsapp
