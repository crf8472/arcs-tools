/**
 * \internal
 *
 * \file
 *
 * \brief Implements symbols from app-verify.hpp.
 */

#ifndef ARCSTOOLS_APPVERIFY_HPP_
#include "app-verify.hpp"
#endif

#include <algorithm>       // for replace, max, transform
#include <cctype>          // for toupper
#include <cmath>           // for ceil
#include <cstddef>         // for size_t
#include <cstdlib>         // for EXIT_SUCCESS
#include <exception>       // for exception
#include <iterator>        // for begin, end
#include <memory>          // for unique_ptr, make_unique
#include <sstream>         // for istringstream, ostringstream
#include <stdexcept>       // for invalid_argument, runtime_error
#include <string>          // for stoul, string, to_string
#include <tuple>           // for get, tuple
#include <utility>         // for move, pair

#ifndef LIBARCSTK_METADATA_HPP_
#include <arcstk/metadata.hpp>      // for ToC
#endif
#ifndef LIBARCSTK_VERIFY_HPP_
#include <arcstk/verify.hpp>        // for ChecksumSource
#endif
#ifndef LIBARCSTK_DBAR_HPP_
#include <arcstk/dbar.hpp>          // for DBAR, DBARSource
#endif
#ifndef LIBARCSTK_LOGGING_HPP_
#include <arcstk/logging.hpp>       // for ARCS_LOG_DEBUG, ARCS_LOG_ERROR
#endif

#ifndef ARCSTOOLS_APPREGISTRY_HPP_
#include "appregistry.hpp"          // for RegisterApplicationType
#endif
#ifndef ARCSTOOLS_CONFIG_HPP_
#include "config.hpp"               // for Configurator, OptionCode
#endif
#ifndef ARCSTOOLS_TOOLS_ARID_HPP_
#include "tools-arid.hpp"           // for ARIdLayout
#endif
#ifndef ARCSTOOLS_TOOLS_CALC_HPP_
#include "tools-calc.hpp"           // for audiofile_layout
#endif
#ifndef ARCSTOOLS_TOOLS_DBAR_HPP_
#include "tools-dbar.hpp"           // for ContentHandler
#endif
#ifndef ARCSTOOLS_TOOLS_INPUT_HPP_
#include "tools-input.hpp"          // for DBARParser, EmptyChecksumSource
#endif
#ifndef ARCSTOOLS_TOOLS_TABLE_HPP_
#include "tools-table.hpp"          // for StringTableLayout, CellDecorator
									// TableComposer
#endif
#ifndef ARCSTOOLS_TOOLS_VALIDATE_HPP_
#include "tools-validate.hpp"       // for Validate
#endif
#ifndef ARCSTOOLS_RESULT_HPP_
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
using arcstk::ARId;
using arcstk::Checksum;
using arcstk::Checksums;
using arcstk::ChecksumSource;
using arcstk::DBARSource;
using arcstk::Logging;
using arcstk::AlbumVerifier;
using arcstk::TracksetVerifier;

// arcsapp
using arid::ARID_FLAG;
using arid::ARIdLayout;
using arid::ARIdTableLayout;
using arid::RichARId;
using calc::HexLayout;
using input::DBARParser;
using input::ChecksumValuesParser;
using input::ChecksumValuesSource;
using input::ChecksumValuesType;
using table::ATTR;
using table::AddField;
using table::CellDecorator;
using table::FieldCreator;
using table::RowTableComposerBuilder;
using table::StringTableLayout;
using table::TableComposer;


// ColorSpecParser


ColorSpecParser::ColorSpecParser()
	: registry_ {/*empty*/}
{
	// empty
}


std::string ColorSpecParser::start_message() const
{
	return "List of output color requests";
}


