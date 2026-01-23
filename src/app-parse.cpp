/**
 * \internal
 *
 * \file
 *
 * \brief Implements symbols from app-parse.hpp.
 */

#ifndef __ARCSTOOLS_APPPARSE_HPP__
#include "app-parse.hpp"
#endif

#include <cstdlib>             // for EXIT_SUCCESS
#include <memory>              // for make_unique, unique_ptr
#include <string>              // for string

#ifndef __LIBARCSTK_DBAR_HPP__
#include <arcstk/dbar.hpp>
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#ifndef __ARCSTOOLS_APPREGISTRY_HPP__
#include "appregistry.hpp"         // for RegisterApplicationType
#endif
#ifndef __ARCSTOOLS_CONFIG_HPP__
#include "config.hpp"              // for DefaultConfigurator
#endif
#ifndef __ARCSTOOLS_TOOLS_ARID_HPP__
#include "tools-arid.hpp"          // for ARIdLayout
#endif
#ifndef __ARCSTOOLS_TOOLS_DBAR_HPP__
#include "tools-dbar.hpp"          // for PrintParseHandler, DBARTripletLayout
#endif

namespace arcsapp
{
inline namespace v_1_0_0
{

namespace registered
{
// Enable ApplicationFactory::lookup() to find this application by its name
const auto parse = RegisterApplicationType<ARParseApplication>("parse");
}

// arcsapp
using dbar::DBAROutputFormat;
using dbar::PrintParseHandler;
using input::read_from_stdin;


// ARParseConfigurator


void ARParseConfigurator::do_flush_local_options(OptionRegistry& r) const
{
	using cli::OP_VALUE;

	using std::cend;
	r.insert(cend(r),
	{
		{ ARParseOptions::FORMAT,
		{ "format", true, OP_VALUE::USE_DEFAULT, "Specify output format" }},
	});
}


std::unique_ptr<Options> ARParseConfigurator::do_configure_options(
			std::unique_ptr<Options> options) const
{
	using cli::OP_VALUE;

	if (!options->is_set(ARParseOptions::FORMAT)
			|| options->value(ARParseOptions::FORMAT) == OP_VALUE::USE_DEFAULT)
	{
		options->set(ARParseOptions::FORMAT, "text_decorated");
	}

	return options;
}


// ARParseApplication


std::string ARParseApplication::do_name() const
{
	return "parse";
}


std::string ARParseApplication::do_call_syntax() const
{
	return "[OPTIONS] <filename>";
}


std::unique_ptr<Configurator> ARParseApplication::do_create_configurator() const
{
	return std::make_unique<ARParseConfigurator>();
}


int ARParseApplication::do_run(const Configuration& config)
{
	auto printer = PrintParseHandler {};

	auto format  = std::unique_ptr<DBAROutputFormat> {};

	using dbar::DBAR_LABEL;
	using dbar::DBAR_TRIPLET_LABEL;

	if ("yaml" == config.value(ARParseOptions::FORMAT))
	{
		using dbar::YamlFormat;
		format = std::make_unique<YamlFormat>();
	} else
	if ("json" == config.value(ARParseOptions::FORMAT))
	{
		using dbar::JsonFormat;
		format = std::make_unique<JsonFormat>();
	} else
	if ("text" == config.value(ARParseOptions::FORMAT))
	{
		// only text, no labels, no delimiters except newlines

		using dbar::TextDecoratedFormat;
		using dbar::TextDecoratedTripletLayout;
		using details::flag_operand;

		format = std::make_unique<TextDecoratedFormat>(
				LabelStore<DBAR_LABEL>::store_t
				{
					{ DBAR_LABEL::DELIM2, "\n" },
				},
				Flags::ALL_FALSE | flag_operand(DBAR_LABEL::DELIM2, true),
				nullptr, /* no ARIdLayout required */
				std::make_unique<TextDecoratedTripletLayout>(
					LabelStore<DBAR_TRIPLET_LABEL>::store_t
					{
						{ DBAR_TRIPLET_LABEL::DELIM4, "\n" }
					},
					Flags::ALL_FALSE
						| flag_operand(DBAR_TRIPLET_LABEL::DELIM4, true)
				)
		);
	} else
	if ("raw" == config.value(ARParseOptions::FORMAT))
	{
		// only text, no labels, no delimiters

		using dbar::TextDecoratedFormat;
		using dbar::TextDecoratedTripletLayout;

		format = std::make_unique<TextDecoratedFormat>(
				LabelStore<DBAR_LABEL>::store_t { /* none */ },
				Flags::ALL_FALSE,
				nullptr, /* no ARIdLayout required */
				std::make_unique<TextDecoratedTripletLayout>(
					LabelStore<DBAR_TRIPLET_LABEL>::store_t { /* none */ },
					Flags::ALL_FALSE
				)
		);
	} else
	{
		// TODO CallSyntaxException
	};

	printer.set_format(std::move(format));

	const auto arguments = config.arguments();

	// read from file(s)
	if (arguments && !arguments->empty())
	{
		for (const auto& file : *arguments)
		{
			arcstk::parse_file(file, &printer, nullptr);
		}
	}
	else // read from stdin
	{
		// TODO Actual amount of bytes should come from config
		read_from_stdin(1024, &printer, nullptr);
	}

	return EXIT_SUCCESS;
}

} // namespace v_1_0_0
} // namespace arcsapp

