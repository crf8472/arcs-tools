#ifndef __ARCSTOOLS_APPARVERIFY_HPP__
#include "app-verify.hpp"
#endif

#include <algorithm>       // for replace, max
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
#include <type_traits>     // for add_const<>::type
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


void ARVerifyConfigurator::flush_local_options(OptionRegistry &r) const
{
	using std::end;
	r.insert(end(r),
	{
		// from FORMATBASE

		{ VERIFY::READERID ,
		{  "reader", true, "auto",
			"Force use of audio reader with specified id" }},

		{ VERIFY::PARSERID ,
		{  "parser", true, "auto",
			"Force use of toc parser with specified id" }},

		{ VERIFY::LIST_TOC_FORMATS ,
		{  "list-toc-formats", false, "FALSE",
			"List all supported file formats for TOC metadata" }},

		{ VERIFY::LIST_AUDIO_FORMATS ,
		{  "list-audio-formats", false, "FALSE",
			"List all supported audio codec/container formats" }},

		// from CALCBASE

		{ VERIFY::METAFILE ,
		{  'm', "metafile", true, "none",
			"Specify metadata file (TOC) to use" }},

		{ VERIFY::NOTRACKS ,
		{  "no-track-nos", false, "FALSE", "Do not print track numbers" }},

		{ VERIFY::NOFILENAMES ,
		{  "no-filenames", false, "FALSE",
			"Do not print the filenames" }},

		{ VERIFY::NOOFFSETS ,
		{  "no-offsets", false, "FALSE", "Do not print track offsets" }},

		{ VERIFY::NOLENGTHS ,
		{  "no-lengths", false, "FALSE", "Do not print track lengths" }},

		{ VERIFY::NOLABELS ,
		{  "no-labels", false, "FALSE", "Do not print column or row labels" }},

		{ VERIFY::COLDELIM ,
		{  "col-delim", true, "ASCII-32", "Specify column delimiter" }},

		{ VERIFY::PRINTID ,
		{  "print-id", false, "FALSE",
			"Print the AccurateRip Id of the album" }},

		{ VERIFY::PRINTURL ,
		{  "print-url", false, "FALSE",
			"Print the AccurateRip URL of the album" }},

		// from VERIFY

		{ VERIFY::NOFIRST ,
		{  "no-first", false, "FALSE",
			"Do not treat first audio file as first track" }},

		{ VERIFY::NOLAST ,
		{  "no-last", false, "FALSE",
			"Do not treat last audio file as last track" }},

		{ VERIFY::NOALBUM ,
		{  "no-album", false, "FALSE",
			"Abbreviates \"--no-first --no-last\"" }},

		{ VERIFY::RESPONSEFILE ,
		{  'r', "response", true, "none",
			"Specify AccurateRip response file" }},

		{ VERIFY::REFVALUES ,
		{  "refvalues", true, "none",
			"Specify AccurateRip reference values (as hex value list)" }},

		{ VERIFY::PRINTALL ,
		{  "print-all-matches", false, "FALSE",
			"Print verification results for all blocks" }},

		{ VERIFY::BOOLEAN ,
		{  'b', "boolean", false, "FALSE",
			"Return number of differing tracks in best match" }},

		{ VERIFY::NOOUTPUT ,
		{  'n', "no-output", false, "FALSE",
			"Do not print the result (implies --boolean)" }},

		{ VERIFY::COLORED ,
		{  "colors", false, "FALSE",
			"Use colored output" }}
	});
}


std::unique_ptr<Options> ARVerifyConfigurator::do_configure_options(
		std::unique_ptr<Options> options) const
{
	auto voptions = configure_calcbase_options(std::move(options));
	auto no_album_options = std::string {};

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

	// Print ID only in case we have one from the reference data

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
	}

	if (voptions->is_set(VERIFY::RESPONSEFILE)
			and voptions->is_set(VERIFY::REFVALUES))
	{
		throw ConfigurationException("Cannot process --refvalues along with "
				" -r/--response, only one of theses options is allowed");
	}

	// NOOUTPUT implies BOOLEAN

	if (voptions->is_set(VERIFY::NOOUTPUT))
	{
		voptions->set(VERIFY::BOOLEAN);
	}

	return voptions;
}


// MatchDecorator


MatchDecorator::MatchDecorator(const std::size_t n)
	: CellDecorator(n)
{
	/* empty */
}


MatchDecorator::MatchDecorator(const MatchDecorator& rhs)
	: CellDecorator(rhs)
{
	/* empty */
}


std::string MatchDecorator::do_decorate_set(std::string&& s) const
{
	using ansi::Color;
	using ansi::Colorize;

	return Colorize<Color::FG_GREEN>{}(s);
}


