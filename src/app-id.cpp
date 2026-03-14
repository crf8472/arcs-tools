/**
 * \internal
 *
 * \file
 *
 * \brief Implements symbols from app-id.hpp.
 */

#ifndef ARCSTOOLS_APPID_HPP__
#include "app-id.hpp"
#endif

#include <cstdlib>                  // for EXIT_SUCCESS
#include <iterator>                 // for end
#include <memory>                   // for unique_ptr, make_unique
#include <string>                   // for string
#include <utility>                  // for make_pair, move, pair

#ifndef LIBARCSTK_IDENTIFIER_HPP__
#include <arcstk/identifier.hpp>
#endif
#ifndef LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#ifndef LIBARCSDEC_SELECTION_HPP__
#include <arcsdec/selection.hpp>      // for FileReaderSelection
#endif
#ifndef LIBARCSDEC_CALCULATE_HPP__
#include <arcsdec/calculators.hpp>
#endif

#ifndef ARCSTOOLS_APPREGISTRY_HPP__
#include "appregistry.hpp"            // for RegisterApplicationType
#endif
#ifndef ARCSTOOLS_CONFIG_HPP__
#include "config.hpp"                 // for Configurator, OptionCode
#endif
#ifndef ARCSTOOLS_RESULT_HPP__
#include "result.hpp"                 // for ResultObject
#endif
#ifndef ARCSTOOLS_TOOLS_ARID_HPP__
#include "tools-arid.hpp"             // for ARIdLayout
#endif
#ifndef ARCSTOOLS_TOOLS_CALC_HPP__
#include "tools-calc.hpp"             // for IdSelection
#endif


namespace arcsapp
{
inline namespace v_1_0_0
{

namespace registered
{
// Enable ApplicationFactory::lookup() to find this application by its name
const auto id = RegisterApplicationType<ARIdApplication>("id");
}

// libarcstk
using arcstk::ARId;
using arcstk::AudioSize;
using arcstk::make_arid;

// libarcsdec
using arcsdec::calc::AudioInfo;
using arcsdec::calc::ToCParser;

// arcsapp
using arid::ARIdLayout;
using arid::ARIdTableLayout;
using arid::RichARId;


// ARIdConfigurator


void ARIdConfigurator::do_flush_local_options(OptionRegistry& r) const
{
	using std::cend;
	r.insert(cend(r),
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

	// No output attributes specified? => Print just the ID as the default

	if(not (options->is_set(ARIdOptions::CDDBID)
			|| options->is_set(ARIdOptions::DBID)
			|| options->is_set(ARIdOptions::URL)))
	{
		options->set(ARIdOptions::ID);
	}

	return options;
}


// ARIdApplication


bool ARIdApplication::do_calculation_requested(const Configuration& config)
	const
{
	return config.is_set(ARIdOptions::AUDIOFILE) || not config.no_arguments();
}


auto ARIdApplication::do_run_calculation(const Configuration& config) const
	-> std::pair<int, std::unique_ptr<Result>>
{
	const auto metafilename = config.argument(0);

	// Step 1: Parse metadata to acquire ToC

	auto toc = std::unique_ptr<ToC> {};

	{ // scope

		auto parser = ToCParser {};

		const auto toc_selection {
			create_selection(ARIdOptions::PARSERID, config)
		};

		if (toc_selection)
		{
			parser.set_selection(toc_selection.get());
		}

		toc = parser.parse(metafilename);
	}

	if (!toc) { this->fatal_error("Could not acquire ToC."); }


	// Step 2: Calculate ARId by optionally using the audiofile

	auto arid = arcstk::make_empty_arid();

	if (toc->complete())
	{
		// Audio file is not required

		arid = make_arid(*toc);

	} else
	{
		// Audio file is required to get the leadout

		auto audiofilename = config.value(ARIdOptions::AUDIOFILE);

		if (audiofilename.empty())
		{
			ARCS_LOG_DEBUG << "No audio file specified, try to derive from ToC";

			using calc::ToCFiles;

			const auto& [ single, pw_dist, files ] = ToCFiles::get(*toc);

			if (!single)
			{
				throw std::runtime_error("Could not calculate ARId from "
						"audio input spanning more than 1 file.");
			}
			// Since we guarantee a single file, we can safely ignore pw_dist

			audiofilename = ToCFiles::expand_path(metafilename, files.front());

			ARCS_LOG_DEBUG << "Try to get leadout from file: " << audiofilename;
		}

		{ // scope
			auto a = AudioInfo {};
			auto audio_sel { create_selection(ARIdOptions::READERID, config) };
			if (audio_sel)
			{
				a.set_selection(audio_sel.get());
			}

			const auto audio_size = a.size(audiofilename);

			ARCS_LOG_DEBUG << "Got leadout: " << audio_size->frames();

			arid = make_arid(*toc, *audio_size);
		}
	}

	if (!arid) { this->fatal_error("Could not compute AccurateRip id."); }


	// Step 3: Build the result object

	auto layout = std::unique_ptr<ARIdLayout>{};

	if (config.is_set(ARIdOptions::PROFILE))
	{
		layout = std::make_unique<ARIdTableLayout>(
			true, true, true, true, true, true, true, true);
	} else
	{
		// Use labels iff allowed and more than one property is to be printed
		const bool print_labels = !config.is_set(ARIdOptions::NOLABELS)
			and (
			1 < config.is_set(ARIdOptions::ID)
			+   config.is_set(ARIdOptions::URL)
			+   config.is_set(ARIdOptions::DBID)
			+   config.is_set(ARIdOptions::CDDBID));

		layout = std::make_unique<ARIdTableLayout>(
			print_labels,
			config.is_set(ARIdOptions::ID),
			config.is_set(ARIdOptions::URL),
			config.is_set(ARIdOptions::DBID),
			false /* no track count */,
			false /* no disc id 1 */,
			false /* no disc id 2 */,
			config.is_set(ARIdOptions::CDDBID)
		);
	}

	auto id = RichARId { arid, std::move(layout),
		config.value(ARIdOptions::URLPREFIX) };
	layout = nullptr;

	return std::make_pair(EXIT_SUCCESS,
			std::make_unique<ResultObject<RichARId>>(std::move(id)));
}


std::vector<arcstk::checksum::type> ARIdApplication::do_requested_types(
			const Configuration& /*config*/) const
{
	return {/*none*/};
}


std::string ARIdApplication::do_name() const
{
	return "id";
}


std::string ARIdApplication::do_call_syntax() const
{
	return "[OPTIONS] FILENAME";
}


std::unique_ptr<Configurator> ARIdApplication::do_create_configurator() const
{
	return std::make_unique<ARIdConfigurator>();
}

} // namespace v_1_0_0
} // namespace arcsapp

