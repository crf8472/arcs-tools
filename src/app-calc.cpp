#ifndef __ARCSTOOLS_ARCALC_HPP__
#include "app-calc.hpp"
#endif

#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <tuple>
#include <unordered_set>
#include <vector>

#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#ifndef __LIBARCSDEC_CALCULATORS_HPP__
#include <arcsdec/calculators.hpp>
#endif

#ifndef __ARCSTOOLS_OUTPUTFORMATS_HPP__
#include "outputformats.hpp"
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
#ifndef __ARCSTOOLS_TOOLS_CALC_HPP__
#include "tools_calc.hpp"
#endif
#ifndef __ARCSTOOLS_TOOLS_FS_HPP__
#include "tools_fs.hpp"
#endif


using arcstk::ARId;
using arcstk::Checksums;
using arcstk::TOC;
using arcstk::make_empty_arid;

using arcsdec::ARCSCalculator;


// ARCalcOptions


constexpr uint8_t  ARCalcOptions::V1;
constexpr uint8_t  ARCalcOptions::V2;
constexpr uint8_t  ARCalcOptions::METAFILE;
constexpr uint8_t  ARCalcOptions::FMT;
constexpr uint8_t  ARCalcOptions::OUT;
constexpr uint8_t  ARCalcOptions::ALBUM;
constexpr uint8_t  ARCalcOptions::FIRST;
constexpr uint8_t  ARCalcOptions::LAST;
constexpr uint16_t ARCalcOptions::METAFILEPATH;
constexpr uint16_t ARCalcOptions::LIST_TOC_FORMATS;
constexpr uint16_t ARCalcOptions::LIST_AUDIO_FORMATS;

bool ARCalcOptions::has_operation_flags() const
{
	return rightmost_flag() < ARCalcOptions::LIST_TOC_FORMATS;
}


// ARCalcConfigurator


ARCalcConfigurator::ARCalcConfigurator(int argc, char** argv)
	: Configurator(argc, argv)
{
	// empty
}


ARCalcConfigurator::~ARCalcConfigurator() = default;


std::unique_ptr<Options> ARCalcConfigurator::do_configure_options(
		std::unique_ptr<Options> options)
{
	if (options->empty())
	{
		throw CallSyntaxException("No options or arguments");
	}

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
			if (options->is_set(ARCalcOptions::LIST_TOC_FORMATS))
			{
				// TODO Implement
				std::cout << "Option --list-toc-formats is not yet implemented\n";
			}

			if (options->is_set(ARCalcOptions::LIST_AUDIO_FORMATS))
			{
				// TODO Implement
				std::cout << "Option --list-audio-formats is not yet implemented\n";
			}

			return options;
		} else
		{
			// There are info flags as well as calculation flags

			if (options->is_set(ARCalcOptions::LIST_TOC_FORMATS))
			{
				std::cout << "Option --list-toc-formats is ignored\n";
			}

			if (options->is_set(ARCalcOptions::LIST_AUDIO_FORMATS))
			{
				std::cout << "Option --list-audio-formats is ignored\n";
			}

			// Proceed ...
		}
	}
	// If neither --v1 nor --v2 was passed activate both as default

	if (not options->is_set(ARCalcOptions::V1)
			and not options->is_set(ARCalcOptions::V2))
	{
		ARCS_LOG(DEBUG1) << "Activate default option: --v1 and --v2";

		options->set(ARCalcOptions::V1);
		options->set(ARCalcOptions::V2);
	}

	// Retrieve + verify metadata file

	{
		std::string metafilename(options->get(ARCalcOptions::METAFILE));

		if (metafilename.empty())
		{
			// no metadata file specified ...

			if (options->get_arguments().empty())
			{
				ARCS_LOG_ERROR << "No metadata and no audio file specified";
				return std::make_unique<Options>();
			}
		}
	}

	// Is outfile properly set, if any?

	{
		std::string outfile(options->get(ARCalcOptions::OUT));

		if (outfile.empty() && options->is_set(ARCalcOptions::OUT))
		{
			ARCS_LOG_ERROR << "-o but no outfilename.";
		}
	}

	return options;
}