std::string MatchDecorator::do_decorate_unset(std::string&& s) const
{
	using ansi::Color;
	using ansi::Colorize;

	return Colorize<Color::FG_RED>{}(s);
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
	const auto& checksums = std::get<0>(t);
	const auto  toc       = std::get<5>(t);
	const auto& arid      = std::get<6>(t);
	const auto& filenames = std::get<8>(t);

	validate(checksums, toc, arid, filenames);

	// Specific for verify

	const auto& response  = std::get<1>(t);
	const auto& refsums   = std::get<2>(t);
	const auto& match     = std::get<3>(t);
	const auto  block     = std::get<4>(t);

	if (refsums.empty() && !response->size())
	{
		throw std::invalid_argument("Missing reference checksums, "
				"nothing to print.");
	}

	if (!refsums.empty() && refsums.size() != checksums.size())
	{
		throw std::invalid_argument("Mismatch: "
				"Reference for " + std::to_string(refsums.size())
				+ " tracks, but Checksums specify "
				+ std::to_string(checksums.size()) + " tracks.");
	}

	if (!match)
	{
		throw std::invalid_argument("Missing match information, "
				"nothing to print.");
	}

	//if (block < 0)
	//{
	//	throw std::invalid_argument(
	//		"Index of matching checksum block is negative, nothing to print.");
	//}

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
	const auto& checksums = std::get<0>(t);
	const auto  response  = std::get<1>(t);
	const auto& refvalues = std::get<2>(t);
	const auto  match     = std::get<3>(t);
	const auto  block     = std::get<4>(t);
	const auto  toc       = std::get<5>(t);
	const auto  arid      = std::get<6>(t);
	const auto& altprefix = std::get<7>(t);
	const auto& filenames = std::get<8>(t);

	auto result = std::make_unique<ResultList>();

	// If an ARResponse is used for the references with a block (not PRINTALL)
	if (response != nullptr && response->size() > 0 && block > -1)
	{
		// Use ARId of specified block for "Theirs" ARId
		result->append(std::make_unique<ResultObject<RichARId>>(
				build_id(toc, response->at(block).id(), altprefix)));
	}

	result->append(build_result(checksums, response, &refvalues, match,
			block, toc, arid, altprefix, filenames, types_to_print()));

	return result;
}


