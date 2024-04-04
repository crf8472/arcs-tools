#include "clitokens.hpp"
#ifndef __ARCSTOOLS_APPARVERIFY_HPP__
#include "app-verify.hpp"
#endif

#include <algorithm>       // for replace, max, transform
#include <cctype>          // for toupper
#include <cmath>           // for ceil
#include <cstddef>         // for size_t
#include <cstdint>         // for uint32_t
#include <cstdlib>         // for EXIT_SUCCESS
#include <exception>       // for exception
#include <iterator>        // for begin, end
#include <memory>          // for unique_ptr, make_unique
#include <stdexcept>       // for invalid_argument, runtime_error
#include <sstream>         // for istringstream, ostringstream
#include <string>          // for stoul, string, to_string
#include <tuple>           // for get, make_tuple, tuple
#include <utility>         // for move, pair

#ifndef __LIBARCSTK_VERIFY_HPP__
#include <arcstk/verify.hpp>
#endif
#ifndef __LIBARCSTK_DBAR_HPP__
#include <arcstk/dbar.hpp>
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
#ifndef __ARCSTOOLS_ANSI_HPP__
#include "ansi.hpp"                 // for Colorize
#endif
#ifndef __ARCSTOOLS_TOOLS_ARID_HPP__
#include "tools-arid.hpp"           // for ARIdLayout
#endif
#ifndef __ARCSTOOLS_TOOLS_CALC_HPP__
#include "tools-calc.hpp"           // for audiofile_layout
#endif
#ifndef __ARCSTOOLS_TOOLS_DBAR_HPP__
#include "tools-dbar.hpp"           // for ContentHandler
#endif
#ifndef __ARCSTOOLS_TOOLS_TABLE_HPP__
#include "tools-table.hpp"          // for StringTableLayout, CellDecorator
									// TableComposer
#endif
#ifndef __ARCSTOOLS_RESULT_HPP__
#include "result.hpp"               // for ResultObject, Result
#endif

