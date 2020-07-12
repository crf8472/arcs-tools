#ifndef __ARCSTOOLS_APPARID_HPP__
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

#ifndef __LIBARCSDEC_CALCULATE_HPP__
#include <arcsdec/calculators.hpp>
#endif

#ifndef __ARCSTOOLS_APPREGISTRY_HPP__
#include "appregistry.hpp"
#endif
#ifndef __ARCSTOOLS_CONFIG_HPP__
#include "config.hpp"
#endif
#ifndef __ARCSTOOLS_PRINTERS_HPP__
#include "printers.hpp"
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
	const static std::vector<std::pair<Option, OptionCode>> local_options = {
			{{  "cddb_id",  false,
				"FALSE", "print the CDDB id" },
				ARIdOptions::CDDBID },
			{{  "db_id",   false, "FALSE",
				"print the AccurateRip DB ID (filename)" },
				ARIdOptions::DBID },
			{{  "filename",   false, "FALSE",
				"print the AccurateRip DB ID (filename)" },
				ARIdOptions::DBID }, // alias for db_id
			{{  "url",   false, "FALSE",
				"print the AccurateRip URL" },
				ARIdOptions::URL },
			{{ "profile",    false, "FALSE", "" },
				ARIdOptions::PROFILE },
			{{  "url_prefix", true, "none",
				"use the specified prefix instead of the default "
					"'http://accuraterip.com/accuraterip/'" },
				ARIdOptions::URLPREFIX },
			{{ 'a', "audiofile",   true, "none",
					"specify input audio file" },
					ARIdOptions::AUDIOFILE }
	};

	return local_options;
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


int ARIdApplication::do_run(const Options &options)
{
	// Compute requested values

	const auto metafilename  = options.argument(0);
	const auto audiofilename = options.value(ARIdOptions::AUDIOFILE);

	std::unique_ptr<ARId> arid = nullptr;

	{
		ARIdCalculator calculator;
		arid = calculator.calculate(audiofilename, metafilename);
	}

	if (!arid) { this->fatal_error("Could not compute AccurateRip id."); }

	// Adjust format and print information

	std::unique_ptr<ARIdLayout> layout;

	if (options.is_set(ARIdOptions::PROFILE))
	{
		layout = std::make_unique<ARIdTableLayout>(
			true, true, true, true, true, true, true);
	} else
	{
		layout = std::make_unique<ARIdTableLayout>(
			options.is_set(ARIdOptions::ID),
			options.is_set(ARIdOptions::URL),
			options.is_set(ARIdOptions::DBID),
			false /* no track count */,
			false /* no disc id 1 */,
			false /* no disc id 2 */,
			options.is_set(ARIdOptions::CDDBID)
		);
	}

	auto prefix = options.value(ARIdOptions::URLPREFIX);
	auto result = layout->format(std::make_tuple(arid.get(), &prefix));

	output(result, options.value(OPTION::OUTFILE));

	return EXIT_SUCCESS;
}

} // namespace arcsapp

