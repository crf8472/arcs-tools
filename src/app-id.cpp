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

#ifndef __ARCSTOOLS_APPS_HPP__
#include "apps.hpp"
#endif
#ifndef __ARCSTOOLS_CLITOKENS_HPP__
#include "clitokens.hpp"
#endif
#ifndef __ARCSTOOLS_CONFIG_HPP__
#include "config.hpp"
#endif
#ifndef __ARCSTOOLS_OPTIONS_HPP__
#include "options.hpp"
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


// ARIdOptions


ARIdOptions::ARIdOptions() = default;


// ARIdConfigurator


ARIdConfigurator::ARIdConfigurator(int argc, char** argv)
	: Configurator(argc, argv)
{
	// empty
}

const std::vector<std::pair<Option, uint32_t>>&
	ARIdConfigurator::do_supported_options() const
{
	const static std::vector<std::pair<Option, uint32_t>> local_options = {
			{{      "cddb_id",  false,
					"FALSE", "print the CDDB id" },
					ARIdOptions::CDDBID },
			{{ 		"db_id",   false, "FALSE",
					"print the AccurateRip DB ID (filename)" },
					ARIdOptions::DBID },
			{{ 		"filename",   false, "FALSE",
					"print the AccurateRip DB ID (filename)" },
					ARIdOptions::DBID }, // alias for db_id
			{{ 		"url",   false, "FALSE",
					"print the AccurateRip URL" },
					ARIdOptions::URL },
			{{ 		"profile",    false, "FALSE", "" },
					ARIdOptions::PROFILE },
			{{ 		"url_prefix", true, "none",
					"use the specified prefix"
						"instead of the default "
						"'http://accuraterip.com/accuraterip/'" },
					ARIdOptions::PRE },
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


std::unique_ptr<Configurator> ARIdApplication::create_configurator(
		int argc, char** argv) const
{
	return std::make_unique<ARIdConfigurator>(argc, argv);
}


int ARIdApplication::do_run(const Options &options)
{
	// Compute requested values

	const auto metafilename  = options.get_argument(0);
	const auto audiofilename = options.get(ARIdOptions::AUDIOFILE);

	std::unique_ptr<ARId> id = nullptr;

	{
		ARIdCalculator calculator;
		id = calculator.calculate(audiofilename, metafilename);
	}

	if (!id) { this->fatal_error("Could not compute AccurateRip id."); }

	// Adjust format and print information

	std::unique_ptr<ARIdPrinter> format;

	if (options.is_set(ARIdOptions::PROFILE))
	{
		format = std::make_unique<ARIdTableFormat>(*id,
			options.get(ARIdOptions::PRE),
			true, true, true, true, true, true);
	} else
	{
		format = std::make_unique<ARIdTableFormat>(*id,
			options.get(ARIdOptions::PRE),
			options.is_set(ARIdOptions::URL),
			options.is_set(ARIdOptions::DBID),
			false /* no track count */,
			false /* no disc id 1 */,
			false /* no disc id 2 */,
			options.is_set(ARIdOptions::CDDBID)
		);
	}

	//format->use(*id, options.get(ARIdOptions::PRE));
	output(*format);

	return EXIT_SUCCESS;
}

} // namespace arcsapp