namespace arcsapp
{
inline namespace v_1_0_0
{

namespace registered
{
// Enable ApplicationFactory::lookup() to find this application by its name
const auto verify = RegisterApplicationType<ARVerifyApplication>("verify");
}

// libarcstk
using arcstk::Checksum;
using arcstk::Checksums;
using arcstk::DBARBuilder;
using arcstk::DBARSource;
using arcstk::Logging;
using arcstk::AlbumVerifier;
using arcstk::TracksetVerifier;

// arcsapp
using arid::ARIdLayout;
using arid::ARIdTableLayout;
using arid::RichARId;
using calc::HexLayout;
using dbar::read_from_stdin;
using table::ATTR;
using table::AddField;
using table::CellDecorator;
using table::FieldCreator;
using table::RowTableComposerBuilder;
using table::TableComposer;


// RefvaluesSource


ARId RefvaluesSource::do_id(const ChecksumSource::size_type /*block_idx*/) const
{
	return arcstk::EmptyARId;
}


Checksum RefvaluesSource::do_checksum(const ChecksumSource::size_type /*b*/,
		const ChecksumSource::size_type idx) const
{
	return source()->at(idx);
}


const uint32_t& RefvaluesSource::do_arcs_value(
		const ChecksumSource::size_type /*b*/,
		const ChecksumSource::size_type track) const
{
	return source()->at(track);
}


const uint32_t& RefvaluesSource::do_confidence(
		const ChecksumSource::size_type /*b*/,
		const ChecksumSource::size_type /*t*/) const
{
	static const auto zero = uint32_t { 0 };
	return zero;
}


const uint32_t& RefvaluesSource::do_frame450_arcs_value(
		const ChecksumSource::size_type /*b*/,
		const ChecksumSource::size_type /*t*/) const
{
	static const auto zero = uint32_t { 0 };
	return zero;
}


std::size_t RefvaluesSource::do_size(
		const ChecksumSource::size_type block_idx) const
{
	if (block_idx > 0)
	{
		throw std::invalid_argument("Only index 0 is legal, cannot access index"
				+ std::to_string(block_idx));
	}

	return source()->size();
}


std::size_t RefvaluesSource::do_size() const
{
	return 1;
}


// EmptyChecksumSource


const uint32_t EmptyChecksumSource::zero;


EmptyChecksumSource::EmptyChecksumSource() = default;


ARId EmptyChecksumSource::do_id(const ChecksumSource::size_type /*block_idx*/)
	const
{
	return arcstk::EmptyARId;
}


Checksum EmptyChecksumSource::do_checksum(const ChecksumSource::size_type /*b*/,
		const ChecksumSource::size_type /*idx*/) const
{
	return arcstk::EmptyChecksum;
}


const uint32_t& EmptyChecksumSource::do_arcs_value(
		const ChecksumSource::size_type /*b*/,
		const ChecksumSource::size_type /*track*/) const
{
	return zero;
}


const uint32_t& EmptyChecksumSource::do_confidence(
		const ChecksumSource::size_type /*b*/,
		const ChecksumSource::size_type /*t*/) const
{
	return zero;
}


const uint32_t& EmptyChecksumSource::do_frame450_arcs_value(
		const ChecksumSource::size_type /*b*/,
		const ChecksumSource::size_type /*track*/) const
{
	return zero;
}


std::size_t EmptyChecksumSource::do_size(
		const ChecksumSource::size_type /*block_idx*/) const
{
	return 0;
}


std::size_t EmptyChecksumSource::do_size() const
{
	return 0;
}


// DBARParser


DBAR DBARParser::load_data(const std::string& responsefile) const
{
	using input::CallSyntaxException;

	auto builder = DBARBuilder {};

	try
	{
		if (!responsefile.empty())
		{
			arcstk::parse_file(responsefile, &builder, nullptr);
		} else
		{
			read_from_stdin(1024, &builder, nullptr);
		}
	} catch (const std::exception& e)
	{
		throw CallSyntaxException(e.what());
	}

	return builder.result();
}


std::string DBARParser::start_message() const
{
	return "AccurateRip reference checksums (=\"Theirs\")";
}


DBAR DBARParser::do_parse_empty() const
{
	return this->load_data("");
}


DBAR DBARParser::do_parse_nonempty(const std::string& s) const
{
	return this->load_data(s);
}


// ChecksumListParser


std::string ChecksumListParser::start_message() const
{
	return "List of local reference checksums (=\"Theirs\")";
}


RefValuesType ChecksumListParser::do_parse_nonempty(
		const std::string& checksum_list) const
{
	auto i = int { 0 };
	auto refvals = parse_list_to_objects<uint32_t>(
				checksum_list,
				',',
				[&i](const std::string& s) -> uint32_t
				{
					const uint32_t value = std::stoul(s, nullptr, 16);
					ARCS_LOG(DEBUG1) << "Parse checksum: " << Checksum { value }
						<< " (Track " << ++i << ")";
					return value;
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
	using input::CallSyntaxException;
	using input::OP_VALUE;

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
	using input::OP_VALUE;
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
			"Print confidence values if available" }}
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

	// Album requested but no TOC info provided?

	if (not voptions->is_set(VERIFY::NOALBUM)
		and voptions->value(VERIFY::METAFILE).empty())
	{
		// Album requires dedicated first + last track.
		// If no TOC is passed, an album can only be verified when passed a
		// single file for each track.

		// This means we must ensure
		// (total reference track checksums == total input track files)
		// and it will be OK. We can do this only later when command line input
		// was parsed.
	}

	if (voptions->is_set(VERIFY::NOFIRST) or voptions->is_set(VERIFY::NOLAST))
	{
		if (voptions->is_set(VERIFY::METAFILE))
		{
			ARCS_LOG(WARNING) << "Passing TOC file "
				<< voptions->value(VERIFY::METAFILE)
				<< " requests album calculation, but adding "
				<< no_album_options
				<< " will ignore album calculation at least partly."
				<< " Expect unwanted results.";
		}
	}

	// Only print those things from the reference data that we actually may have

	if (voptions->is_set(VERIFY::REFVALUES))
	{
		if (voptions->is_set(VERIFY::PRINTID))
		{
			ARCS_LOG_WARNING <<
				"Ignore option PRINTID since option REFVALUES is active "
				"and reference values do not provide an ID to print.";
			voptions->unset(VERIFY::PRINTID);
		}
		if (voptions->is_set(VERIFY::PRINTURL))
		{
			ARCS_LOG_WARNING <<
				"Ignore option PRINTURL since option REFVALUES is active "
				"and reference values do not provide an URL to print.";
			voptions->unset(VERIFY::PRINTURL);
		}
		if (voptions->is_set(VERIFY::CONFIDENCE))
		{
			ARCS_LOG_WARNING <<
				"Ignore option CONFIDENCE since option REFVALUES is active and "
				"reference values do not provide confidence values to print.";
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
			[]{ return std::make_unique<DBARParser>(); } },
		{ VERIFY::REFVALUES,
			[]{ return std::make_unique<ChecksumListParser>(); } },
		{ VERIFY::COLORED,
			[]{ return std::make_unique<ColorSpecParser>(); } }
	};
}


void ARVerifyConfigurator::do_validate(const Configuration& c) const
{
	// No reference checksums at all? => Error

	if (c.object<DBAR>(VERIFY::RESPONSEFILE).size() == 0
		&& c.object<RefValuesType>(VERIFY::REFVALUES).empty())
	{
		throw std::runtime_error(
				"No reference checksums for verification available.");
	}
}


// VerifyTableCreator


VerifyTableCreator::VerifyTableCreator()
	: match_symbol_ {}
{
	// empty
}


void VerifyTableCreator::set_match_symbol(const std::string &match_symbol)
{
	match_symbol_ = match_symbol;
}


const std::string& VerifyTableCreator::match_symbol() const
{
	return match_symbol_;
}


void VerifyTableCreator::update_field_labels(TableComposer& c) const
{
	const auto label_for_mine = std::string { "Mine" };

	if (c.has_field(ATTR::CHECKSUM_ARCS2))
	{
		c.set_label(ATTR::CHECKSUM_ARCS2, label_for_mine + "(v2)");
	}

	if (c.has_field(ATTR::CHECKSUM_ARCS1))
	{
		c.set_label(ATTR::CHECKSUM_ARCS1, label_for_mine + "(v1)");
	}
}


void VerifyTableCreator::add_result_fields(std::vector<ATTR>& field_list,
		const print_flag_t /*print_flags*/,
		const std::vector<arcstk::checksum::type>& types_to_print,
		const std::size_t total_theirs_per_block) const
{
	using checksum = arcstk::checksum::type;

	for (const auto& t : types_to_print)
	{
		if (checksum::ARCS1 == t)
		{
			field_list.emplace_back(ATTR::CHECKSUM_ARCS1);
		} else
		{
			if (checksum::ARCS2 == t)
			{
				field_list.emplace_back(ATTR::CHECKSUM_ARCS2);
			}
		}

		for (auto i = std::size_t { 0 }; i < total_theirs_per_block; ++i)
		{
			field_list.emplace_back(ATTR::THEIRS);

			if (is_requested(ATTR::CONFIDENCE))
			{
				field_list.emplace_back(ATTR::CONFIDENCE);
			}
		}
	}
}


void VerifyTableCreator::populate_result_creators(
		std::vector<std::unique_ptr<FieldCreator>>& creators,
		const print_flag_t /*print_flags*/,
		const std::vector<ATTR>& field_list,
		const std::vector<arcstk::checksum::type>& types,
		const VerificationResult& vresult,
		const int block,
		const Checksums& checksums,
		const ChecksumSource& ref_source,
		const int total_theirs_per_block) const
{
	// do not repeat the find mechanism
	const auto required = [](const std::vector<ATTR>& fields, const ATTR f)
			{
				using std::begin;
				using std::end;
				using std::find;
				return find(begin(fields), end(fields), f) != end(fields);
			};

	// do not repeat populating the THEIRS fields
	const auto populate_theirs = [&]()
			{
				for (auto i = int { 0 }; i < total_theirs_per_block; ++i)
				{
					creators.emplace_back(
					std::make_unique<AddField<ATTR::THEIRS>>(&vresult, block,
						&ref_source, &types, this, total_theirs_per_block,
						is_requested(ATTR::CONFIDENCE)));
				}
			};

	if (required(field_list, ATTR::CHECKSUM_ARCS1))
	{
		creators.emplace_back(
			std::make_unique<AddField<ATTR::CHECKSUM_ARCS1>>(
				&checksums, this->checksum_layout()));

		populate_theirs();
	}

	if (required(field_list, ATTR::CHECKSUM_ARCS2))
	{
		creators.emplace_back(
			std::make_unique<AddField<ATTR::CHECKSUM_ARCS2>>(
				&checksums, this->checksum_layout()));

		populate_theirs();
	}
}


void VerifyTableCreator::assertions(const InputTuple t) const
{
	const auto checksums  = std::get<3>(t);
	const auto arid       = std::get<4>(t);
	const auto toc        = std::get<5>(t);
	const auto filenames  = std::get<7>(t);

	// Specific for verify
	const auto ref_source = std::get<6>(t);
	const auto vresult    = std::get<1>(t);
	const auto block      = std::get<2>(t);

	validate(checksums, toc, arid, filenames, *ref_source, vresult, block);
}


void VerifyTableCreator::their_checksum(const Checksum& checksum,
		const bool does_match, const int record, const int field,
		TableComposer* c) const
{
	if (does_match)
	{
		do_their_match(checksum, record, field, c);
	} else
	{
		do_their_mismatch(checksum, record, field, c);
	}
}


std::unique_ptr<Result> VerifyTableCreator::do_format(InputTuple t) const
{
	const auto types_to_print = std::get<0>(t);
	const auto vresult        = std::get<1>(t);
	const auto block          = std::get<2>(t);
	const auto checksums      = std::get<3>(t);
	const auto mine_arid      = std::get<4>(t);
	const auto toc            = std::get<5>(t);
	const auto ref_source     = std::get<6>(t);
	const auto filenames      = std::get<7>(t);
	const auto alt_prefix     = std::get<8>(t);

	using arid::build_id;
	using arid::default_arid_layout;

	auto buf = ResultBuffer  {};

	if (!mine_arid.empty())
	{
		auto layout { arid_layout()
			? arid_layout()->clone()
			: default_arid_layout(formats_labels()) };

		// Print locally calculated ARId ("Mine")
		buf.append(build_id(toc, mine_arid, alt_prefix, *layout));
	}

	const auto print_flags { create_field_requests(toc, filenames) };

	auto field_list { create_field_types(print_flags) };

	// Determine total number of 'theirs' field_types per reference block
	// (Maybe 0 for empty response and empty refvalues)
	const auto best_block_declared = bool { block > -1 };
	const auto total_theirs_per_block {
		best_block_declared ? 1 : ref_source->size()
	};

	add_result_fields(field_list, print_flags, types_to_print,
			total_theirs_per_block);

	std::vector<std::unique_ptr<FieldCreator>> creators;

	populate_creators_list(creators, field_list, *toc, checksums, filenames);
	populate_result_creators(creators, print_flags, field_list, types_to_print,
			*vresult, block, checksums, *ref_source, total_theirs_per_block);

	buf.append(format_table(
				field_list, checksums.size(), formats_labels(), creators));

	return buf.flush();
}


// MonochromeVerifyTableCreator


void MonochromeVerifyTableCreator::do_init_composer(TableComposer& c) const
{
	// Overwrite default labels for local Checksums
	this->update_field_labels(c);
}


void MonochromeVerifyTableCreator::do_their_match(const Checksum& /*checksum*/,
		const int record_idx, const int field_idx, TableComposer* c) const
{
	// XXX Why a fixed symbol? Should be configurable by decoration
	c->set_field(record_idx, field_idx, match_symbol());
}


void MonochromeVerifyTableCreator::do_their_mismatch(
		const Checksum& checksum, const int record_idx,
		const int field_idx, TableComposer* c) const
{
	c->set_field(record_idx, field_idx,
			table::formatted(checksum, *checksum_layout()));
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


// name()


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


// ColorizingVerifyTableCreator


ColorizingVerifyTableCreator::ColorizingVerifyTableCreator()
	: ColorizingVerifyTableCreator(ColorRegistry{})
{
	// empty
}


ColorizingVerifyTableCreator::
	ColorizingVerifyTableCreator(const ColorRegistry& colors)
	: colors_ { colors }
{
	// empty
}


void ColorizingVerifyTableCreator::register_decorators(TableComposer& c)
	const
{
	using ansi::Highlight;

	const auto r_size { c.total_records() };

	auto i = int { 0 };
	for (const auto& field : c.fields())
	{
		// Register a color Decorator to each "Theirs" field

		if (ATTR::THEIRS == field)
		{
			ARCS_LOG(DEBUG2) << "Register MatchDecorator to field index " << i;

			c.register_to_field(i,
				std::make_unique<MatchDecorator>(r_size,
					Highlight::BOLD, colors_.get(DecorationType::MATCH),
					Highlight::BOLD, colors_.get(DecorationType::MISMATCH) ));
		}

		++i;
	}
}


void ColorizingVerifyTableCreator::do_init_composer(TableComposer& c) const
{
	// Overwrite default labels for local Checksums
	this->update_field_labels(c);

	// Register color Decorators to each "Theirs" field
	this->register_decorators(c);
}


void ColorizingVerifyTableCreator::do_their_match(const Checksum& checksum,
		const int record_idx, const int field_idx, TableComposer* c) const
{
	c->set_field(record_idx, field_idx,
			table::formatted(checksum, *checksum_layout()));

	ARCS_LOG(DEBUG2) << "Mark cell " << record_idx << ", " << field_idx
		<< " as match-decorated";

	c->mark(record_idx, field_idx);
}


void ColorizingVerifyTableCreator::do_their_mismatch(
		const Checksum& checksum, const int record_idx, const int field_idx,
		TableComposer* c) const
{
	c->set_field(record_idx, field_idx,
			table::formatted(checksum, *checksum_layout()));

	ARCS_LOG(DEBUG2) << "Mark cell " << record_idx << ", " << field_idx
		<< " as mismatch-decorated";
}


std::pair<ansi::Color, ansi::Color> ColorizingVerifyTableCreator::colors(
		DecorationType d) const
{
	return colors_.get(d);
}


ansi::Color ColorizingVerifyTableCreator::color_fg(DecorationType d) const
{
	return colors_.has(d) ? colors_.get_fg(d) : ansi::Color::FG_DEFAULT;
}


ansi::Color ColorizingVerifyTableCreator::color_bg(DecorationType d) const
{
	return colors_.has(d) ? colors_.get_fg(d) : ansi::Color::FG_DEFAULT;
}


void ColorizingVerifyTableCreator::set_color_fg(DecorationType d, Color c)
{
	colors_.set_fg(d, c);
}


void ColorizingVerifyTableCreator::set_color_bg(DecorationType d, Color c)
{
	colors_.set_bg(d, c);
}


// SourceCreator


bool SourceCreator::reference_is_dbar(
			const DBAR& dBAR, const RefValuesType& /*refvalues*/) const
{
	return dBAR.size() > 0;
}


std::unique_ptr<const ChecksumSource>
SourceCreator::create_reference_source(const DBAR& dBAR,
		const RefValuesType& refvalues) const
{
	std::unique_ptr<const ChecksumSource> ref_src;

	if (reference_is_dbar(dBAR, refvalues))
	{
		ref_src = std::make_unique<DBARSource>(&dBAR);
	} else
	{
		if (!refvalues.empty())
		{
			ref_src = std::make_unique<RefvaluesSource>(&refvalues);
		} else
		{
			ref_src = std::make_unique<EmptyChecksumSource>();
			// TODO Why proceed? Just throw
		}
	}

	return ref_src;
}


std::unique_ptr<const ChecksumSource>
SourceCreator::operator()(const DBAR& dBAR, const RefValuesType& refvalues)
	const
{
	return create_reference_source(dBAR, refvalues);
}


// AddField


void AddField<ATTR::THEIRS>::do_create(TableComposer* c, const int r_idx)
	const
{
	if (r_idx < 0)
	{
		//throw;
	}
	const auto record_idx = static_cast<std::size_t>(r_idx);

	using std::to_string;

	auto block_idx  = std::size_t { 0 }; // Iterate over blocks of checksums
	auto curr_type  { types_to_print_->at(0) }; // Current checksum type
	auto does_match = bool { false }; // Is current checksum a match?

	// Total number of THEIRS fields in the entire record type
	const auto total_theirs = total_theirs_per_block_ * types_to_print_->size();

	// 1-based number of the reference block to print
	auto idx_label = int { 0 };

	// field index of the "theirs"-column
	auto field_idx = int { 0 };

	// Create all "theirs" fields
	for (auto b = std::size_t { 0 }; b < total_theirs; ++b)
	{
		// Enumerate one or more blocks
		// (If block_ < 0 PRINTALL is present)
		block_idx =  block_ >= 0
			? static_cast<std::size_t>(block_)
			: b % total_theirs_per_block_;

		curr_type =
			types_to_print_->at(std::ceil(b / total_theirs_per_block_));

		does_match = vresult_->track(block_idx, record_idx,
						curr_type == arcstk::checksum::type::ARCS2);

		idx_label = block_idx + 1;
		field_idx = c->field_idx(ATTR::THEIRS, b + 1);

		// Update field label to show best block index
		c->set_label(field_idx, DefaultLabel<ATTR::THEIRS>()
					+ (idx_label < 10 ? std::string{" "} : std::string{})
					// XXX Block index greater than 99 will screw up labels
					+ to_string(idx_label));

		formatter_->their_checksum(
				checksums_->checksum(block_idx, record_idx), does_match,
				record_idx, field_idx, c);

		if (print_confidence_)
		{
			table::add_field(c, record_idx, field_idx + 1,
				to_string(checksums_->confidence(block_idx, record_idx)));
		}
	}
}


AddField<ATTR::THEIRS>::AddField(
		const VerificationResult* vresult,
		const int block,
		const ChecksumSource* checksums,
		const std::vector<arcstk::checksum::type>* types,
		const VerifyTableCreator* formatter,
		const std::size_t total_theirs_per_block,
		const bool print_confidence)
	: vresult_                { vresult   }
	, block_                  { block     }
	, checksums_              { checksums }
	, types_to_print_         { types     }
	, formatter_              { formatter }
	, total_theirs_per_block_ { total_theirs_per_block }
	, print_confidence_       { print_confidence       }
{
	/* empty */
}


// validate


void validate(const Checksums& checksums, const TOC* toc,
	const ARId& arid, const std::vector<std::string>& filenames,
	const ChecksumSource& reference,
	const VerificationResult* vresult, const int block)
{
	calc::validate(checksums, toc, arid, filenames);

	if (!reference.size())
	{
		throw std::invalid_argument("Missing reference checksums, "
				"nothing to print.");
	}

	// TODO ref should have at least one block with id == arid

	auto at_least_one_block_of_equal_size = bool { false };
	for (auto i = std::size_t {0}; i < reference.size(); ++i)
	{
		if (reference.size(i) == checksums.size())
		{
			at_least_one_block_of_equal_size = true;
			break;
		}
	}

	if (!at_least_one_block_of_equal_size)
	{
		throw std::invalid_argument("Mismatch: "
				"There are " + std::to_string(checksums.size())
				+ " local tracks to verify, but no block in reference "
				" contains exactly this number of tracks");
	}

	// TODO reference should have at least one block with id == arid

	if (!vresult)
	{
		throw std::invalid_argument("Missing match information, "
				"nothing to print.");
	}

	if (block > vresult->total_blocks()) // block < 0 is ok (means: no block)
	{
		throw std::invalid_argument("Mismatch: "
				"Match contains no block " + std::to_string(block)
				+ " but contains only "
				+ std::to_string(vresult->total_blocks()) + " blocks.");
	}
}


// ARVerifyApplication


std::unique_ptr<VerifyTableCreator> ARVerifyApplication::create_formatter(
		const Configuration& config) const
{
	auto fmt = std::unique_ptr<VerifyTableCreator>();

	if (config.is_set(VERIFY::COLORED))
	{
		fmt = std::make_unique<ColorizingVerifyTableCreator>(
				config.object<ColorRegistry>(VERIFY::COLORED));
	} else
	{
		fmt = std::make_unique<MonochromeVerifyTableCreator>();
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
	fmt->set_format_labels(!config.is_set(VERIFY::NOLABELS));

	// TOC present? Helper for determining other properties
	const bool has_toc = !config.value(VERIFY::METAFILE).empty();

	// Print track numbers if they are not forbidden and a TOC is present
	fmt->set_format_field(ATTR::TRACK,
			config.is_set(VERIFY::NOTRACKS) ? false : has_toc);

	// Print offsets if they are not forbidden and a TOC is present
	fmt->set_format_field(ATTR::OFFSET,
			config.is_set(VERIFY::NOOFFSETS) ? false : has_toc);

	// Print lengths if they are not forbidden
	fmt->set_format_field(ATTR::LENGTH, !config.is_set(VERIFY::NOLENGTHS));

	// Print filenames if they are not forbidden and a TOC is _not_ present
	fmt->set_format_field(ATTR::FILENAME,
			!config.is_set(VERIFY::NOFILENAMES) || !has_toc);

	// Indicate a matching checksum by this symbol
	fmt->set_match_symbol("==");

	// Indicate that confidence values should be printed (if available)
	fmt->set_format_field(ATTR::CONFIDENCE, config.is_set(VERIFY::CONFIDENCE));

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
		const VerificationResult& vresult, const int block,
		const bool version) const
{
	auto unmatched { checksums.size() };

	// Traverse checksums
	for (std::size_t t = 0; t < checksums.size() and unmatched > 0; ++t)
	{
		// Traverse specified block
		for (int track = 0; track < vresult.tracks_per_block(); ++track)
		{
			if (vresult.track(block, track, version))
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
	const auto dbar   = config.object<DBAR>(VERIFY::RESPONSEFILE);
	const auto refvls = config.object<RefValuesType>(VERIFY::REFVALUES);

	const auto get_src = SourceCreator {};
	const auto ref_source { get_src(dbar, refvls) };

	ARCS_LOG_DEBUG << "Reference checksum source contains "
		<< ref_source->size() << "blocks of checksums";

	// Album calculation is requested but no metafile is passed

	if (not config.is_set(VERIFY::NOALBUM)
		and config.value(VERIFY::METAFILE).empty())
	{
		// If no TOC is available, an album can only be verified when passed
		// its audio input as a single file for each track. Only in this case,
		// we do not need the offsets. (Also a single track album needs an
		// offset for its track.)

		// This means we must ensure that the total number of reference track
		// checksums is equal to the total number of input track files.

		if (!ref_source->size()
				|| ref_source->size(0) != config.arguments()->size())
		{
			this->fatal_error("Album requested, but number of AccurateRip "
					"references does not match number of input audio files.");
		}
	}

	// Configure selections (e.g. --reader and --parser)

	auto audio_selection = create_selection(CALC::READERID, config);
	auto toc_selection   = create_selection(CALC::PARSERID, config);

	// If no selections are assigned, the libarcsdec default selections
	// will be used.

	// Calculate the actual ARCSs from input files

	auto [ checksums, mine_arid, toc ] = ARCalcApplication::calculate(
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

	// Prepare verification

	std::unique_ptr<const VerificationResult> vresult { nullptr };

	if (config.is_set(VERIFY::REFVALUES))
	{
		// Process as list of reference values

		ARCS_LOG_DEBUG << "Process reference input as value list";

		const auto v = std::make_unique<TracksetVerifier>(checksums);
		vresult = v->perform(*ref_source);
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

		if (mine_arid.empty())
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

		if (!vresult) // No previous result from refvals?
		{
			ARCS_LOG_DEBUG <<
				"Process reference input as AccurateRip response for album";
			ARCS_LOG_DEBUG <<
				"Computed AccurateRip ID: "  << mine_arid.to_string();
			ARCS_LOG_DEBUG <<
				"Computed AccurateRip URL: " << mine_arid.url();

			const auto v =
				std::make_unique<AlbumVerifier>(checksums, mine_arid);
			vresult = v->perform(*ref_source);
		}

		print_filenames = !single_audio_file;
	} else
	{
		// No Offsets => No TOC => No ARId

		if (!vresult) // No result from refvals?
		{
			ARCS_LOG_DEBUG <<
				"Process reference input as AccurateRip response for tracks";

			const auto v = std::make_unique<TracksetVerifier>(checksums);
			vresult = v->perform(*ref_source);
		}

		if (Logging::instance().has_level(arcstk::LOGLEVEL::DEBUG))
		{
			log_matching_files(checksums, *vresult, 1, true);
		}
	}

	// Perform verification

	const auto best_b = vresult->best_block();

	if (vresult->all_tracks_verified())
	{
		ARCS_LOG_INFO << "Response contains a total match (v"
			<< (std::get<1>(best_b) + 1)
			<< ") to the input tracks in block "
			<< std::get<0>(best_b);
	} else
	{
		ARCS_LOG_INFO << "Best match was block "  << std::get<0>(best_b)
			<< " in response, having difference " << std::get<2>(best_b);
	}

	if (config.is_set(VERIFY::NOOUTPUT)) // implies BOOLEAN
	{
		// 0 on accurate match, else > 0
		return { std::get<2>(best_b), nullptr };
	}

	// Create result object

	const auto best_block = config.is_set(VERIFY::PRINTALL) &&
		config.is_set(VERIFY::RESPONSEFILE)
							? -1 // Won't be used
							: std::get<0>(best_b);

	const auto matching_version = std::get<1>(best_b);

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

	// TODO Create formatter, then add types_to_print as print flags,
	// remove the dedicated vector
	auto result { create_formatter(config)->format(
		/* types to print */           types_to_print,
		/* verification results */     vresult.get(),
		/* optional best match */      best_block,
		/* mine ARCSs */               checksums,
		/* optional mine ARId */       mine_arid,
		/* optional TOC */             toc.get(),
		/* reference checksum source */ref_source.get(),
		/* input audio filenames */    filenames,
		/* optional URL prefix */      alt_prefix
	)};

	auto exit_code = config.is_set(VERIFY::BOOLEAN)
		? std::get<2>(best_b) // best difference
		: EXIT_SUCCESS;

	return { exit_code, std::move(result) };
}

} // namespace v_1_0_0
} // namespace arcsapp