void ColorSpecParser::do_parse_nonempty(const std::string& input) const
{
	using cli::CallSyntaxException;
	using cli::OP_VALUE;

	if (input == OP_VALUE::USE_DEFAULT)
	{
		return; /* default colors */
	}

	const std::string r_sep = ":"; // record separator (name : value)
	const std::string v_sep = "+"; // value separator  (value, value)

	const auto r = std::addressof(registry_);
	r->clear(); // remove defaults, use only values from input string

	auto count = counter();

	input::parse_list(input, ',',
			[&r, &r_sep, &v_sep, &count](const std::string& s)
			{
				// parse a single TYPE:COLOR pair

				const auto colon = s.find(r_sep);
				const auto plus  = s.find(v_sep);

				if (colon == std::string::npos)
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
					using std::cbegin;
					using std::cend;

					std::transform(cbegin(str), cend(str), begin(str),
						[](unsigned char c) { return std::toupper(c); });
					return str;
				};

				const auto type_str { uppercase(s.substr(0, colon)) };
				auto color1 = std::string {};
				auto color2 = std::string {};

				if (plus == std::string::npos)
				{
					// Single color
					color1 = uppercase(s.substr(colon + r_sep.length()));

					ARCS_LOG(DEBUG1) << "Set colors for " << type_str << ": "
						<< color1;
				} else
				{
					const auto start { colon + r_sep.length() };

					// Color pair
					color1 = uppercase(s.substr(start, plus - start));
					color2 = uppercase(s.substr(plus + v_sep.length()));

					ARCS_LOG(DEBUG1) << "Set colors for " << type_str << ": "
						<< color1 << ", " << color2;
				}

				++count;

				using ansi::get_color;

				if (color2.empty())
				{
					const auto color { get_color(color1) };

					if (is_foreground(color))
					{
						r->set_fg(get_decorationtype(type_str), color);
					} else
					{
						r->set_bg(get_decorationtype(type_str), color);
					}

				} else
				{
					r->set(get_decorationtype(type_str),
						get_color(color1), get_color(color2));
				}
			});
}


