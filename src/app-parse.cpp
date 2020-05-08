#ifndef __ARCSTOOLS_APPARPARSE_HPP__
#include "app-parse.hpp"
#endif

#ifdef _WIN32

#include <io.h>     // for stdin
#include <fcntl.h>  // for _setmode, 0_BINARY

#endif

#include <array>
#include <cstdio>   // for std::freopen, std::ferror, std::fread, std::feof
#include <cstring>  // for std::strerror
#include <iostream>
#include <memory>
#include <sstream>
#include <streambuf> // for std::basic_streambuf
#include <string>

#ifndef __LIBARCSTK_PARSE_HPP__
#include <arcstk/parse.hpp>
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#ifndef __ARCSTOOLS_PARSEHANDLERS_HPP__
#include "parsehandlers.hpp"
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
#ifndef __ARCSTOOLS_TOOLS_FS_HPP__
#include "tools_fs.hpp"
#endif


using arcstk::ARFileParser;
using arcstk::ARStdinParser;
using arcstk::DefaultErrorHandler;


namespace {

/**
 * \brief Wrap a vector in an istream.
 */
template <typename CharT, typename TraitsT = std::char_traits<CharT>>
class istreamwrapper : public std::basic_streambuf<CharT, TraitsT>
{
public:

	/**
	 * Constructor
	 *
	 * \param[in] vec The vector to wrap
	 */
	explicit istreamwrapper(std::vector<CharT> &vec)
	{
		this->setg(vec.data(), vec.data(), vec.data() + vec.size());
	}
};

} // namespace


// ARParseOptions


ARParseOptions::ARParseOptions() = default;


// ARParseConfigurator


ARParseConfigurator::ARParseConfigurator(int argc, char** argv)
	: Configurator(argc, argv)
{
	// empty
}


std::unique_ptr<Options> ARParseConfigurator::do_configure_options(
		std::unique_ptr<Options> options)
{
	if (options->empty())
	{
		return options;
	}

	// Config 1
	//
	// Does output file exist?

	{
		std::string outfile = options->get(ARParseOptions::OUTFILE);

		if (!outfile.empty() and file::file_exists(outfile))
		{
			ARCS_LOG_WARNING << "Outfile " << outfile << " exists.";
		}
	}

	return options;
}


std::unique_ptr<Options> ARParseConfigurator::parse_options(CLIParser& cli)
{
	auto options = std::make_unique<ARParseOptions>();

	this->check_for_option_with_argument("-o", ARParseOptions::OUTFILE,
			cli, *options);

	// Input Filename(s)
	// No error if absent, but binary input will then be expected from stdin
	{
		std::string filename(cli.consume_argument());

		while (not filename.empty())
		{
			options->push_back_argument(filename);
			filename = cli.consume_argument();
		}
	}

	return options;
}


// ARParseApplication


std::unique_ptr<Configurator> ARParseApplication::create_configurator(
		int argc, char** argv) const
{
	return std::make_unique<ARParseConfigurator>(argc, argv);
}


std::string ARParseApplication::do_name() const
{
	return "parse";
}


int ARParseApplication::do_run(const Options &options)
{
	std::unique_ptr<ARParserContentPrintHandler> content_handler;

	if (options.is_set(ARParseOptions::OUTFILE))
	{
		std::string outfilename = options.get(ARParseOptions::OUTFILE);
		content_handler =
			std::make_unique<ARParserContentPrintHandler>(outfilename);
	} else
	{
		content_handler = std::make_unique<ARParserContentPrintHandler>();
	}

	auto arguments = options.get_arguments();

	if (not arguments.empty()) // read from file(s)
	{
		ARFileParser parser;
		parser.set_content_handler(std::move(content_handler));
		parser.set_error_handler(std::make_unique<DefaultErrorHandler>());

		for (const auto& file : arguments)
		{
			parser.set_file(file);
			parser.parse();
		}
	}
	else // read from stdin
	{
		ARStdinParser parser;
		parser.set_content_handler(std::move(content_handler));
		parser.set_error_handler(std::make_unique<DefaultErrorHandler>());

		parser.parse();
	}

	return EXIT_SUCCESS;
}


void ARParseApplication::do_print_usage() const
{
	std::cout << this->name() << " [OPTIONS] <filename>\n";
	std::cout << "Where <filename> is the name of some dBAR-*.bin file ";
	std::cout << "downloaded from AccurateRip.\n\n";

	std::cout << "Options:\n\n";

	std::cout << "-q             only output parsed content, nothing else\n\n";
	std::cout << "-v <i>         verbous output (loglevel 0-6)\n\n";
	std::cout << "-o <filename>  specify output file\n\n";
}