std::vector<ATTR> VerifyResultFormatter::do_create_attributes(
		const bool p_tracks, const bool p_offsets, const bool p_lengths,
		const bool p_filenames,
		const std::vector<arcstk::checksum::type>& types_to_print,
		const int total_theirs) const
{
	const auto total_fields = p_tracks + p_offsets + p_lengths + p_filenames
		+ types_to_print.size() + total_theirs;

	using checksum = arcstk::checksum::type;

	std::vector<ATTR> fields;
	fields.reserve(total_fields);
	if (p_tracks)    { fields.emplace_back(ATTR::TRACK);    }
	if (p_filenames) { fields.emplace_back(ATTR::FILENAME); }
	if (p_offsets)   { fields.emplace_back(ATTR::OFFSET);   }
	if (p_lengths)   { fields.emplace_back(ATTR::LENGTH);   }

	for (const auto& t : types_to_print)
	{
		if (checksum::ARCS1 == t)
		{
			fields.emplace_back(ATTR::CHECKSUM_ARCS1);
		} else
		if (checksum::ARCS2 == t)
		{
			fields.emplace_back(ATTR::CHECKSUM_ARCS2);
		}
	}

	for (auto i = int { 0 }; i < total_theirs; ++i)
	{
		fields.emplace_back(ATTR::THEIRS);
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


// ColorizingVerifyResultFormatter


void ColorizingVerifyResultFormatter::init_composer(TableComposer* c) const
{
	// Register Decorators to each "Theirs" field

	int i = 0;
	for (const auto& field : c->fields())
	{
		if (ATTR::THEIRS == field)
		{
			ARCS_LOG(DEBUG1) << "Register MatchDecorator to field index " << i;

			c->register_to_field(i,
				std::make_unique<MatchDecorator>(c->total_records()));
		}

		++i;
	}
}


void ColorizingVerifyResultFormatter::do_their_match(const Checksum& checksum,
		const int record_idx, const int field_idx, TableComposer* c) const
{
	c->set_field(record_idx, field_idx, this->checksum(checksum));

	ARCS_LOG(DEBUG1) << "Mark cell " << record_idx << ", " << field_idx
		<< "as match-decorated";

	c->mark(record_idx, field_idx);
}


void ColorizingVerifyResultFormatter::do_their_mismatch(
		const Checksum& checksum, const int record_idx, const int field_idx,
		TableComposer* c) const
{
	c->set_field(record_idx, field_idx, this->checksum(checksum));

	ARCS_LOG(DEBUG1) << "Mark cell " << record_idx << ", " << field_idx
		<< "as mismatch-decorated";
}


// ARVerifyApplication


std::unique_ptr<VerifyResultFormatter> ARVerifyApplication::configure_layout(
		const Options &options,
		const std::vector<arcstk::checksum::type> &types, const Match &match)
		const
{
	auto fmt = std::unique_ptr<VerifyResultFormatter>();

	if (options.is_set(VERIFY::COLORED))
	{
		fmt = std::make_unique<ColorizingVerifyResultFormatter>();
	} else
	{
		fmt = std::make_unique<MonochromeVerifyResultFormatter>();
	}

	fmt->set_types_to_print(types);

	// Layouts for Checksums + ARId

	fmt->set_checksum_layout(std::make_unique<HexLayout>());

	// Layout for ARId

	if (options.is_set(VERIFY::PRINTID) || options.is_set(VERIFY::PRINTURL))
	{
		std::unique_ptr<ARIdLayout> id_layout =
			std::make_unique<ARIdTableLayout>(
				!options.is_set(VERIFY::NOLABELS),
				options.is_set(VERIFY::PRINTID),
				options.is_set(VERIFY::PRINTURL),
				false, /* no filenames */
				false, /* no tracks */
				false, /* no id 1 */
				false, /* no id 2 */
				false  /* no cddb id */
		);

		fmt->set_arid_layout(std::move(id_layout));
	}

	// Print labels or not
	fmt->set_label(!options.is_set(VERIFY::NOLABELS));

	// TOC present? Helper for determining other properties
	const bool has_toc = !options.value(VERIFY::METAFILE).empty();

	// Print track numbers if they are not forbidden and a TOC is present
	fmt->set_track(options.is_set(VERIFY::NOTRACKS) ? false : has_toc);

	// Print offsets if they are not forbidden and a TOC is present
	fmt->set_offset(options.is_set(VERIFY::NOOFFSETS) ? false : has_toc);

	// Print lengths if they are not forbidden
	fmt->set_length(!options.is_set(VERIFY::NOLENGTHS));

	// Print filenames if they are not forbidden and a TOC is _not_ present
	fmt->set_filename(!options.is_set(VERIFY::NOFILENAMES) || !has_toc);

	// Indicate a matching checksum by this symbol
	fmt->set_match_symbol("==");

	// Method for creating the result table
	fmt->set_builder_creator(std::make_unique<RowTableComposerBuilder>());

	auto layout { std::make_unique<StringTableLayout>() };

	// Set inner column delimiter
	layout->set_col_inner_delim(options.is_set(VERIFY::COLDELIM)
		? options.value(VERIFY::COLDELIM)
		: " ");

	fmt->set_table_layout(std::move(layout));

	return fmt;
}


std::tuple<ARResponse, std::vector<Checksum>>
	ARVerifyApplication::get_reference_checksums(const Options &options) const
{
	auto response = ARResponse {};
	auto refsums  = std::vector<Checksum> {};

	if (options.is_set(VERIFY::REFVALUES))
	{
		// use referene checksums passed on command line
		refsums = parse_refvalues(options);
	} else
	{
		// stdin or accuraterip .bin-file
		response = parse_response(options);
	}

	return std::make_tuple(response, refsums);
}


ARResponse ARVerifyApplication::parse_response(const Options &options) const
{
	// Parse the AccurateRip response

	std::unique_ptr<ARStreamParser> parser;

	std::string responsefile { options.value(VERIFY::RESPONSEFILE) };

	if (responsefile.empty())
	{
		ARCS_LOG_DEBUG << "Parse response from stdin";

		parser = std::make_unique<ARStdinParser>();

	} else
	{
		ARCS_LOG_DEBUG << "Parse response from file " << responsefile;

		parser = std::make_unique<ARFileParser>(responsefile);
	}

	ARResponse response;
	auto c_handler { std::make_unique<DefaultContentHandler>() };
	c_handler->set_object(response);
	auto e_handler { std::make_unique<DefaultErrorHandler>() };

	parser->set_content_handler(std::move(c_handler));
	parser->set_error_handler(std::move(e_handler));

	try
	{
		if (parser->parse() == 0)
		{
			this->fatal_error("No bytes parsed, exit");
		}
	} catch (const std::exception& e)
	{
		this->fatal_error(e.what());
	}

	ARCS_LOG_DEBUG << "Response object created";

	return response;
}


std::vector<Checksum> ARVerifyApplication::parse_refvalues(
		const Options &options) const
{
	auto refvals = parse_refvalues_sequence(options.value(VERIFY::REFVALUES));

	// Log the parsing result

	if (Logging::instance().has_level(arcstk::LOGLEVEL::DEBUG))
	{
		std::ostringstream outlist;
		for (const auto& v : refvals) { outlist << v << " "; }
		ARCS_LOG_DEBUG << "Option --refvals was passed the following values: "
				<< outlist.str();
	}

	return refvals;
}


std::vector<Checksum> ARVerifyApplication::parse_refvalues_sequence(
		const std::string &input) const
{
	if (input.empty())
	{
		return {};
	}

	auto in { input };
	{
		using std::begin;
		using std::end;
		const char delim = ',';
		std::replace(begin(in), end(in), delim, ' '); // erase commas
	}

	auto refvals = std::istringstream { in };
	auto value   = std::string {};
	auto refsum  = uint32_t { 0 };
	auto refsums = std::vector<Checksum> {};
	auto t       = int { 1 };

	while (refvals >> value)
	{
		refsum = std::stoul(value, 0, 16);

		ARCS_LOG_DEBUG << "Input reference sum " << value
			<< " for track/file " << std::setw(2) << t
			<< " is parsed as " << refsum;

		refsums.push_back(Checksum { refsum });
		value.clear();
		++t;
	}

	return refsums;
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


auto ARVerifyApplication::run_calculation(const Options &options) const
	-> std::pair<int, std::unique_ptr<Result>>
{
	// Parse reference ARCSs from AccurateRip

	const auto ref_respns = options.is_set(VERIFY::REFVALUES)
		? ARResponse { /* empty */ }
		: parse_response(options);

	const auto ref_values = options.is_set(VERIFY::REFVALUES)
		? parse_refvalues(options)
		: std::vector<Checksum> { /* empty */ };

	if (ref_values.empty() && !ref_respns.size())
	{
		throw std::runtime_error(
				"No reference checksums for matching available.");
	}

	// Configure selections (e.g. --reader and --parser)

	const calc::IdSelection id_selection;

	auto audio_selection = options.is_set(VERIFY::READERID)
		? id_selection(options.value(VERIFY::READERID))
		: nullptr;

	auto toc_selection = options.is_set(VERIFY::PARSERID)
		? id_selection(options.value(VERIFY::PARSERID))
		: nullptr;

	// If no selections are assigned, the libarcsdec default selections
	// will be used.

	// Calculate the actual ARCSs from input files

	auto [ checksums, arid, toc ] = ARCalcApplication::calculate(
			options.value(VERIFY::METAFILE),
			options.arguments(),
			!options.is_set(VERIFY::NOFIRST),
			!options.is_set(VERIFY::NOLAST),
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

	if (options.is_set(VERIFY::REFVALUES))
	{
		diff = std::make_unique<ListMatcher>(checksums, ref_values);
	}

	bool print_filenames = true;

	if (/* Album requested? */not options.is_set(VERIFY::NOALBUM))
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

	if (options.is_set(VERIFY::NOOUTPUT)) // implies BOOLEAN
	{
		// 0 on accurate match, else > 0
		return { diff->best_difference(), nullptr };
	}

	// Create result object

	const auto best_block = options.is_set(VERIFY::PRINTALL) &&
		options.is_set(VERIFY::RESPONSEFILE)
							? -1 // Won't be used
							: diff->best_match();

	const auto matching_version = diff->best_match_is_v2();

	const auto alt_prefix = std::string {/* TODO Alt-Prefix */};

	auto filenames = std::vector<std::string> { };
	if (print_filenames)
	{
		if (options.no_arguments())
		{
			if (toc)
			{
				filenames = arcstk::toc::get_filenames(*toc);
			}
		} else
		{
			filenames = options.arguments();
		}
	}

	// TODO Compose set of types to be printed
	// If all types are requested, insert all types, otherwise insert the
	// matching only
	using TYPE = arcstk::checksum::type;
	std::vector<TYPE> types_to_print;

	if (options.is_set(VERIFY::PRINTALL))
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

	const auto f { configure_layout(options, types_to_print, *match) };

	auto result {
		f->format(checksums, &ref_respns, ref_values,
			match, best_block, toc.get(), arid, alt_prefix, filenames)
	};

	auto exit_code = options.is_set(VERIFY::BOOLEAN)
		? diff->best_difference()
		: EXIT_SUCCESS;

	return { exit_code, std::move(result) };
}

} // namespace arcsapp

