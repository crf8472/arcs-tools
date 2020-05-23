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

#ifndef __ARCSTOOLS_CLIPARSE_HPP__
#include "cliparse.hpp"             // for CLIParser, __ARCSTOOLS_CLIPARSE_H...
#endif
#ifndef __ARCSTOOLS_CONFIG_HPP__
#include "config.hpp"
#endif
#ifndef __ARCSTOOLS_FORMAT_HPP__
#include "format.hpp"               // for operator<<
#endif
#ifndef __ARCSTOOLS_PRINTERS_HPP__
#include "printers.hpp"
#endif
#ifndef __ARCSTOOLS_TOOLS_CALC_HPP__
#include "tools_calc.hpp"           // for ARCSMultifileAlbumCalculator
#endif
#ifndef __ARCSTOOLS_TOOLS_FS_HPP__
#include "tools_fs.hpp"             // for path
#endif

class Options;

using arcstk::ARId;
using arcstk::Checksums;
using arcstk::TOC;
using arcstk::make_empty_arid;

using arcsdec::ARCSCalculator;
using arcsdec::TOCParser;


// ARCalcOptions


constexpr uint8_t  ARCalcOptions::NOV1;
constexpr uint8_t  ARCalcOptions::NOV2;
constexpr uint8_t  ARCalcOptions::ALBUM;
constexpr uint8_t  ARCalcOptions::FIRST;
constexpr uint8_t  ARCalcOptions::LAST;
constexpr uint8_t  ARCalcOptions::METAFILE;
constexpr uint16_t ARCalcOptions::LIST_TOC_FORMATS;
constexpr uint16_t ARCalcOptions::LIST_AUDIO_FORMATS;
constexpr uint16_t ARCalcOptions::METAFILEPATH;


// ARCalcConfigurator


ARCalcConfigurator::ARCalcConfigurator(int argc, char** argv)
	: Configurator(argc, argv)
{
	if (supported().size() == 5) // FIXME Magic number
	{
		this->support(
			{      "no-v1",    false, "FALSE",
				"do not provide ARCSv1" },
				ARCalcOptions::NOV1);
		this->support(
			{      "no-v2",    false, "FALSE",
				"do not provide ARCSv2" },
				ARCalcOptions::NOV2);
		this->support(
			{      "album",    false, "~",
				"abbreviates --first --last" },
				ARCalcOptions::ALBUM);
		this->support(
			{      "first",    false, "~",
				"treat first audio file as first track" },
				ARCalcOptions::FIRST);
		this->support(
			{      "last",     false, "~",
				"treat last audio file as last track" },
				ARCalcOptions::LAST);
		this->support(
			{ 'm', "metafile", true, "none",
				"specify metadata file (CUE) to use" },
				ARCalcOptions::METAFILE);
		this->support(
			{      "list-toc-formats",  false,   "FALSE",
				"list all supported file formats for TOC metadata" },
				ARCalcOptions::LIST_TOC_FORMATS);
		this->support(
			{      "list-audio-formats",  false, "FALSE",
				"list all supported audio codec/container formats" },
				ARCalcOptions::LIST_AUDIO_FORMATS);
	}
}


ARCalcConfigurator::~ARCalcConfigurator() noexcept
= default;


int ARCalcConfigurator::do_parse_arguments(CLIParser& cli, Options &options)
		const
{
	// Respect multiple arguments

	auto args = this->arguments(cli);
	auto total_args = args.size();

	for (const auto& arg : args)
	{
		options.append(arg);
	}

	return static_cast<int>(total_args);
}


