/**
 * \internal
 *
 * \file
 *
 * \brief Implements symbols from app-parse.hpp.
 */

#ifndef ARCSTOOLS_APPPARSE_HPP_
#include "app-parse.hpp"
#endif

#include <cstdlib>             // for EXIT_SUCCESS
#include <memory>              // for make_unique, unique_ptr
#include <string>              // for string
#include <utility>             // for move

#ifndef LIBARCSTK_DBAR_HPP_
#include <arcstk/dbar.hpp>
#endif
#ifndef LIBARCSTK_LOGGING_HPP_
#include <arcstk/logging.hpp>
#endif

#ifndef ARCSTOOLS_APPREGISTRY_HPP_
#include "appregistry.hpp"         // for RegisterApplicationType
#endif
#ifndef ARCSTOOLS_CONFIG_HPP_
#include "config.hpp"              // for DefaultConfigurator
#endif
#ifndef ARCSTOOLS_TOOLS_DBAR_HPP_
#include "tools-dbar.hpp"          // for PrintParseHandler, DBAROutputFormat
#endif

namespace arcsapp
{
inline namespace v_1_0_0
{

namespace registered
{
// Enable ApplicationFactory::lookup() to find this application by its name
// NOLINTNEXTLINE(bugprone-throwing-static-initialization)
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

	// Define 'text_decorated' as default
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
	using dbar::DBAR_DELIM;
	using delimiters = LabelStore<DBAR_DELIM>::store_t;

	const auto format_name = config.value(ARParseOptions::FORMAT);
	auto format = std::unique_ptr<DBAROutputFormat> {};

	if ("yaml" == format_name || "yml" == format_name)
	{
		format = std::make_unique<dbar::YamlFormat>();
	} else
	if ("json" == format_name)
	{
		format = std::make_unique<dbar::JsonFormat>();
	} else
	if ("text_decorated" == format_name)
	{
		// to be read by a human: text with newlines + some decoration

		format = std::make_unique<dbar::TextDecoratedFormat>(
			delimiters
			{
				{ DBAR_DELIM::UNPARSED,    "????????" },
				{ DBAR_DELIM::BLOCK_START, "---------- Block $BLOCK: " },
				{ DBAR_DELIM::BLOCK_END,   "\n" },
				{ DBAR_DELIM::HEADER_END,  "\n" },
				{ DBAR_DELIM::TRACK_START, "Track $TRACK: " },
				{ DBAR_DELIM::TRACK_DELIM, "\n" },
				{ DBAR_DELIM::PROP_DELIM1, " (" },
				{ DBAR_DELIM::PROP_DELIM2, ") " },
				{ DBAR_DELIM::DBAR_END, "========== Parsed Blocks: $BLOCKS\n" }
			}
		);
	} else
	if ("text" == format_name)
	{
		// default text format of PrintParseHandler
	} else
	if ("raw" == format_name)
	{
		// only text, no labels, single blank as delimiter

		const auto blank = std::string { " " };

		format = std::make_unique<dbar::TextDecoratedFormat>(
			delimiters
			{
				{ DBAR_DELIM::UNPARSED,    "????????" },
				{ DBAR_DELIM::DOC_END,     "\n"  },
				{ DBAR_DELIM::HEADER_END,  blank },
				{ DBAR_DELIM::BLOCK_DELIM, blank },
				{ DBAR_DELIM::TRACK_DELIM, blank },
				{ DBAR_DELIM::PROP_DELIM1, blank },
				{ DBAR_DELIM::PROP_DELIM2, blank }
			}
		);
	} else
	{
		// TODO Implement a parser for format names and throw from there
		throw cli::CallSyntaxException { "Unknown format: '" +  format_name
			+ "'" };
	};


	auto printer = PrintParseHandler { /* default format: 'text' */ };

	if (format)
	{
		printer.set_format(std::move(format));
	} // else: use default format of PrintParseHandler


	const auto arguments { config.arguments() };

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

