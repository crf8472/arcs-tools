#ifndef __ARCSTOOLS_APPARVERIFY_HPP__
#include "app-verify.hpp"
#endif

#include <algorithm>       // for replace, max, transform
#include <cctype>          // for toupper
#include <cstddef>         // for size_t
#include <cstdint>         // for uint32_t
#include <cstdlib>         // for EXIT_SUCCESS
#include <exception>       // for exception
#include <iomanip>         // for setw
#include <iterator>        // for begin, end
#include <memory>          // for unique_ptr, make_unique
#include <stdexcept>       // for invalid_argument, runtime_error
#include <sstream>         // for istringstream, ostringstream
#include <string>          // for stoul, string, to_string
#include <tuple>           // for get, make_tuple, tuple
#include <type_traits>     // for add_const<>::type, underlying_type_t
#include <utility>         // for move, pair

#ifndef __LIBARCSTK_MATCH_HPP__
#include <arcstk/match.hpp>
#endif
#ifndef __LIBARCSTK_PARSE_HPP__
#include <arcstk/parse.hpp>
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>       // for ARCS_LOG_DEBUG, ARCS_LOG_ERROR
#endif

#ifndef __ARCSTOOLS_APPREGISTRY_HPP__
#include "appregistry.hpp"          // for RegisterApplicationType
#endif
#ifndef __ARCSTOOLS_CONFIG_HPP__
#include "config.hpp"               // for Configurator
#endif
#ifndef __ARCSTOOLS_RESULT_HPP__
#include "result.hpp"               // for ResultObject, Result
#endif
#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#include "layouts.hpp"              // for ARIdLayout
#endif
#ifndef __ARCSTOOLS_TABLE_HPP__
#include "table.hpp"                // for StringTableLayout, BoolDecorator
#endif
#ifndef __ARCSTOOLS_ANSI_HPP__
#include "ansi.hpp"                 // for Colorize
#endif
#ifndef __ARCSTOOLS_TOOLS_PARSE_HPP__
#include "tools-parse.hpp"          // for ContentHandler
#endif
#ifndef __ARCSTOOLS_TOOLS_CALC_HPP__
#include "tools-calc.hpp"           // for audiofile_layout
#endif
#ifndef __ARCSTOOLS_TOOLS_FS_HPP__
#include "tools-fs.hpp"             // for file_exists
#endif
#ifndef __ARCSTOOLS_TOOLS_INFO_HPP__
#include "tools-info.hpp"           // for SupportedFormats
#endif