std::unique_ptr<Options> ARCalcConfigurator::do_configure_options(
		std::unique_ptr<Options> options)
{
	// TODO A hack: we know that all flags left of LIST_TOC_FORMATS are
	// info-do-nothing flags. Hence, if the leftmost (set) flag is bigger (== to
	// the left) of this flag, there are info flags. If, furthermore, the
	// rightmost (set) flag is bigger than that, there are _only_ info flags.

	// If there are info flags
	if (options->leftmost_flag() >= ARCalcOptions::LIST_TOC_FORMATS)
	{
		// If there are no calculation flags
		if (options->rightmost_flag() >= ARCalcOptions::LIST_TOC_FORMATS)
		{
			return options;
		}
		else
		{
			// There are info flags as well as calculation flags

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

			// Proceed ...
		}
	}

	// Metafile: get path + activate album mode

	if (options->is_set(ARCalcOptions::METAFILE))
	{
		auto metafilename(options->get(ARCalcOptions::METAFILE));

		if (metafilename.empty())
		{
			// no metadata file specified ...

			 // TODO Better ARCalcOptions::AUDIOFILES
			if (options->get_arguments().empty())
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
	}

	// Album or not

	if (options->is_set(ARCalcOptions::ALBUM))
	{
		if (options->is_set(ARCalcOptions::METAFILE))
		{
			ARCS_LOG_INFO << "Option ALBUM is redundant when METAFILE is passed";
		} else
		{
			ARCS_LOG(DEBUG1) << "Activate album mode";

			options->set(ARCalcOptions::FIRST);
			options->set(ARCalcOptions::LAST);
		}
	}

	return options;
}


// ARCalcApplication


std::tuple<Checksums, ARId, std::unique_ptr<TOC>> ARCalcApplication::calculate(
		const Options &options)
{
	std::vector<std::string> audiofilenames = options.get_arguments();
	std::string metafilename = options.get(ARCalcOptions::METAFILE);

	if (metafilename.empty())
	{
		// No Offsets => No ARId => No TOC
		// => No Album information, but may be requested as album by options

		ARCSCalculator c;

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

		ARCSMultifileAlbumCalculator c;

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
	bool show_tracks    = false;
	bool show_offsets   = false;
	bool show_lengths   = true;
	bool show_filenames = false;

	if (options.get(ARCalcOptions::METAFILE).empty())
	{
		// Without Offsets, ARId and TOC
		show_filenames = true;
	} else
	{
		// With Offsets, ARId and TOC
		show_tracks  = true;
		show_offsets = true;
	}

	// Configure which implementation + which columns to show.
	// All other details are hidden.

	return std::make_unique<AlbumChecksumsTableFormat>(0, 0,
				show_tracks, show_offsets, show_lengths, show_filenames);
}


int ARCalcApplication::run_info(const Options &options)
{
	FormatCollector collector;
	auto apply_func = std::bind(&FormatCollector::add, &collector,
			std::placeholders::_1);

	if (options.is_set(ARCalcOptions::LIST_TOC_FORMATS))
	{
		TOCParser p;
		p.selection().traverse_descriptors(apply_func);
	}

	if (options.is_set(ARCalcOptions::LIST_AUDIO_FORMATS))
	{
		ARCSCalculator c;
		c.selection().traverse_descriptors(apply_func);
	}

	FormatList list(collector.info().size());
	for (const auto& entry : collector.info())
	{
		list.append_line(entry[0], entry[1], entry[2], entry[3]);
	}

	output(list, options.output());

	return EXIT_SUCCESS;
}


int ARCalcApplication::run_calculation(const Options &options)
{
	auto [ checksums, arid, toc ] = this->calculate(options);

	if (checksums.size() == 0)
	{
		this->fatal_error("Calculation returned no checksums");
	}

	// Configure output stream

	// FIXME This should be handled by output()
	std::streambuf *buf;
	std::ofstream out_file_stream;
	if (auto filename = options.output(); filename.empty())
	{
		buf = std::cout.rdbuf();
	} else
	{
		out_file_stream.open(filename);
		buf = out_file_stream.rdbuf();
	}
	std::ostream out_stream(buf);

	// Print formatted results to output stream

	auto format = configure_format(options);

	auto filenames = toc
		? arcstk::toc::get_filenames(toc)
		: options.get_arguments();

	format->out(out_stream, checksums, filenames, toc.get(), arid);

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

	if (options.rightmost_flag() >= ARCalcOptions::LIST_TOC_FORMATS
			and not options.empty())
	{
		return this->run_info(options);
	}

	// Else: Handle calculation request

	return this->run_calculation(options);
}