ColorRegistry ColorSpecParser::provide_object() const
{
	return registry_;
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
constexpr OptionCode VERIFY::COLORED;
constexpr OptionCode VERIFY::CONFIDENCE;


// ARVerifyConfigurator


OptionCode ARVerifyConfigurator::select_reference_source(const Configuration& c)
	const
{
	const auto is_valid = [](const ChecksumSource* s) -> bool
	{
		return s && s->size() > 0;// TODO && s.has_nonempty_blocks()
	};

	// TODO Just cache the size when parsing instead of querying for objects!
	// Could be done by inspecting total_records_parsed() in apply_parsers()
	// Empty input could be directly ignored instead of analyzed

	if (const auto o = c.object_ptr<DBAR>(VERIFY::RESPONSEFILE);
			o && is_valid(std::make_unique<DBARSource>(o).get()))
	{
		ARCS_LOG(DEBUG1)<< "Reference source is a dBAR object of size "
			<< o->size();

		return VERIFY::RESPONSEFILE;
	}

	if (const auto o = c.object_ptr<ChecksumValuesSource>(VERIFY::REFVALUES);
			o && is_valid(o))
	{
		ARCS_LOG(DEBUG1) << "Reference source is a sequence of checksum values "
			<< "of size " << o->size();

		return VERIFY::REFVALUES;
	}

	throw ConfigurationException("Input of either a non-empty dBAR object"
			" or a non-empty list of reference checksums is required");
}


void ARVerifyConfigurator::do_flush_local_options(OptionRegistry& r) const
{
	using cli::OP_VALUE;
	using std::cend;

	r.insert(cend(r),
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
			"List all supported file formats for ToC metadata" }},

		{ VERIFY::LIST_AUDIO_FORMATS ,
		{  "list-audio-formats", false, OP_VALUE::FALSE,
			"List all supported audio codec/container formats" }},

		// from CALCBASE

		{ VERIFY::METAFILE ,
		{  'm', "metafile", true, OP_VALUE::NONE,
			"Specify metadata file (ToC) to use" }},

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

	// Album requested but no ToC info provided?

	if (not voptions->is_set(VERIFY::NOALBUM)
		and voptions->value(VERIFY::METAFILE).empty())
	{
		// TODO

		// Album requires dedicated first + last track.
		// If no ToC is passed, an album can only be verified when passed a
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
			ARCS_LOG(WARNING) << "Passing ToC file "
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
	using Validation = valid::Validate<Options>;

	const std::vector<Validation> validations =
	{
		Validation
		{
			"Use either dBAR file or reference values",
			[](const Options& o)
			{
				return o.is_set(VERIFY::RESPONSEFILE)
							|| o.is_set(VERIFY::REFVALUES);
			},
			"No reference values specified."
			" One of --refvalues and -r/--response is required"
		},
		Validation
		{
			"Do not use both, dBAR file and reference values",
			[](const Options& o)
			{
				return ! (o.is_set(VERIFY::RESPONSEFILE)
							&& o.is_set(VERIFY::REFVALUES));
			},
			"Cannot process --refvalues along with -r/--response, "
			"only one of these options is allowed"
		}
	};

	for (const auto& validation : validations)
	{
		try
		{
			validation.perform(options);
		} catch (const std::exception& e)
		{
			throw ConfigurationException(e.what());
		}
	}
}


OptionParsers ARVerifyConfigurator::do_parser_list() const
{
	return {
		{ VERIFY::RESPONSEFILE,
			[]{ return std::make_unique<DBARParser>(); } },
		{ VERIFY::REFVALUES,
			[]{ return std::make_unique<ChecksumValuesParser>(); } },
		{ VERIFY::COLORED,
			[]{ return std::make_unique<ColorSpecParser>(); } }
	};
}


void ARVerifyConfigurator::do_postprocess(Configuration& c) const
{
	// Pre-select the reference source to use
	const auto ref_source { select_reference_source(c) };
	c.put(VERIFY::REFSOURCE, ref_source);
}


void ARVerifyConfigurator::do_validate(const Configuration& c) const
{
	using Validation = valid::Validate<Configuration>;

	Validation
	{
		"Provide non-empty reference source",
		[](const Configuration& config)
		{
			const auto* const ref_source {
					config.object_ptr<OptionCode>(VERIFY::REFSOURCE) };

			return ref_source && OPTION::NONE != *ref_source;
		},
		"No reference checksums for verification available."
	}.perform(c);
}


// VerifyTableCreator


VerifyTableCreator::VerifyTableCreator()
	: match_symbol_ {}
{
	// empty
}


void VerifyTableCreator::set_match_symbol(const std::string& match_symbol)
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
				using std::cbegin;
				using std::cend;
				using std::find;
				return find(cbegin(fields), cend(fields), f) != cend(fields);
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

	//TODO lambda: add attr with theirs
	// ATTR field_list

	if (required(field_list, ATTR::CHECKSUM_ARCS1))
	{
		// add ARCSv1 row or column
		creators.emplace_back(
			std::make_unique<AddField<ATTR::CHECKSUM_ARCS1>>(
				&checksums, this->checksum_layout()));

		// add corresponding theirs column
		populate_theirs();
	}

	if (required(field_list, ATTR::CHECKSUM_ARCS2))
	{
		// add ARCSv2 row or column
		creators.emplace_back(
			std::make_unique<AddField<ATTR::CHECKSUM_ARCS2>>(
				&checksums, this->checksum_layout()));

		// add corresponding theirs column
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

	arid::validate(arid, checksums.size(), toc);
	// TODO ref_source should have at least one block with id == arid

	//TODO using verify::validate;
	validate(checksums, toc, filenames, *ref_source, vresult, block);
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
	const auto arid           = std::get<4>(t);
	const auto toc            = std::get<5>(t);
	const auto ref_source     = std::get<6>(t);
	const auto filenames      = std::get<7>(t);
	const auto alt_prefix     = std::get<8>(t);

	using arid::build_id;

	auto buf = ResultBuffer {};

	// If ARId is present, print it

	if (!arid.empty())
	{
		// Print locally calculated ARId ("Mine")
		buf.append(build_id(arid, alt_prefix, *arid_layout()->clone()));
	}

	const auto print_flags { create_field_requests(toc, filenames) };

	// Create ordered list of table columns

	auto field_list { create_field_types(print_flags) };

	// Determine total number of 'theirs' field_types per reference block
	// (Maybe 0 for empty response and empty refvalues)
	const auto total_theirs_per_block {
		block > -1/* best block declared */ ? 1 : ref_source->size()
	};

	add_result_fields(field_list, print_flags, types_to_print,
			total_theirs_per_block);

	// Populate table with data creators

	auto creators { std::vector<std::unique_ptr<FieldCreator>>{} };

	populate_creators_list(creators, field_list, filenames, toc, checksums);

	if (total_theirs_per_block > std::numeric_limits<int>::max())
	{
		ARCS_LOG_ERROR << "Too many entries for 'theirs'";
	}

	populate_result_creators(creators, print_flags, field_list, types_to_print,
			*vresult, block, checksums, *ref_source,
			static_cast<int>(total_theirs_per_block));

	// Add table to result

	buf.append(format_table(field_list, checksums.size(), creators));

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


std::string MatchDecorator::do_decorate_set(const std::string& s) const
{
	return  colored(hl(DecorationType::MATCH),
				fg(DecorationType::MATCH),
				bg(DecorationType::MATCH),
				s);
}


std::string MatchDecorator::do_decorate_unset(const std::string& s) const
{
	return  colored(hl(DecorationType::MISMATCH),
				fg(DecorationType::MISMATCH),
				bg(DecorationType::MISMATCH),
				s);
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
	using std::cend;
	return colors_.find(d) != cend(colors_);
}


std::pair<ansi::Color,ansi::Color> ColorRegistry::get(DecorationType d) const
{
	using std::cend;
	if (const auto c = colors_.find(d); c != cend(colors_))
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
	using std::cend;

	if (const auto p = colors_.find(d); p != cend(colors_))
	{
		p->second.first = c;
	} else
	{
		colors_.insert({ d, { c, ansi::Color::BG_DEFAULT }});
	}
}


void ColorRegistry::set_bg(DecorationType d, ansi::Color c)
{
	using std::cend;

	if (const auto p = colors_.find(d); p != cend(colors_))
	{
		p->second.second = c;
	} else
	{
		colors_.insert({ d, { ansi::Color::FG_DEFAULT, c }});
	}
}


void ColorRegistry::set(DecorationType d, ansi::Color fg, ansi::Color bg)
{
	using std::cend;

	if (const auto p = colors_.find(d); p != cend(colors_))
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
	ColorizingVerifyTableCreator(ColorRegistry colors)
	: colors_ { std::move(colors) }
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

		if (ATTR::CHECKSUM_ARCS2 == field
				|| ATTR::CHECKSUM_ARCS1 == field)
		{
			//c.register_to_field(i,
			//	std::make_unique<MineDecorator>(r_size,
			//		Highlight::NORMAL, colors_.get(DecorationType::MIINE) ));
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

		does_match = vresult_->track(block_idx, record_idx, curr_type);

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


void validate(const Checksums& checksums, const ToC& toc,
	const std::vector<std::string>& filenames,
	const ChecksumSource& reference,
	const VerificationResult* /*vresult*/, const int /*block*/)
{
	calc::validate(checksums, toc, filenames);

	{ //scope
		using Validation = valid::Validate<ChecksumSource, Checksums>;

		const std::vector<Validation> validations =
		{
			Validation
			{
				"Reference source actually contains Checksums",
				[](const ChecksumSource& s, const Checksums& /*c*/) noexcept
				{
					return s.size() > 0;
				},
				"Reference source does not contain any blocks",
			},
			Validation
			{
				"Reference source contains at least 1 block of same size",
				[](const ChecksumSource& s, const Checksums& c) noexcept
				{
					for (auto i = std::size_t { 0 }; i < s.size(); ++i)
					{
						if (s.size(i) == c.size()) { return true; }
					}

					return false;
				},
				"Reference does not contain a source with the same number of "
				"total tracks like the actual checksums",
			}

			// TODO reference should have at least one block with id == arid
		};

		for (const auto& validation : validations)
		{
			validation.perform(reference, checksums);
		}
	}
}


// ARVerifyApplication


std::unique_ptr<VerifyTableCreator> ARVerifyApplication::create_formatter(
		const Configuration& config) const
{
	auto fmt = std::unique_ptr<VerifyTableCreator>();


	// Configure Attributes

	ARCS_LOG(DEBUG3) << "Define output attributes:";

	if (config.is_set(VERIFY::COLORED))
	{
		ARCS_LOG(DEBUG3) << "Print with colors";

		fmt = std::make_unique<ColorizingVerifyTableCreator>(
				config.object<ColorRegistry>(VERIFY::COLORED));
	} else
	{
		ARCS_LOG(DEBUG3) << "Print without colors";

		fmt = std::make_unique<MonochromeVerifyTableCreator>();
	}

	// ToC present? Helper for determining other properties
	const bool has_toc = !config.value(VERIFY::METAFILE).empty();

	// Print track numbers if they are not forbidden and a ToC is present
	fmt->update_property(ATTR::TRACK,
			config.is_set(VERIFY::NOTRACKS) ? false : has_toc);

	ARCS_LOG(DEBUG3) << "Print TRACK :     " << fmt->has_property(ATTR::TRACK);

	// Print offsets if they are not forbidden and a ToC is present
	fmt->update_property(ATTR::OFFSET,
			config.is_set(VERIFY::NOOFFSETS) ? false : has_toc);

	ARCS_LOG(DEBUG3) << "Print OFFSET:     " << fmt->has_property(ATTR::OFFSET);

	// Print lengths if they are not forbidden
	fmt->update_property(ATTR::LENGTH, !config.is_set(VERIFY::NOLENGTHS));

	ARCS_LOG(DEBUG3) << "Print LENGTH:     " << fmt->has_property(ATTR::LENGTH);

	// Print filenames if they are not forbidden and a ToC is _not_ present
	fmt->update_property(ATTR::FILENAME,
			!config.is_set(VERIFY::NOFILENAMES) || !has_toc);

	ARCS_LOG(DEBUG3) << "Print FILENAME:   " <<
			fmt->has_property(ATTR::FILENAME);

	// Indicate that confidence values should be printed (if available)
	fmt->update_property(ATTR::CONFIDENCE, config.is_set(VERIFY::CONFIDENCE));

	ARCS_LOG(DEBUG3) << "Print CONFIDENCE: " <<
			fmt->has_property(ATTR::CONFIDENCE);

	// Indicate a matching checksum by this symbol
	fmt->set_match_symbol("==");


	// Layout for checksum table

	auto cs_table_layout { std::make_unique<StringTableLayout>() };

	// Set inner column delimiter

	if (config.is_set(VERIFY::COLDELIM))
	{
		cs_table_layout->set_col_inner_delim(config.value(VERIFY::COLDELIM));
	}

	// Remove labels and delims if requested

	if (config.is_set(VERIFY::NOLABELS))
	{
		ARCS_LOG(DEBUG3) << "Print without labels";

		cs_table_layout->set_row_labels(false);
		cs_table_layout->set_col_labels(false);
		cs_table_layout->set_col_labels_delims(false); // for safety

	} else
	{
		ARCS_LOG(DEBUG3) << "Print with labels";
	}

	// Print labels or not
	//fmt->set_with_labels(!config.is_set(VERIFY::NOLABELS));


	// Layout for ARId

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

	id_layout->set_label(ARID_FLAG::ID,  "ID(mine) ");
	id_layout->set_label(ARID_FLAG::URL, "URL(mine)");


	// Put things together

	fmt->set_arid_layout(std::move(id_layout));
	fmt->set_checksum_layout(std::make_unique<HexLayout>());
	fmt->set_table_layout(std::move(cs_table_layout));
	fmt->set_builder(std::make_unique<RowTableComposerBuilder>());

	return fmt;
}


void ARVerifyApplication::log_matching_files(const Checksums& checksums,
		const VerificationResult& vresult, const int block,
		const arcstk::checksum::type type) const
{
	using std::to_string;
	using index_type = VerificationResult::index_type;

	auto unmatched { checksums.size() };

	// Traverse checksums
	for (std::size_t t = 0; t < checksums.size() and unmatched > 0; ++t)
	{
		// Traverse specified block
		for (auto track = unsigned { 0 }; track < vresult.tracks_per_block();
				++track)
		{
			if (vresult.track(block, static_cast<index_type>(track), type))
			{
				ARCS_LOG_DEBUG << "Pos " << to_string(track)
					<< " matches track " << to_string(track + 1)
					<< " in block " << to_string(block);

				--unmatched;
			}
		}
	}
}


std::unique_ptr<ChecksumSource> ARVerifyApplication::get_reference_source(
		const Configuration& c) const
{
	const auto option { c.object<OptionCode>(VERIFY::REFSOURCE) };

	if (VERIFY::RESPONSEFILE == option)
	{
		using arcstk::DBARSource;

		return std::make_unique<DBARSource>(
					c.object_ptr<DBAR>(VERIFY::RESPONSEFILE));
	}

	if (VERIFY::REFVALUES == option)
	{
		return std::make_unique<ChecksumValuesSource>(
					c.object<ChecksumValuesSource>(VERIFY::REFVALUES));
	}

	return nullptr;
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
	const auto ref_source { get_reference_source(config) };

	// Validations for selected reference source

	using Validation = valid::Validate<Configuration, ChecksumSource>;

	Validation // 1
	{
		"Reference values must not be empty",
		[](const Configuration& /*c*/, const ChecksumSource& s)
		{
			return s.size() > 0; // TODO && s.has_nonempty_blocks()
		},
		"Verification requires non-empty input of reference values."

	}.perform(config, *ref_source);

	Validation // 2
	{
		// If no ToC is available, an album can only be verified when passed
		// its audio input as a single file for each track. Exclusively in this
		// case, it is allowed to have no offsets. (Also a single track album
		// needs an offset for its track.)

		"If album requested but no metafile passed, number of input files "
		"must match number of reference values",
		[](const Configuration& c, const ChecksumSource& s)
		{
			if (!c.is_set(VERIFY::NOALBUM) && c.value(VERIFY::METAFILE).empty())
			{
				for (auto i = std::size_t { 0 }; i < s.size(); ++i)
				{
					if (c.arguments()->size() == s.size(i)) { return true; }
				}
				return false;
			}
			return true;
		},
		"Album requested, but number of AccurateRip "
		"references does not match number of input audio files."

	}.perform(config, *ref_source);

	ARCS_LOG_DEBUG << "Reference checksum source contains "
		<< ref_source->size() << "blocks of checksums";

	// Configure selections (e.g. --reader and --parser)

	auto audio_selection = create_selection(CALC::READERID, config);
	auto toc_selection   = create_selection(CALC::PARSERID, config);

	// If no selections are assigned, the libarcsdec default selections
	// will be used.

	const auto requested_types = this->requested_types(config);

	// Calculate the actual ARCSs from input files

	auto [ checksums, toc ] = ARCalcApplication::calculate(
			*config.arguments(),
			config.value(VERIFY::METAFILE),
			!config.is_set(VERIFY::NOFIRST),
			!config.is_set(VERIFY::NOLAST),
			requested_types,
			audio_selection.get(),
			toc_selection.get()
	);

	if (checksums.size() == 0)
	{
		this->fatal_error("Calculation returned no checksums.");
	}

	// ARId

	auto mine_arid = (toc && toc.complete()) ? arcstk::make_arid(toc) : ARId{};

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
		// Do verification for Offsets, ARId and ToC

		if (!toc)
		{
			this->fatal_error(
					"Album requested, but calculation returned no ToC.");
		}

		if (mine_arid.empty())
		{
			this->fatal_error(
					"Album requested, but calculation returned an empty ARId.");
		}

		// Verify pairwise distinct audio files

		const auto& [ single_audio_file, pairwise_distinct_files ] =
			calc::ToCFiles::flags(toc.filenames());

		if (!single_audio_file && !pairwise_distinct_files)
		{
			throw std::runtime_error("Images with audio files that contain"
				" some but not all tracks are currently unsupported");
		}

		if (!vresult) // No previous result from refvals?
		{
			using std::to_string;

			ARCS_LOG_DEBUG <<
				"Process reference input as AccurateRip response for album";
			ARCS_LOG_DEBUG <<
				"Computed AccurateRip ID: "  << to_string(mine_arid);
			ARCS_LOG_DEBUG <<
				"Computed AccurateRip URL: " << mine_arid.url();

			const auto v =
				std::make_unique<AlbumVerifier>(checksums, mine_arid);
			vresult = v->perform(*ref_source);
		}

		print_filenames = !single_audio_file;
	} else
	{
		// No Offsets => No ToC => No ARId

		if (!vresult) // No result from refvals?
		{
			ARCS_LOG_DEBUG <<
				"Process reference input as AccurateRip response for tracks";

			const auto v = std::make_unique<TracksetVerifier>(checksums);
			vresult = v->perform(*ref_source);
		}

		if (Logging::instance().has_level(arcstk::LOGLEVEL::DEBUG))
		{
			using arcstk::checksum::type;
			log_matching_files(checksums, *vresult, 1, type::ARCS2);
		}
	}

	// Perform verification

	const auto best_b = vresult->best_block();

	namespace best_block = arcstk::best_block;

	if (vresult->all_tracks_verified())
	{
		ARCS_LOG_INFO << "Response contains a total match (v"
			<< best_block::checksumtype(best_b)
			<< ") to the input tracks in block "
			<< best_block::index(best_b);
	} else
	{
		ARCS_LOG_INFO << "Best match was block "
			<< best_block::index(best_b)
			//<< std::get<0>(best_b)
			<< " in response, having difference "
			<< best_block::difference(best_b);
			//<< std::get<2>(best_b);
	}

	if (config.is_set(VERIFY::NOOUTPUT)) // implies BOOLEAN
	{
		// 0 on accurate match, else > 0
		return { best_block::difference(best_b), nullptr };
		//return { std::get<2>(best_b), nullptr };
	}

	// Create result object

	const auto best_block_idx = config.is_set(VERIFY::PRINTALL) &&
		config.is_set(VERIFY::RESPONSEFILE) // FIXME blocks only with RFILE??
							? -1 // Won't be used
							: best_block::index(best_b);
							//: std::get<0>(best_b);

	const auto matching_checksum_type = best_block::checksumtype(best_b);

	auto filenames = std::vector<std::string> { };
	if (print_filenames)
	{
		if (config.no_arguments())
		{
			if (toc)
			{
				filenames = toc.filenames();
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
		types_to_print = std::vector<TYPE>{ matching_checksum_type };
	}

	// TODO Create formatter, then add types_to_print as print flags,
	// remove the dedicated vector
	auto result { create_formatter(config)->format(
		/* types to print */           types_to_print,
		/* verification results */     vresult.get(),
		/* optional best match */      best_block_idx,
		/* mine ARCSs */               checksums,
		/* optional mine ARId */       mine_arid,
		/* optional ToC */             toc,
		/* reference checksum source */ref_source.get(),
		/* input audio filenames */    filenames,
		/* optional URL prefix */      alt_prefix
	)};

	auto exit_code = config.is_set(VERIFY::BOOLEAN)
		? best_block::difference(best_b) // best difference
		//? std::get<2>(best_b) // best difference
		: EXIT_SUCCESS;

	return { exit_code, std::move(result) };
}


std::vector<arcstk::checksum::type> ARVerifyApplication::do_requested_types(
		const Configuration& /*config*/) const
{
	return { arcstk::checksum::type::ARCS1, arcstk::checksum::type::ARCS2 };
}

} // namespace v_1_0_0
} // namespace arcsapp