namespace arcsapp
{

namespace registered
{
const auto verify = RegisterApplicationType<ARVerifyApplication>("verify");
}

using arcstk::ARStreamParser;
using arcstk::ARResponse;
using arcstk::Checksum;
using arcstk::Checksums;
using arcstk::Logging;
using arcstk::DefaultContentHandler;
using arcstk::DefaultErrorHandler;
using arcstk::Matcher;
using arcstk::AlbumMatcher;
using arcstk::ListMatcher;
using arcstk::TracksetMatcher;


// VERIFY


constexpr OptionCode VERIFY::NOFIRST;
constexpr OptionCode VERIFY::NOLAST;
constexpr OptionCode VERIFY::NOALBUM;
constexpr OptionCode VERIFY::RESPONSEFILE;
constexpr OptionCode VERIFY::REFVALUES;
constexpr OptionCode VERIFY::PRINTALL;
constexpr OptionCode VERIFY::BOOLEAN;
constexpr OptionCode VERIFY::NOOUTPUT;


// ARVerifyConfigurator


void ARVerifyConfigurator::do_flush_local_options(OptionRegistry &r) const
{
	using std::end;
	r.insert(end(r),
	{
		// from FORMATBASE

		{ VERIFY::READERID ,
		{  "reader", true, OP_VALUE::AUTO,
			"Force use of audio reader with specified id" }},

		{ VERIFY::PARSERID ,
		{  "parser", true, OP_VALUE::AUTO,
			"Force use of toc parser with specified id" }},

		{ VERIFY::LIST_TOC_FORMATS ,
		{  "list-toc-formats", false, OP_VALUE::FALSE,
			"List all supported file formats for TOC metadata" }},

		{ VERIFY::LIST_AUDIO_FORMATS ,
		{  "list-audio-formats", false, OP_VALUE::FALSE,
			"List all supported audio codec/container formats" }},

		// from CALCBASE

		{ VERIFY::METAFILE ,
		{  'm', "metafile", true, OP_VALUE::NONE,
			"Specify metadata file (TOC) to use" }},

		{ VERIFY::NOTRACKS ,
		{  "no-track-nos", false, OP_VALUE::FALSE,
			"Do not print track numbers" }},

		{ VERIFY::NOFILENAMES ,
		{  "no-filenames", false, OP_VALUE::FALSE,
			"Do not print the filenames" }},

		{ VERIFY::NOOFFSETS ,
		{  "no-offsets", false, OP_VALUE::FALSE,
			"Do not print track offsets" }},

		{ VERIFY::NOLENGTHS ,
		{  "no-lengths", false, OP_VALUE::FALSE,
			"Do not print track lengths" }},

		{ VERIFY::NOLABELS ,
		{  "no-labels", false, OP_VALUE::FALSE,
			"Do not print column or row labels" }},

		{ VERIFY::COLDELIM ,
		{  "col-delim", true, "ASCII-32", "Specify column delimiter" }},

		{ VERIFY::PRINTID ,
		{  "print-id", false, OP_VALUE::FALSE,
			"Print the AccurateRip Id of the album" }},

		{ VERIFY::PRINTURL ,
		{  "print-url", false, OP_VALUE::FALSE,
			"Print the AccurateRip URL of the album" }},

		// from VERIFY

		{ VERIFY::NOFIRST ,
		{  "no-first", false, OP_VALUE::FALSE,
			"Do not treat first audio file as first track" }},

		{ VERIFY::NOLAST ,
		{  "no-last", false, OP_VALUE::FALSE,
			"Do not treat last audio file as last track" }},

		{ VERIFY::NOALBUM ,
		{  "no-album", false, OP_VALUE::FALSE,
			"Abbreviates \"--no-first --no-last\"" }},

		{ VERIFY::RESPONSEFILE ,
		{  'r', "response", true, OP_VALUE::NONE,
			"Specify AccurateRip response file" }},

		{ VERIFY::REFVALUES ,
		{  "refvalues", true, OP_VALUE::NONE,
			"Specify AccurateRip reference values (as hex value list)" }},

		{ VERIFY::PRINTALL ,
		{  "print-all-matches", false, OP_VALUE::FALSE,
			"Print verification results for all blocks" }},

		{ VERIFY::BOOLEAN ,
		{  'b', "boolean", false, OP_VALUE::FALSE,
			"Return number of differing tracks in best match" }},

		{ VERIFY::NOOUTPUT ,
		{  'n', "no-output", false, OP_VALUE::FALSE,
			"Do not print the result (implies --boolean)" }},

		{ VERIFY::COLORED ,
		{  "colors", true, OP_VALUE::USE_DEFAULT,
			"Use colored output and optionally specify colors" }},

		{ VERIFY::CONFIDENCE ,
		{  "confidence", false, OP_VALUE::FALSE,
			"Print confidence values" }}
	});
}


std::unique_ptr<Options> ARVerifyConfigurator::do_configure_options(
		std::unique_ptr<Options> options) const
{
	auto voptions = configure_calcbase_options(std::move(options));

	auto no_album_options = std::string{}; // for log messages

	// Album mode

	if (voptions->is_set(VERIFY::NOALBUM))
	{
		ARCS_LOG(DEBUG1) << "Activate option NOFIRST due to NOALBUM";
		voptions->set(VERIFY::NOFIRST);

		ARCS_LOG(DEBUG1) << "Activate option NOLAST due to NOALBUM";
		voptions->set(VERIFY::NOLAST);

		no_album_options = "--no-album";
	} else
	{
		if (voptions->is_set(VERIFY::NOFIRST))
		{
			no_album_options += "--no-first";
		}

		if (voptions->is_set(VERIFY::NOLAST))
		{
			if (no_album_options.empty()) { no_album_options += ","; }
			no_album_options += "--no-last";
		}

		if(voptions->is_set(VERIFY::NOFIRST) and
				voptions->is_set(VERIFY::NOLAST))
		{
			ARCS_LOG(DEBUG1) <<
				"Activate option NOALBUM due to NOFIRST and NOLAST";
			voptions->set(VERIFY::NOALBUM);
		}
	}

	if (voptions->is_set(VERIFY::NOFIRST) or voptions->is_set(VERIFY::NOLAST))
	{
		if (voptions->is_set(VERIFY::METAFILE))
		{
			ARCS_LOG(WARNING) << "Metadata file "
				<< voptions->value(VERIFY::METAFILE) << " specifies an album,"
				" but adding " << no_album_options
				<< " will probably lead to unwanted results";
		}
	}

	// Only print those things from the reference data that we actually may have

	if (voptions->is_set(VERIFY::REFVALUES))
	{
		if (voptions->is_set(VERIFY::PRINTID))
		{
			ARCS_LOG_WARNING <<
				"Ignore option PRINTID since option REFVALUES is active "
				"and reference values do not provide an ID to print";
			voptions->unset(VERIFY::PRINTID);
		}
		if (voptions->is_set(VERIFY::PRINTURL))
		{
			ARCS_LOG_WARNING <<
				"Ignore option PRINTURL since option REFVALUES is active "
				"and reference values do not provide an URL to print";
			voptions->unset(VERIFY::PRINTURL);
		}
		if (voptions->is_set(VERIFY::CONFIDENCE))
		{
			ARCS_LOG_WARNING <<
				"Ignore option CONFIDENCE since option REFVALUES is active and "
				"reference values do not provide confidence values to print";
			voptions->unset(VERIFY::CONFIDENCE);
		}
	}

	// NOOUTPUT implies BOOLEAN

	if (voptions->is_set(VERIFY::NOOUTPUT))
	{
		voptions->set(VERIFY::BOOLEAN);
	}

	return voptions;
}


void ARVerifyConfigurator::do_validate(const Options& options) const
{
	if (options.is_set(VERIFY::RESPONSEFILE)
			and options.is_set(VERIFY::REFVALUES))
	{
		throw ConfigurationException("Cannot process --refvalues along with "
				" -r/--response, only one of these options is allowed");
	}

	if (!options.is_set(VERIFY::RESPONSEFILE)
			and !options.is_set(VERIFY::REFVALUES))
	{
		throw ConfigurationException("No reference values specified."
				" One of --refvalues and -r/--response is required");
	}
}


OptionParsers ARVerifyConfigurator::do_parser_list() const
{
	return {
		{ VERIFY::RESPONSEFILE,
			[]{ return std::make_unique<ARResponseParser>();   } },
		{ VERIFY::REFVALUES,
			[]{ return std::make_unique<ChecksumListParser>(); } },
		{ VERIFY::COLORED,
			[]{ return std::make_unique<ColorSpecParser>();    } }
	};
}


void ARVerifyConfigurator::do_validate(const Configuration& c) const
{
	// No reference checksums at all? => Error

	if (c.object<ARResponse>(VERIFY::RESPONSEFILE).size() == 0
			&& c.object<std::vector<Checksum>>(VERIFY::REFVALUES).empty())
	{
		throw std::runtime_error(
				"No reference checksums for matching available.");
	}
}


// MatchDecorator


MatchDecorator::MatchDecorator(const std::size_t n, const Highlight match_hl,
		const Color fg_match, const Color bg_match,
		const Highlight mismatch_hl, const Color fg_mismatch,
		const Color bg_mismatch)
	: CellDecorator(n)
	, highlights_ { match_hl, mismatch_hl }
	, colors_     { fg_match, bg_match, fg_mismatch, bg_mismatch }
{
	/* empty */
}


MatchDecorator::MatchDecorator(const std::size_t n, const Highlight match_hl,
		const std::pair<Color, Color>& match, const Highlight mismatch_hl,
		const std::pair<Color, Color>& mismatch)
	: MatchDecorator(n, match_hl, match.first, match.second,
			mismatch_hl, mismatch.first, mismatch.second)
{
	/* empty */
}


ansi::Highlight MatchDecorator::hl(const DecorationType& d) const
{
	if (DecorationType::MATCH == d)
	{
		return highlights_[0];
	}

	if (DecorationType::MISMATCH == d)
	{
		return highlights_[1];
	}

	return Highlight::NORMAL;
}


std::pair<ansi::Color, ansi::Color> MatchDecorator::colors(
		const DecorationType& d) const
{
	if (DecorationType::MATCH == d)
	{
		return { colors_[0], colors_[1] };
	}

	if (DecorationType::MISMATCH == d)
	{
		return { colors_[2], colors_[3] };
	}

	return { Color::FG_DEFAULT, Color::BG_DEFAULT };
}


ansi::Color MatchDecorator::fg(const DecorationType& d) const
{
	return colors(d).first;
}


ansi::Color MatchDecorator::bg(const DecorationType& d) const
{
	return colors(d).second;
}


std::string MatchDecorator::do_decorate_set(std::string&& s) const
{
	return  colored(hl(DecorationType::MATCH),
				fg(DecorationType::MATCH), bg(DecorationType::MATCH), s);
}


std::string MatchDecorator::do_decorate_unset(std::string&& s) const
{
	return  colored(hl(DecorationType::MISMATCH),
				fg(DecorationType::MISMATCH), bg(DecorationType::MISMATCH), s);
}


std::unique_ptr<CellDecorator> MatchDecorator::do_clone() const
{
	return std::make_unique<MatchDecorator>(*this);
}


// VerifyResultFormatter


VerifyResultFormatter::VerifyResultFormatter()
	: match_symbol_ {}
{
	// empty
}


void VerifyResultFormatter::set_match_symbol(const std::string &match_symbol)
{
	match_symbol_ = match_symbol;
}


const std::string& VerifyResultFormatter::match_symbol() const
{
	return match_symbol_;
}


void VerifyResultFormatter::assertions(const InputTuple t) const
{
	const auto checksums = std::get<3>(t);
	const auto arid      = std::get<4>(t);
	const auto toc       = std::get<5>(t);
	const auto filenames = std::get<8>(t);

	validate(checksums, toc, arid, filenames);

	// Specific for verify

	const auto response  = std::get<6>(t);
	const auto refvalues = std::get<7>(t);
	const auto match     = std::get<1>(t);
	const auto block     = std::get<2>(t);

	if (refvalues.empty() && !response.size())
	{
		throw std::invalid_argument("Missing reference checksums, "
				"nothing to print.");
	}

	if (!refvalues.empty() && refvalues.size() != checksums.size())
	{
		throw std::invalid_argument("Mismatch: "
				"Reference for " + std::to_string(refvalues.size())
				+ " tracks, but Checksums specify "
				+ std::to_string(checksums.size()) + " tracks.");
	}

	if (!match)
	{
		throw std::invalid_argument("Missing match information, "
				"nothing to print.");
	}

	if (block > match->total_blocks())
	{
		throw std::invalid_argument("Mismatch: "
				"Match contains no block " + std::to_string(block)
				+ " but contains only "
				+ std::to_string(match->total_blocks()) + " blocks.");
	}
}


std::unique_ptr<Result> VerifyResultFormatter::do_format(InputTuple t) const
{
	const auto types_to_print = std::get<0>(t);
	const auto match      = std::get<1>(t);
	const auto block      = std::get<2>(t);
	const auto checksums  = std::get<3>(t);
	const auto arid       = std::get<4>(t);
	const auto toc        = std::get<5>(t);
	const auto response   = std::get<6>(t);
	const auto refvalues  = std::get<7>(t);
	const auto filenames  = std::get<8>(t);
	const auto alt_prefix = std::get<9>(t);

	auto result = std::make_unique<ResultList>();

	// If an ARResponse is used for the references with a block (not PRINTALL)
	if (response.size() > 0 && block > -1)
	{
		// Use ARId of specified block for "Theirs" ARId
		result->append(std::make_unique<ResultObject<RichARId>>(
				build_id(toc, response.at(block).id(), alt_prefix)));
	}

	result->append(build_result(
				types_to_print,
				match,
				block,
				checksums,
				arid,
				toc,
				response,
				refvalues,
				filenames,
				alt_prefix));

	return result;
}


std::vector<ATTR> VerifyResultFormatter::do_create_attributes(
		const print_flag_t print_flags,
		const std::vector<arcstk::checksum::type>& types_to_print,
		const int total_theirs_per_block) const
{
	const auto total_fields =
		  print_flags(ATTR::TRACK)
		+ print_flags(ATTR::OFFSET)
		+ print_flags(ATTR::LENGTH)
		+ print_flags(ATTR::FILENAME)
		+ types_to_print.size()
		+ total_theirs_per_block
		+ print_flags(ATTR::CONFIDENCE) * total_theirs_per_block;

	using checksum = arcstk::checksum::type;

	std::vector<ATTR> fields;
	fields.reserve(total_fields);
	if (print_flags(ATTR::TRACK))    { fields.emplace_back(ATTR::TRACK);    }
	if (print_flags(ATTR::FILENAME)) { fields.emplace_back(ATTR::FILENAME); }
	if (print_flags(ATTR::OFFSET))   { fields.emplace_back(ATTR::OFFSET);   }
	if (print_flags(ATTR::LENGTH))   { fields.emplace_back(ATTR::LENGTH);   }

	for (const auto& t : types_to_print)
	{
		if (checksum::ARCS1 == t)
		{
			fields.emplace_back(ATTR::CHECKSUM_ARCS1);
		} else
		{
			if (checksum::ARCS2 == t)
			{
				fields.emplace_back(ATTR::CHECKSUM_ARCS2);
			}
		}

		for (auto i = int { 0 }; i < total_theirs_per_block; ++i)
		{
			fields.emplace_back(ATTR::THEIRS);

			if (print_flags(ATTR::CONFIDENCE))
			{
				fields.emplace_back(ATTR::CONFIDENCE);
			}
		}
	}

	return fields;
}


// MonochromeVerifyResultFormatter


void MonochromeVerifyResultFormatter::do_their_match(const Checksum& checksum,
		const int record_idx, const int field_idx, TableComposer* c) const
{
	// XXX Why a fixed symbol? Should be configurable by decoration
	c->set_field(record_idx, field_idx, match_symbol());
}


void MonochromeVerifyResultFormatter::do_their_mismatch(
		const Checksum& checksum, const int record_idx,
		const int field_idx, TableComposer* c) const
{
	c->set_field(record_idx, field_idx, this->checksum(checksum));
}


// get_decorationType()


DecorationType get_decorationtype(const std::string& name)
{
	using map_type = std::unordered_map<std::string, DecorationType>;

	static map_type types = [](){
		map_type t;
		t["MATCH"]    = DecorationType::MATCH;
		t["MISMATCH"] = DecorationType::MISMATCH;
		t["MINE"]     = DecorationType::MINE;
		return t;
	}();

	return types[name];
}


std::string name(const DecorationType type)
{
	using map_type = std::unordered_map<DecorationType, std::string>;

	static map_type names = [](){
		map_type t;
		t[DecorationType::MATCH]    = "MATCH";
		t[DecorationType::MISMATCH] = "MISMATCH";
		t[DecorationType::MINE]     = "MINE";
		return t;
	}();

	return names[type];
}


// ColorRegistry


ColorRegistry::ColorRegistry()
	: colors_ {
		{ DecorationType::MATCH,    { ansi::Color::FG_GREEN,
									  ansi::Color::BG_DEFAULT} },
		{ DecorationType::MISMATCH, { ansi::Color::FG_RED,
									  ansi::Color::BG_DEFAULT} },
		{ DecorationType::MINE,     { ansi::Color::FG_DEFAULT,
									  ansi::Color::BG_DEFAULT} }
	}
{
	// do nothing
}


bool ColorRegistry::has(DecorationType d) const
{
	using std::end;
	return colors_.find(d) != end(colors_);
}


std::pair<ansi::Color,ansi::Color> ColorRegistry::get(DecorationType d) const
{
	using std::end;
	if (const auto c = colors_.find(d); c != end(colors_))
	{
		return c->second;
	}

	return { ansi::Color::FG_DEFAULT, ansi::Color::BG_DEFAULT };
}


ansi::Color ColorRegistry::get_fg(DecorationType d) const
{
	return get(d).first;
}


ansi::Color ColorRegistry::get_bg(DecorationType d) const
{
	return get(d).second;
}


void ColorRegistry::set_fg(DecorationType d, ansi::Color c)
{
	using std::end;
	if (const auto p = colors_.find(d); p != end(colors_))
	{
		p->second.first = c;
	} else
	{
		colors_.insert({ d, { c, ansi::Color::BG_DEFAULT }});
	}
}


void ColorRegistry::set_bg(DecorationType d, ansi::Color c)
{
	using std::end;
	if (const auto p = colors_.find(d); p != end(colors_))
	{
		p->second.second = c;
	} else
	{
		colors_.insert({ d, { ansi::Color::FG_DEFAULT, c }});
	}
}


void ColorRegistry::set(DecorationType d, ansi::Color fg, ansi::Color bg)
{
	if (const auto p = colors_.find(d); p != end(colors_))
	{
		p->second.first  = fg;
		p->second.second = bg;
	} else
	{
		colors_.insert({ d, { fg, bg } });
	}
}


void ColorRegistry::clear()
{
	colors_.clear();
}


// ColorizingVerifyResultFormatter


ColorizingVerifyResultFormatter::ColorizingVerifyResultFormatter()
	: ColorizingVerifyResultFormatter(ColorRegistry{})
{
	// empty
}


ColorizingVerifyResultFormatter::
	ColorizingVerifyResultFormatter(const ColorRegistry& colors)
	: colors_ { colors }
{
	// empty
}


void ColorizingVerifyResultFormatter::init_composer(TableComposer* c) const
{
	using ansi::Highlight;

	const auto r_size { c->total_records() };

	// Register Decorators to each "Theirs" field

	int i = 0;
	for (const auto& field : c->fields())
	{
		if (ATTR::THEIRS == field)
		{
			ARCS_LOG(DEBUG1) << "Register MatchDecorator to field index " << i;

			c->register_to_field(i,
				std::make_unique<MatchDecorator>(r_size,
					Highlight::BOLD, colors_.get(DecorationType::MATCH),
					Highlight::BOLD, colors_.get(DecorationType::MISMATCH) ));
		}

		++i;
	}
}


void ColorizingVerifyResultFormatter::do_their_match(const Checksum& checksum,
		const int record_idx, const int field_idx, TableComposer* c) const
{
	c->set_field(record_idx, field_idx, this->checksum(checksum));

	ARCS_LOG(DEBUG1) << "Mark cell " << record_idx << ", " << field_idx
		<< " as match-decorated";

	c->mark(record_idx, field_idx);
}


void ColorizingVerifyResultFormatter::do_their_mismatch(
		const Checksum& checksum, const int record_idx, const int field_idx,
		TableComposer* c) const
{
	c->set_field(record_idx, field_idx, this->checksum(checksum));

	ARCS_LOG(DEBUG1) << "Mark cell " << record_idx << ", " << field_idx
		<< " as mismatch-decorated";
}


std::pair<ansi::Color, ansi::Color> ColorizingVerifyResultFormatter::colors(
		DecorationType d) const
{
	return colors_.get(d);
}


ansi::Color ColorizingVerifyResultFormatter::color_fg(DecorationType d) const
{
	return colors_.has(d) ? colors_.get_fg(d) : ansi::Color::FG_DEFAULT;
}


ansi::Color ColorizingVerifyResultFormatter::color_bg(DecorationType d) const
{
	return colors_.has(d) ? colors_.get_fg(d) : ansi::Color::FG_DEFAULT;
}


void ColorizingVerifyResultFormatter::set_color_fg(DecorationType d, Color c)
{
	colors_.set_fg(d, c);
}


void ColorizingVerifyResultFormatter::set_color_bg(DecorationType d, Color c)
{
	colors_.set_bg(d, c);
}


// ARResponseParser


ARResponse ARResponseParser::load_response(const std::string& responsefile)
	const
{
	// Parse the AccurateRip response

	std::unique_ptr<ARStreamParser> parser;

	if (responsefile.empty())
	{
		ARCS_LOG(DEBUG1) << "Expect input from stdin";
		parser = std::make_unique<ARStdinParser>();

	} else
	{
		parser = std::make_unique<ARFileParser>(responsefile);
	}

	ARResponse response;
	auto c_handler { std::make_unique<DefaultContentHandler>() };
	c_handler->set_object(response);
	parser->set_content_handler(std::move(c_handler));
	parser->set_error_handler(std::make_unique<DefaultErrorHandler>());

	try
	{
		if (!parser->parse())
		{
			throw CallSyntaxException("No bytes parsed, exit");
		}
	} catch (const std::exception& e)
	{
		throw CallSyntaxException(e.what());
	}

	return response;
}


std::string ARResponseParser::start_message() const
{
	return "AccurateRip reference checksums (=\"Theirs\")";
}


ARResponse ARResponseParser::do_parse_empty() const
{
	return this->load_response("");
}


ARResponse ARResponseParser::do_parse_nonempty(const std::string& s) const
{
	return this->load_response(s);
}


// ChecksumListParser


std::string ChecksumListParser::start_message() const
{
	return "List of local reference checksums (=\"Theirs\")";
}


std::vector<Checksum> ChecksumListParser::do_parse_nonempty(
		const std::string& checksum_list) const
{
	auto i = int { 0 };
	auto refvals = parse_list_to_objects<Checksum>(
				checksum_list,
				',',
				[&i](const std::string& s) -> Checksum
				{
					Checksum::value_type value = std::stoul(s, 0, 16);
					ARCS_LOG(DEBUG1) << "Parse checksum: " << Checksum { value }
						<< " (Track " << ++i << ")";
					return Checksum { value };
				});

	ARCS_LOG(DEBUG1) << "Parsed " << refvals.size() << " checksums";
	return refvals;
}


// ColorSpecParser


std::string ColorSpecParser::start_message() const
{
	return "List of output color requests";
}


ColorRegistry ColorSpecParser::do_parse_nonempty(const std::string& input) const
{
	if (input == OP_VALUE::USE_DEFAULT)
	{
		return ColorRegistry{ /* default colors */ };
	}

	const std::string sep = ":"; // name-value separator

	ColorRegistry r;
	r.clear(); // remove defaults, use only values from input string

	parse_list(input, ',',
			[&r,&sep](const std::string& s) // parse a single TYPE:COLOR pair
			{
				const auto pos = s.find(sep);

				if (pos == std::string::npos)
				{
					std::ostringstream msg;
					msg << "Could not parse --colors input: '"
						<< s << "'. Expected a "
						"comma-separated sequence of pairs like "
						"'type1:color1,type2:color2,...'";
					throw CallSyntaxException(msg.str());
				}

				const auto uppercase = [](std::string str) -> std::string
				{
					using std::begin;
					using std::end;
					std::transform(begin(str), end(str), begin(str),
						[](unsigned char c) { return std::toupper(c); });
					return str;
				};

				const auto type   { uppercase(s.substr(0, pos)) };
				const auto colors { uppercase(s.substr(pos + sep.length())) };

				using ansi::get_color;

				const auto plus = colors.find("+");
				if (plus != std::string::npos)
				{
					// Color pair

					const auto color_fg { colors.substr(0, plus)  };
					const auto color_bg { colors.substr(plus + 1) };

					ARCS_LOG(DEBUG1) << "For " << type << " set "
							<< color_fg << " as foreground color";
					ARCS_LOG(DEBUG1) << "For " << type << " set "
							<< color_bg << " as background color";

					r.set(get_decorationtype(type),
						get_color(color_fg), get_color(color_bg));
				} else
				{
					// Single color

					if ("BG_" == colors.substr(0,3))
					{
						ARCS_LOG(DEBUG1) << "For " << type << " set " << colors
							<< " as background color";
						r.set_bg(get_decorationtype(type), get_color(colors));
					} else
					{
						ARCS_LOG(DEBUG1) << "For " << type << " set " << colors
							<< " as foreground color";
						r.set_fg(get_decorationtype(type), get_color(colors));
					}
				}

			});
	return r;
}


// ARVerifyApplication


std::unique_ptr<VerifyResultFormatter> ARVerifyApplication::create_formatter(
		const Configuration& config) const
{
	auto fmt = std::unique_ptr<VerifyResultFormatter>();

	if (config.is_set(VERIFY::COLORED))
	{
		fmt = std::make_unique<ColorizingVerifyResultFormatter>(
				config.object<ColorRegistry>(VERIFY::COLORED));
	} else
	{
		fmt = std::make_unique<MonochromeVerifyResultFormatter>();
	}

	// Layouts for Checksums + ARId

	fmt->set_checksum_layout(std::make_unique<HexLayout>());

	// Layout for ARId

	if (config.is_set(VERIFY::PRINTID) || config.is_set(VERIFY::PRINTURL))
	{
		std::unique_ptr<ARIdLayout> id_layout =
			std::make_unique<ARIdTableLayout>(
				!config.is_set(VERIFY::NOLABELS),
				config.is_set(VERIFY::PRINTID),
				config.is_set(VERIFY::PRINTURL),
				false, /* no filenames */
				false, /* no tracks */
				false, /* no id 1 */
				false, /* no id 2 */
				false  /* no cddb id */
		);

		fmt->set_arid_layout(std::move(id_layout));
	}

	// Print labels or not
	fmt->format_label(!config.is_set(VERIFY::NOLABELS));

	// TOC present? Helper for determining other properties
	const bool has_toc = !config.value(VERIFY::METAFILE).empty();

	// Print track numbers if they are not forbidden and a TOC is present
	fmt->format_data(ATTR::TRACK,
			config.is_set(VERIFY::NOTRACKS) ? false : has_toc);

	// Print offsets if they are not forbidden and a TOC is present
	fmt->format_data(ATTR::OFFSET,
			config.is_set(VERIFY::NOOFFSETS) ? false : has_toc);

	// Print lengths if they are not forbidden
	fmt->format_data(ATTR::LENGTH, !config.is_set(VERIFY::NOLENGTHS));

	// Print filenames if they are not forbidden and a TOC is _not_ present
	fmt->format_data(ATTR::FILENAME,
			!config.is_set(VERIFY::NOFILENAMES) || !has_toc);

	// Indicate a matching checksum by this symbol
	fmt->set_match_symbol("==");

	// Indicate that confidence values should be printed (if available)
	fmt->format_data(ATTR::CONFIDENCE, config.is_set(VERIFY::CONFIDENCE));

	// Method for creating the result table
	fmt->set_builder(std::make_unique<RowTableComposerBuilder>());

	auto layout { std::make_unique<StringTableLayout>() };

	// Set inner column delimiter
	layout->set_col_inner_delim(config.is_set(VERIFY::COLDELIM)
		? config.value(VERIFY::COLDELIM)
		: " ");

	fmt->set_table_layout(std::move(layout));

	return fmt;
}


void ARVerifyApplication::log_matching_files(const Checksums &checksums,
		const Match &match, const uint32_t block,
		const bool version) const
{
	auto unmatched { checksums.size() };

	// Traverse checksums
	for (std::size_t t = 0; t < checksums.size() and unmatched > 0; ++t)
	{
		// Traverse specified block
		for (int track = 0; track < match.tracks_per_block(); ++track)
		{
			if (match.track(block, track, version))
			{
				ARCS_LOG_DEBUG << "Pos " << std::to_string(track)
					<< " matches track " << std::to_string(track + 1)
					<< " in block " << std::to_string(block);

				--unmatched;
			}
		}
	}
}


std::string ARVerifyApplication::do_name() const
{
	return "verify";
}


std::string ARVerifyApplication::do_call_syntax() const
{
	return "[OPTIONS] <filename1> [ <filename2> ... ]";
}


std::unique_ptr<Configurator> ARVerifyApplication::do_create_configurator()
	const
{
	return std::make_unique<ARVerifyConfigurator>();
}


auto ARVerifyApplication::do_run_calculation(const Configuration& config) const
	-> std::pair<int, std::unique_ptr<Result>>
{
	auto ref_respns = config.object<ARResponse>(VERIFY::RESPONSEFILE);

	auto ref_values = config.object<std::vector<Checksum>>(VERIFY::REFVALUES);

	// Configure selections (e.g. --reader and --parser)

	const calc::IdSelection id_selection;

	auto audio_selection = config.is_set(VERIFY::READERID)
		? id_selection(config.value(VERIFY::READERID))
		: nullptr;

	auto toc_selection = config.is_set(VERIFY::PARSERID)
		? id_selection(config.value(VERIFY::PARSERID))
		: nullptr;

	// If no selections are assigned, the libarcsdec default selections
	// will be used.

	// Calculate the actual ARCSs from input files

	auto [ checksums, arid, toc ] = ARCalcApplication::calculate(
			config.value(VERIFY::METAFILE),
			*config.arguments(),
			!config.is_set(VERIFY::NOFIRST),
			!config.is_set(VERIFY::NOLAST),
			{ arcstk::checksum::type::ARCS2 }, /* force ARCSv1 + ARCSv2 */
			audio_selection.get(),
			toc_selection.get()
	);

	if (checksums.size() == 0)
	{
		this->fatal_error("Calculation returned no checksums.");
	}

	// Prepare match

	std::unique_ptr<const Matcher> diff;

	if (config.is_set(VERIFY::REFVALUES))
	{
		diff = std::make_unique<ListMatcher>(checksums, ref_values);
	}

	bool print_filenames = true;

	if (/* Album requested? */not config.is_set(VERIFY::NOALBUM))
	{
		// Do verification for Offsets, ARId and TOC

		if (!toc)
		{
			this->fatal_error(
					"Album requested, but calculation returned no TOC.");
		}

		if (arid.empty())
		{
			this->fatal_error(
					"Album requested, but calculation returned an empty ARId.");
		}

		// Verify pairwise distinct audio files

		const auto& [ single_audio_file, pairwse_distinct_files, audiofiles ] =
			calc::audiofile_layout(*toc);

		if (!single_audio_file && !pairwse_distinct_files)
		{
			throw std::runtime_error("Images with audio files that contain"
				" some but not all tracks are currently unsupported");
		}

		if (!diff) // No ListMatcher for some refvals previously set?
		{
			diff = std::make_unique<AlbumMatcher>(checksums, arid, ref_respns);
		}

		print_filenames = !single_audio_file;
	} else
	{
		// No Offsets => No TOC => No ARId

		if (!diff) // No ListMatcher for some refvals previously set
		{
			diff = std::make_unique<TracksetMatcher>(checksums, ref_respns);
		}

		if (Logging::instance().has_level(arcstk::LOGLEVEL::DEBUG))
		{
			log_matching_files(checksums, *diff->match(), 1, true);
		}
	}

	// Perform match

	if (diff->matches())
	{
		ARCS_LOG_INFO << "Response contains a total match (v"
			<< (diff->best_match_is_v2() + 1)
			<< ") to the input tracks in block "
			<< diff->best_match();
	} else
	{
		ARCS_LOG_INFO << "Best match was block "  << diff->best_match()
			<< " in response, having difference " << diff->best_difference();
	}

	if (config.is_set(VERIFY::NOOUTPUT)) // implies BOOLEAN
	{
		// 0 on accurate match, else > 0
		return { diff->best_difference(), nullptr };
	}

	// Create result object

	const auto best_block = config.is_set(VERIFY::PRINTALL) &&
		config.is_set(VERIFY::RESPONSEFILE)
							? -1 // Won't be used
							: diff->best_match();

	const auto matching_version = diff->best_match_is_v2();

	auto filenames = std::vector<std::string> { };
	if (print_filenames)
	{
		if (config.no_arguments())
		{
			if (toc)
			{
				filenames = arcstk::toc::get_filenames(*toc);
			}
		} else
		{
			filenames = *config.arguments();
		}
	}

	const auto alt_prefix = std::string {/* TODO Alt-Prefix */};

	// TODO Compose set of types to be printed
	// If all types are requested, insert all types, otherwise insert the
	// matching only
	using TYPE = arcstk::checksum::type;
	std::vector<TYPE> types_to_print;

	if (config.is_set(VERIFY::PRINTALL))
	{
		// Print every match computed in the course of the calculation
		types_to_print = { TYPE::ARCS1, TYPE::ARCS2 };
	} else
	{
		// Print only type that produced the best match
		types_to_print = matching_version
			? std::vector<TYPE>{ TYPE::ARCS2 }
			: std::vector<TYPE>{ TYPE::ARCS1 };
	}

	const auto match { diff->match() };

	auto result { create_formatter(config)->format(
		/* types to print */           types_to_print,
		/* match results */            match,
		/* optional best match */      best_block,
		/* mine ARCSs */               checksums,
		/* optional mine ARId */       arid,
		/* optional TOC */             toc.get(),
		/* optional ARResponse */      ref_respns,
		/* optional input ref sums */  ref_values,
		/* input audio filenames */    filenames,
		/* optional URL prefix */      alt_prefix
	)};

	auto exit_code = config.is_set(VERIFY::BOOLEAN)
		? diff->best_difference()
		: EXIT_SUCCESS;

	return { exit_code, std::move(result) };
}

} // namespace arcsapp

