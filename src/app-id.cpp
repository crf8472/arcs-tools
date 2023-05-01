#ifndef __ARCSTOOLS_APPID_HPP__
#include "app-id.hpp"
#endif

#include <cstdlib>                  // for EXIT_SUCCESS
#include <iterator>                 // for end
#include <memory>                   // for unique_ptr, make_unique
#include <string>                   // for string
#include <utility>                  // for make_pair, move, pair

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
#include "appregistry.hpp"            // for RegisterApplicationType
#endif
#ifndef __ARCSTOOLS_CONFIG_HPP__
#include "config.hpp"                 // for Configurator
#endif
#ifndef __ARCSTOOLS_RESULT_HPP__
#include "result.hpp"                 // for ResultObject
#endif
#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#include "layouts.hpp"                // for ARIdLayout, ARIdTableLayout ...
#endif
#ifndef __ARCSTOOLS_TOOLS_CALC_HPP__
#include "tools-calc.hpp"             // for IdSelection
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


void ARIdConfigurator::flush_local_options(OptionRegistry& r) const
{
	using std::end;
	r.insert(end(r),
	{
		// from FORMATBASE

		{ ARIdOptions::READERID,
		{ "reader", true, "auto",
			"Force use of audio reader with specified id" }},

		{ ARIdOptions::PARSERID,
		{ "parser", true, "auto",
			"Force use of toc parser with specified id" }},

		{ ARIdOptions::LIST_TOC_FORMATS,
		{ "list-toc-formats", false, "FALSE",
			"List all supported file formats for TOC metadata" }},

		{ ARIdOptions::LIST_AUDIO_FORMATS,
		{ "list-audio-formats", false, "FALSE",
			"List all supported audio codec/container formats" }},

		// from ARIdOptions

		{ ARIdOptions::CDDBID,
		{ "cddb-id", false, "FALSE", "Print the CDDB id" }},

		{ ARIdOptions::DBID,
		{ "db-id", false, "FALSE",
			"Print the AccurateRip DB ID (equivalent to filename)" }},

		{ ARIdOptions::FILENAME,
		{ "filename", false, "FALSE",
			"Print the AccurateRip DB ID (equivalent to db-id)" }},

		{ ARIdOptions::NOLABELS,
		{ "no-labels", false, "FALSE", "No labels on columns and rows" }},

		{ ARIdOptions::URL,
		{ "url", false, "FALSE",
			"Print the AccurateRip URL" }},

		{ ARIdOptions::PROFILE,
		{ "profile", false, "FALSE", "Print all information" }},

		{ ARIdOptions::URLPREFIX,
		{ "url-prefix", true, "none",
			"Use the specified prefix instead of the default "
			"'http://accuraterip.com/accuraterip/'" }},

		{ ARIdOptions::AUDIOFILE,
		{ 'a', "audiofile", true, "none", "Specify input audio file" }}
	});
}


std::unique_ptr<Options> ARIdConfigurator::do_configure_options(
			std::unique_ptr<Options> options) const
{
	// Use DBID instead of the equivalent FILENAME
	if (options->is_set(ARIdOptions::FILENAME))
	{
		options->set(ARIdOptions::DBID);
		options->unset(ARIdOptions::FILENAME);
	}
	return options;
}


// ARIdApplication


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


bool ARIdApplication::calculation_requested(const Options &options) const
{
	return options.is_set(ARIdOptions::AUDIOFILE) || not options.no_arguments();
}


auto ARIdApplication::run_calculation(const Options &options) const
	-> std::pair<int, std::unique_ptr<Result>>
{
	// Compute requested values

	const auto metafilename  = options.argument(0);
	const auto audiofilename = options.value(ARIdOptions::AUDIOFILE);

	std::unique_ptr<ARId> arid = nullptr;

	{
		// Configure selections (e.g. --reader and --parser)

		const calc::IdSelection id_selection;

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

		arid = c.calculate(metafilename, audiofilename);
	}

	if (!arid) { this->fatal_error("Could not compute AccurateRip id."); }

	// Build the result object

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

	auto id = RichARId{*arid, std::move(layout),
		options.value(ARIdOptions::URLPREFIX)};

	//if (not result_str.empty() and result_str.back() != '\n')
	//{
	//	result_str += '\n';
	//}

	//return { EXIT_SUCCESS, std::move(r) };
	return std::make_pair(EXIT_SUCCESS,
			std::make_unique<ResultObject<RichARId>>(std::move(id)));
}


int ARIdApplication::do_run(const Options &options)
{
	// Is an actual calculation requested?
	if (calculation_requested(options))
	{
		if (options.is_set(ARIdOptions::LIST_TOC_FORMATS))
		{
			// TODO Warn about ignored option or do it in Configurator
		}
		if (options.is_set(ARIdOptions::LIST_AUDIO_FORMATS))
		{
			// TODO Warn about ignored option or do it in Configurator
		}

		auto [ exit_code, result ] = this->run_calculation(options);

		this->output(std::move(result), options);
		return exit_code;
	}

	// If only info options are present, handle info request

	if (options.is_set(ARIdOptions::LIST_TOC_FORMATS))
	{
		Output::instance().output(AvailableFileReaders::toc());
	}

	if (options.is_set(ARIdOptions::LIST_AUDIO_FORMATS))
	{
		Output::instance().output(AvailableFileReaders::audio());
	}

	return EXIT_SUCCESS;
}

} // namespace arcsapp