std::unique_ptr<Options> ARCalcConfigurator::parse_options(CLIParser& cli)
{
	auto options = std::make_unique<ARCalcOptions>();

	this->check_for_option_with_argument("-o", ARCalcOptions::OUT,
			cli, *options);
	this->check_for_option_with_argument("-m", ARCalcOptions::METAFILE,
			cli, *options);

	if (options->is_set(ARCalcOptions::METAFILE))
	{
		// Provide Path of the Metafile as Searchpath

		options->put(ARCalcOptions::METAFILEPATH,
				file::path(options->get(ARCalcOptions::METAFILE)));
	}

	this->check_for_option("--v1", ARCalcOptions::V1, cli, *options);
	this->check_for_option("--v2", ARCalcOptions::V2, cli, *options);

	if (cli.consume_option("--album"))
	{
		options->set(ARCalcOptions::FIRST);
		options->set(ARCalcOptions::LAST);

		if (options->is_set(ARCalcOptions::METAFILE))
		{
			ARCS_LOG_WARNING << "Option --album is ignored when -m is passed";
		}
	}

	if (cli.consume_option("--first"))
	{
		if (options->is_set(ARCalcOptions::FIRST))
		{
			ARCS_LOG_WARNING
				<< "Option --first is redundant when --album is passed";
		}
		else if (options->is_set(ARCalcOptions::METAFILE))
		{
			ARCS_LOG_WARNING << "Option --first is ignored when -m is passed";
		}
		else
		{
			options->set(ARCalcOptions::FIRST);
		}
	}

	if (cli.consume_option("--last"))
	{
		if (options->is_set(ARCalcOptions::LAST))
		{
			ARCS_LOG_WARNING
				<< "Option --last is redundant when --album is passed";
		}
		else if (options->is_set(ARCalcOptions::METAFILE))
		{
			ARCS_LOG_WARNING << "Option --last is ignored when -m is passed";
		}
		else
		{
			options->set(ARCalcOptions::LAST);
		}
	}

	if (cli.consume_option("--list-toc-formats"))
	{
		options->set(ARCalcOptions::LIST_TOC_FORMATS);
	}

	if (cli.consume_option("--list-audio-formats"))
	{
		options->set(ARCalcOptions::LIST_AUDIO_FORMATS);
	}

	// Audio Filenames (whatever is left, must be a filename)

	std::string filename(cli.consume_argument());
	while (not filename.empty())
	{
		options->push_back_argument(filename);
		filename = cli.consume_argument();
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


std::unique_ptr<AlbumChecksumsTableFormat> ARCalcApplication::create_format(
		const Options &options, const Checksums &checksums) const
{
	if (std::string metafilename = options.get(ARCalcOptions::METAFILE);
			metafilename.empty())
	{
		// Without Offsets, ARId and TOC

		return std::make_unique<AlbumChecksumsTableFormat>(
				options.get_arguments().size(),
				2 + checksums[0].size(), // Checksum types + 2 metadata columns
				false, false, true, true); // length + filename

	} else
	{
		// With Offsets, ARId and TOC

		return std::make_unique<AlbumChecksumsTableFormat>(
				checksums.size(),
				3 + checksums[0].size(), // Checksum types + 3 metadata columns
				true, true, true, false); // no filenames
	}

	return nullptr;
}


std::unique_ptr<Configurator> ARCalcApplication::create_configurator(
		int argc, char** argv) const
{
	return std::make_unique<ARCalcConfigurator>(argc, argv);
}


std::string ARCalcApplication::do_name() const
{
	return "calc";
}


int ARCalcApplication::do_run(const Options &options)
{
	auto [ checksums, arid, toc ] = this->calculate(options);

	if (checksums.size() == 0)
	{
		this->fatal_error("Calculation returned no checksums");
	}

	auto format = this->create_format(options, checksums);

	if (toc)
	{
		format->format(checksums, arid, *toc);
	} else
	{
		format->format(checksums, options.get_arguments());
	}

	this->print(*format->lines(), options.get(ARCalcOptions::OUT));

	return EXIT_SUCCESS;
}


void ARCalcApplication::do_print_usage() const
{
	std::cout << this->name() << " [OPTIONS] <filename1>, <filename2>, ...\n\n";

	std::cout << "Options:\n\n";

	std::cout << "-q                 only output ARCSs, nothing else\n\n";
	std::cout << "-v <i>             verbous output (loglevel 0-6)\n\n";
	std::cout << "-l <filename>      specify log file for output\n\n";
	std::cout << "-m <filename>      specify metadata file (CUE) to use\n\n";
	std::cout << "-o <filename>      specify result file for output\n\n";
	std::cout << "--v1               output ARCS v1 (default)\n\n";
	std::cout << "--v2               output ARCS v2 (default)\n\n";
	std::cout << "--first            treat first audio file as first track\n";
	std::cout << "                   (implied if -m is specified)\n\n";
	std::cout << "--last             treat last audio file as last track\n";
	std::cout << "                   (implied if -m is specified)\n\n";
	std::cout << "--album            short for --first --last\n";
	std::cout << "                   (implied if -m is specified)\n\n";
	std::cout << "--version          print version and exit\n";
	std::cout << "                   (ignoring any other options)\n\n";
}

