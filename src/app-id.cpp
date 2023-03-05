#ifndef __ARCSTOOLS_APPID_HPP__
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

#ifndef __LIBARCSDEC_SELECTION_HPP__
#include <arcsdec/selection.hpp>      // for FileReaderSelection
#endif
#ifndef __LIBARCSDEC_CALCULATE_HPP__
#include <arcsdec/calculators.hpp>
#endif

#ifndef __ARCSTOOLS_APPREGISTRY_HPP__
#include "appregistry.hpp"
#endif
#ifndef __ARCSTOOLS_CONFIG_HPP__
#include "config.hpp"
#endif
#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#include "layouts.hpp"                // for ARIdLayout, ...
#endif
#ifndef __ARCSTOOLS_TOOLS_INFO_HPP__
#include "tools-info.hpp"             // for AvailableFileReaders
#endif

namespace arcsapp
{

namespace registered
{
const auto id = RegisterApplicationType<ARIdApplication>("id");
}

using arcstk::ARId;

using arcsdec::ARIdCalculator;


// ARIdConfigurator


const std::vector<std::pair<Option, OptionCode>>&
	ARIdConfigurator::do_supported_options() const
{
	const static std::vector<std::pair<Option, OptionCode>> local_options =
	{
			// from FORMATBASE

			{{  "reader", true, "auto",
				"Force use of audio reader with specified id" },
				ARIdOptions::READERID},

			{{  "parser", true, "auto",
				"Force use of toc parser with specified id" },
				ARIdOptions::PARSERID},

			{{  "list-toc-formats", false, "FALSE",
				"List all supported file formats for TOC metadata" },
				ARIdOptions::LIST_TOC_FORMATS },

			{{  "list-audio-formats", false, "FALSE",
				"List all supported audio codec/container formats" },
				ARIdOptions::LIST_AUDIO_FORMATS },

			// from ARIdOptions

			{{  "cddb-id", false, "FALSE", "Print the CDDB id" },
				ARIdOptions::CDDBID },

			{{  "db-id", false, "FALSE",
				"Print the AccurateRip DB ID (equivalent to filename)" },
				ARIdOptions::DBID },

			{{  "filename", false, "FALSE",
				"Print the AccurateRip DB ID (equivalent to db-id)" },
				ARIdOptions::DBID },

			{{ "no-labels", false, "FALSE", "No labels on columns and rows" },
				ARIdOptions::NOLABELS },

			{{  "url", false, "FALSE",
				"Print the AccurateRip URL" },
				ARIdOptions::URL },

			{{ "profile", false, "FALSE", "Print all information" },
				ARIdOptions::PROFILE },

			{{  "url-prefix", true, "none",
				"Use the specified prefix instead of the default "
					"'http://accuraterip.com/accuraterip/'" },
				ARIdOptions::URLPREFIX },

			{{ 'a', "audiofile", true, "none", "Specify input audio file" },
				ARIdOptions::AUDIOFILE }
	};

	return local_options;
}


// ARIdApplication


int ARIdApplication::run_calculation(const Options &options)
{
	// Compute requested values

	const auto metafilename  = options.argument(0);
	const auto audiofilename = options.value(ARIdOptions::AUDIOFILE);

	std::unique_ptr<ARId> arid = nullptr;

	{
		// Configure selections (e.g. --reader and --parser)

		const IdSelection id_selection;

		auto audio_selection = options.is_set(ARIdOptions::READERID)
			? id_selection(options.value(ARIdOptions::READERID))
			: nullptr;

		auto toc_selection = options.is_set(ARIdOptions::PARSERID)
			? id_selection(options.value(ARIdOptions::PARSERID))
			: nullptr;

		if (toc_selection)
		{
			ARCS_LOG_INFO << "Select parser " <<
				options.value(ARIdOptions::PARSERID);
		}

		// If no selections are assigned, the libarcsdec default selections
		// will be used.

		ARIdCalculator c;

		if (audio_selection) { c.set_audio_selection(audio_selection.get()); }
		if (toc_selection)   { c.set_toc_selection(toc_selection.get()); }

		arid = c.calculate(audiofilename, metafilename);
	}

	if (!arid) { this->fatal_error("Could not compute AccurateRip id."); }

	// Adjust format and print information

	std::unique_ptr<ARIdLayout> layout;

	if (options.is_set(ARIdOptions::PROFILE))
	{
		layout = std::make_unique<ARIdTableLayout>(
			true, true, true, true, true, true, true, true);
	} else
	{
		// Use labels iff allowed and more than one property is to be printed
		const bool print_labels = !options.is_set(ARIdOptions::NOLABELS)
			and (
			1 < options.is_set(ARIdOptions::ID)
			+ options.is_set(ARIdOptions::URL)
			+ options.is_set(ARIdOptions::DBID)
			+ options.is_set(ARIdOptions::CDDBID));

		layout = std::make_unique<ARIdTableLayout>(
			print_labels,
			options.is_set(ARIdOptions::ID),
			options.is_set(ARIdOptions::URL),
			options.is_set(ARIdOptions::DBID),
			false /* no track count */,
			false /* no disc id 1 */,
			false /* no disc id 2 */,
			options.is_set(ARIdOptions::CDDBID)
		);
	}

	auto result = layout->format(*arid, options.value(ARIdOptions::URLPREFIX));

	if (not result.empty() and result.back() != '\n') { result += '\n'; }

	if (not options.value(OPTION::OUTFILE).empty())
	{
		Output::instance().to_file(options.value(OPTION::OUTFILE));
	}
	Output::instance().output(result);

	return EXIT_SUCCESS;
}


std::string ARIdApplication::do_name() const
{
	return "id";
}


std::string ARIdApplication::do_call_syntax() const
{
	return "[OPTIONS] FILENAME";
}


std::unique_ptr<Configurator> ARIdApplication::create_configurator() const
{
	return std::make_unique<ARIdConfigurator>();
}


int ARIdApplication::do_run(const Options &options)
{
	// Is an actual calculation requested?
	if (options.is_set(ARIdOptions::AUDIOFILE) || !options.no_arguments())
	{
		return this->run_calculation(options);
	}

	if (options.is_set(ARIdOptions::LIST_TOC_FORMATS))
	{
		Output::instance().output("TOC:\n");
		Output::instance().output(AvailableFileReaders::toc());
	}

	if (options.is_set(ARIdOptions::LIST_AUDIO_FORMATS))
	{
		Output::instance().output("Audio:\n");
		Output::instance().output(AvailableFileReaders::audio());
	}

	return EXIT_SUCCESS;
}

} // namespace arcsapp

