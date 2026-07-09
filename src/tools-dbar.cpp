/**
 * \internal
 *
 * \file
 *
 * \brief Implements symbols from tools-dbar.hpp.
 */

#include "layouts.hpp"
#ifndef ARCSTOOLS_TOOLS_DBAR_HPP_
#include "tools-dbar.hpp"
#endif

#include <cstdint>           // for uint32_t, uint8_t
#include <iomanip>           // for setw, setfill
#include <memory>            // for unique_ptr, make_unique
#include <sstream>           // for ostringstream
#include <string>            // for string
#include <utility>           // for move, swap

#ifndef LIBARCSTK_IDENTIFIER_HPP_
#include <arcstk/identifier.hpp>     // for ARId
#endif
#ifndef LIBARCSTK_CALCULATE_HPP_
#include <arcstk/calculate.hpp>      // for Checksum
#endif
#ifndef LIBARCSTK_DBAR_HPP_
#include <arcstk/dbar.hpp>           // for DBARTriplet, parse_stream
#endif

#ifndef ARCSTOOLS_APPLICATION_HPP_
#include "application.hpp"           // for Output
#endif
#ifndef ARCSTOOLS_TOOLS_ARID_HPP_    // for ARIdLayout
#include "tools-arid.hpp"
#endif
#ifndef ARCSTOOLS_TOOLS_CALC_HPP_
#include "tools-calc.hpp"            // for HexLayout
#endif


namespace arcsapp
{
inline namespace v_1_0_0
{
namespace dbar
{

// libarcstk
using arcstk::ARId;
using arcstk::DBARTriplet;

// arcsapp
using arcsapp::arid::ARIdTableLayout;


namespace details
{

/**
 * \brief Evaluate a string with placeholders.
 *
 * Replace placeholder \c ph in string \c s by a \c number, filled up to fixed
 * width \c width with character \c c.
 */
std::string evaluate(std::string s, const std::string& ph,
		const long unsigned number, const long unsigned width, const char c);

std::string evaluate(std::string s, const std::string& ph,
		const long unsigned number, const long unsigned width, const char c)
{
	if (auto pos = s.find(ph, 0); pos != std::string::npos)
	{
		using std::to_string;
		auto n_str = to_string(number);

		if (const auto len = n_str.length(); width > len)
		{
			n_str = std::string(width - len, c) + n_str;
		}

		s.replace(pos, ph.length(), n_str);
	}

	return s;
}

} // namespace details


// DBAROutputFormat


DBAROutputFormat::DBAROutputFormat(std::unique_ptr<ARIdLayout> arid_layout)
	: block_counter_  { 0 }
	, track_counter_  { 0 }
	, arid_layout_    { std::move(arid_layout)    }
{
	// empty
}


DBAROutputFormat::DBAROutputFormat()
	: DBAROutputFormat(
		std::make_unique<ARIdTableLayout>( /* print only ID */
			false, true, false, false, false, false, false, false))
{
	// empty
}


std::string DBAROutputFormat::start_input() const
{
	block_counter_ = 0;
	track_counter_ = 0;

	return do_start_input();
}


std::string DBAROutputFormat::start_block() const
{
	++block_counter_;
	track_counter_ = 0;

	return do_start_block();
}


std::string DBAROutputFormat::header(const uint8_t track_count,
		const uint32_t id1,
		const uint32_t id2,
		const uint32_t cddb_id) const
{
	return do_header(track_count, id1, id2, cddb_id);
}


std::string DBAROutputFormat::start_triplets() const
{
	return do_start_triplets();
}


std::string DBAROutputFormat::triplet(const uint32_t arcs,
		const uint8_t confidence,
		const uint32_t frame450_arcs) const
{
	++track_counter_;

	return do_triplet(arcs, confidence, frame450_arcs);
}


std::string DBAROutputFormat::end_triplets() const
{
	return do_end_triplets();
}


std::string DBAROutputFormat::end_block() const
{
	return do_end_block();
}


std::string DBAROutputFormat::end_input() const
{
	return do_end_input();
}


std::string DBAROutputFormat::name() const
{
	return do_name();
}


std::string DBAROutputFormat::default_id(const uint8_t track_count,
			const uint32_t id1,
			const uint32_t id2,
			const uint32_t cddb_id) const
{
	const auto id = ARId { track_count, id1, id2, cddb_id };

	if (!arid_layout_ptr())
	{
		using std::to_string;
		return to_string(id);
	}

	return arid_layout().format(id, std::string{/*no alt prefix*/});
}


std::string DBAROutputFormat::default_arcs(const uint32_t number) const
{
	using std::to_string;
	return to_string(arcstk::Checksum { number });
}


std::string DBAROutputFormat::default_confidence(const unsigned number) const
{
	using std::to_string;
	return to_string(number);
}


std::string DBAROutputFormat::default_f450_arcs(const uint32_t number) const
{
	return default_arcs(number);
}


unsigned DBAROutputFormat::block_counter() const
{
	return block_counter_;
}


unsigned DBAROutputFormat::track_counter() const
{
	return track_counter_;
}


std::ostringstream DBAROutputFormat::create_stream() const
{
	return std::ostringstream {/*default*/};
}


ARIdLayout* DBAROutputFormat::arid_layout_ptr() const
{
	return arid_layout_.get();
}


void DBAROutputFormat::set_arid_layout(
		std::unique_ptr<ARIdLayout> format)
{
	arid_layout_ = std::move(format);
}


const ARIdLayout& DBAROutputFormat::arid_layout() const
{
	return *arid_layout_;
}


const std::string& DBAROutputFormat::empty_string() const
{
	return EmptyString;
}


// DBARBaseFormat


DBARBaseFormat::DBARBaseFormat(const LabelStore::store_t& labels,
		const flags_t properties, std::unique_ptr<ARIdLayout> arid_layout,
		const std::string::size_type indent_start, const unsigned indent_step)
	: DBAROutputFormat { std::move(arid_layout) }
	, LabelStore       { labels }
	, PropertyStore    { properties }
	, indent_          { indent_start }
	, indent_step_     { indent_step }
{
	// empty
}


DBARBaseFormat::DBARBaseFormat(const LabelStore::store_t& labels,
		std::unique_ptr<ARIdLayout> arid_layout,
		const std::string::size_type indent_start, const unsigned indent_step)
	: DBARBaseFormat { labels, existing_flags(labels), std::move(arid_layout),
		indent_start, indent_step }
{
	// empty
}


DBARBaseFormat::DBARBaseFormat(const LabelStore::store_t& labels,
		const std::string::size_type indent_start, const unsigned indent_step)
	: DBARBaseFormat { labels, nullptr, indent_start, indent_step }
{
	// empty
}


DBARBaseFormat::DBARBaseFormat()
	: DBARBaseFormat {
		{/*empty*/}, // no labels
		nullptr, // no layout for ARId
		0 /* start with no indent */,
		2 /* indent by 2 chars */
	}
{
	// empty
}


std::string DBARBaseFormat::indent() const
{
	// NOLINTNEXTLINE(modernize-return-braced-init-list)
	return std::string ( indent_, ' ' );
}


int DBARBaseFormat::inc_indent() const
{
	indent_ += indent_step_;
	return indent_;
}


int DBARBaseFormat::dec_indent() const
{
	indent_ -= indent_step_;
	return indent_;
}


std::string DBARBaseFormat::do_start_input() const
{
	auto doc_start = empty_string();

	if (has_property(DBAR_DELIM::DOC_START))
	{
		doc_start += delim(DBAR_DELIM::DOC_START);
	}

	if (has_property(DBAR_DELIM::DBAR_START))
	{
		return doc_start + delim(DBAR_DELIM::DBAR_START);
	}

	return doc_start;
}


std::string DBARBaseFormat::do_end_input() const
{
	auto doc_end = empty_string();

	if (has_property(DBAR_DELIM::DBAR_END))
	{
		doc_end += details::evaluate(label(DBAR_DELIM::DBAR_END), "$BLOCKS",
						block_counter(), 0/*no fixed width*/, ' ');
	}

	if (has_property(DBAR_DELIM::DOC_END))
	{
		doc_end += delim(DBAR_DELIM::DOC_END);
	}

	return doc_end;
}


std::string DBARBaseFormat::do_start_block() const
{
	inc_indent();

	auto block_start = empty_string();

	// Add missing delimiter after previous block
	if (has_property(DBAR_DELIM::BLOCK_DELIM) && 2 <= block_counter())
	{
		block_start += delim(DBAR_DELIM::BLOCK_DELIM);
	}

	if (has_property(DBAR_DELIM::BLOCK_START))
	{
		block_start += indent() +
				details::evaluate(label(DBAR_DELIM::BLOCK_START), "$BLOCK",
						block_counter(), 0/*no fixed width*/, ' ');
	}

	inc_indent();

	return block_start;
}


std::string DBARBaseFormat::do_end_block() const
{
	dec_indent();

	auto block_end = empty_string();

	if (has_property(DBAR_DELIM::BLOCK_END))
	{
		block_end = indent() + delim(DBAR_DELIM::BLOCK_END);
	}

	dec_indent();

	return block_end;
}


std::string DBARBaseFormat::do_start_triplets() const
{
	auto tracks_start = empty_string();

	if (has_property(DBAR_DELIM::TRACKS_START))
	{
		tracks_start += indent() + delim(DBAR_DELIM::TRACKS_START);
	}

	inc_indent();

	return tracks_start;
}


std::string DBARBaseFormat::do_end_triplets() const
{
	dec_indent();

	if (has_property(DBAR_DELIM::TRACKS_END))
	{
		return delim(DBAR_DELIM::TRACKS_END);
	}

	return empty_string();
}


std::string DBARBaseFormat::do_header(const uint8_t track_count,
		const uint32_t id1,
		const uint32_t id2,
		const uint32_t cddb_id) const
{
	auto header = do_id(track_count, id1, id2, cddb_id);

	if (has_property(DBAR_DELIM::HEADER_START))
	{
		header = delim(DBAR_DELIM::HEADER_START) + header;
	}

	if (has_property(DBAR_DELIM::HEADER_END))
	{
		header += delim(DBAR_DELIM::HEADER_END);
	}

	return header;
}


std::string DBARBaseFormat::do_triplet(const uint32_t arcs,
		const uint8_t confidence,
		const uint32_t frame450_arcs) const
{
	auto ss = std::ostringstream {};

	// Add missing newline after previous track
	if (has_property(DBAR_DELIM::TRACK_DELIM) && 2 <= track_counter())
	{
		ss << delim(DBAR_DELIM::TRACK_DELIM);
	}

	ss << indent();

	if (has_property(DBAR_DELIM::TRACK_START))
	{
		ss << details::evaluate(label(DBAR_DELIM::TRACK_START), "$TRACK",
				track_counter(), 2, '0');
	}

	const auto triplet = DBARTriplet { arcs, confidence, frame450_arcs };

	ss << do_arcs(triplet.arcs());
	ss << delim(DBAR_DELIM::PROP_DELIM1);
	ss << do_confidence(triplet.confidence());
	ss << delim(DBAR_DELIM::PROP_DELIM2);
	ss << do_f450_arcs(triplet.frame450_arcs());

	if (has_property(DBAR_DELIM::TRACK_END))
	{
		ss << delim(DBAR_DELIM::TRACK_END);
	}

	return ss.str();
}


std::string DBARBaseFormat::do_id(const uint8_t track_count,
			const uint32_t id1,
			const uint32_t id2,
			const uint32_t cddb_id) const
{
	return default_id(track_count, id1, id2, cddb_id);
}


std::string DBARBaseFormat::do_arcs(const uint32_t number) const
{
	return default_arcs(number);
}


std::string DBARBaseFormat::do_confidence(const unsigned number) const
{
	return default_confidence(number);
}


std::string DBARBaseFormat::do_f450_arcs(const uint32_t number) const
{
	return default_arcs(number);
}


std::string DBARBaseFormat::do_delim(const DBAR_DELIM delim) const
{
	return label(delim);
}


std::string DBARBaseFormat::delim(const DBAR_DELIM delim) const
{
	return do_delim(delim);
}


// TextDecoratedFormat


TextDecoratedFormat::TextDecoratedFormat(const LabelStore::store_t& delims,
		const flags_t properties, std::unique_ptr<ARIdLayout> arid_layout)
	: DBARBaseFormat { delims, properties, std::move(arid_layout), 0, 0 }
{
	// empty
}


TextDecoratedFormat::TextDecoratedFormat(const LabelStore::store_t& delims,
		std::unique_ptr<ARIdLayout> arid_layout)
	: DBARBaseFormat { delims, std::move(arid_layout), 0, 0 }
{
	// empty
}


TextDecoratedFormat::TextDecoratedFormat(const LabelStore::store_t& delims)
	: TextDecoratedFormat { delims, nullptr }
{
	// empty
}


TextDecoratedFormat::TextDecoratedFormat()
	: TextDecoratedFormat {
		{   /* only text, no labels, no delimiters except space and newline */
			{ DBAR_DELIM::UNPARSED,     "????????" },
			{ DBAR_DELIM::BLOCK_END,    "\n" },
			{ DBAR_DELIM::HEADER_END,   "\n" },
			{ DBAR_DELIM::TRACK_DELIM,  "\n" },
			{ DBAR_DELIM::PROP_DELIM1,  " "  },
			{ DBAR_DELIM::PROP_DELIM2,  " "  },
		}
	}
{
	// empty
}


std::string TextDecoratedFormat::do_name() const
{
	return "text_decorated";
}


// LabelledDBAROutputFormat


LabelledDBAROutputFormat::LabelledDBAROutputFormat(
		const LabelStore<DBAR_ENTITY>::store_t& labels,
		const LabelStore<DBAR_DELIM>::store_t& delims,
		const flags_t properties,
		std::unique_ptr<ARIdLayout> arid_layout)
	: DBARBaseFormat { delims, properties, std::move(arid_layout), 0, 2 }
	, labels_        { labels }
{
	// empty
}


LabelledDBAROutputFormat::LabelledDBAROutputFormat(
		const LabelStore<DBAR_DELIM>::store_t& delims)
	: DBARBaseFormat { delims, 0, 2 }
	, labels_        {
		{
			/* default labels */
			{ DBAR_ENTITY::DBAR,   "dbar"          },
			{ DBAR_ENTITY::ID,     "id"            },
			{ DBAR_ENTITY::TRACKS, "tracks"        },
			{ DBAR_ENTITY::ARCS,   "arcs"          },
			{ DBAR_ENTITY::CONF,   "conf"          },
			{ DBAR_ENTITY::F450,   "frame450_arcs" }
		}
	}
{
	// empty
}


std::string LabelledDBAROutputFormat::do_id(const uint8_t track_count,
			const uint32_t id1,
			const uint32_t id2,
			const uint32_t cddb_id) const
{
	return label(DBAR_ENTITY::ID) + delim(DBAR_DELIM::LABEL_DELIM)
			+ value(default_id(track_count, id1, id2, cddb_id));
}


std::string LabelledDBAROutputFormat::do_arcs(const uint32_t number) const
{
	return label(DBAR_ENTITY::ARCS) + delim(DBAR_DELIM::LABEL_DELIM)
			+ value(default_arcs(number));
}


std::string LabelledDBAROutputFormat::do_confidence(const unsigned number) const
{
	return label(DBAR_ENTITY::CONF) + delim(DBAR_DELIM::LABEL_DELIM)
			+ value(default_confidence(number));
}


std::string LabelledDBAROutputFormat::do_f450_arcs(const uint32_t number) const
{
	return label(DBAR_ENTITY::F450) + delim(DBAR_DELIM::LABEL_DELIM)
			+ value(default_f450_arcs(number));
}


std::string LabelledDBAROutputFormat::do_delim(const DBAR_DELIM delim) const
{
	auto str = std::string {};

	// Intercept DBAR_START (precedes sequence of blocks), and TRACKS_START
	// (precedes sequence of triplets). Both are labelled but not handled by
	// a virtual member function (as do_id() e.g.) that could add the label.
	switch (delim)
	{
		case DBAR_DELIM::DBAR_START: // sequence of blocks
			str += label(DBAR_ENTITY::DBAR)
					+ DBARBaseFormat::label(DBAR_DELIM::LABEL_DELIM);
			break;
		case DBAR_DELIM::TRACKS_START: // sequence of tracks
			str += label(DBAR_ENTITY::TRACKS)
					+ DBARBaseFormat::label(DBAR_DELIM::LABEL_DELIM);
			break;
		default:
			;
	}

	return str + DBARBaseFormat::label(delim);
}


std::string LabelledDBAROutputFormat::do_label(const DBAR_ENTITY& entity) const
{
	if (has_property(DBAR_DELIM::NAME_DELIM))
	{
		return delim(DBAR_DELIM::NAME_DELIM) + labels_.label(entity)
			+ delim(DBAR_DELIM::NAME_DELIM);
	}

	return labels_.label(entity);
}


std::string LabelledDBAROutputFormat::do_value(const std::string& s) const
{
	if (has_property(DBAR_DELIM::VAL_DELIM))
	{
		return delim(DBAR_DELIM::VAL_DELIM) + s + delim(DBAR_DELIM::VAL_DELIM);
	}

	return s;
}


std::string LabelledDBAROutputFormat::label(const DBAR_ENTITY& entity) const
{
	return do_label(entity);
}


std::string LabelledDBAROutputFormat::value(const std::string& s) const
{
	return do_value(s);
}


// YamlFormat


YamlFormat::YamlFormat()
	: LabelledDBAROutputFormat{
		{
			{ DBAR_DELIM::UNPARSED,     "????????" },
			{ DBAR_DELIM::DOC_START,    "---\n" },
			{ DBAR_DELIM::DOC_END,      "\n"    },
			{ DBAR_DELIM::DBAR_START,   "\n"    },
			{ DBAR_DELIM::BLOCK_START,  "- "    },
			{ DBAR_DELIM::BLOCK_DELIM,  "\n"    },
			{ DBAR_DELIM::HEADER_END,   "\n"    },
			{ DBAR_DELIM::TRACKS_START, "\n"    },
			{ DBAR_DELIM::TRACK_START,  "- { "  },
			{ DBAR_DELIM::TRACK_END,    " }"    },
			{ DBAR_DELIM::PROP_DELIM1,  ", "    },
			{ DBAR_DELIM::PROP_DELIM2,  ", "    },
			{ DBAR_DELIM::TRACK_DELIM,  "\n"    },
			{ DBAR_DELIM::LABEL_DELIM,  ": "    },
			{ DBAR_DELIM::VAL_DELIM,    "\""    }
		}
	}
{
	// empty
}


std::string YamlFormat::do_name() const
{
	return "yaml";
}


// JsonFormat


JsonFormat::JsonFormat()
	: LabelledDBAROutputFormat{
		{
			{ DBAR_DELIM::UNPARSED,     "????????" },
			{ DBAR_DELIM::DOC_START,    "{\n" },
			{ DBAR_DELIM::DOC_END,      "}\n" },
			{ DBAR_DELIM::DBAR_START,   "[\n" },
			{ DBAR_DELIM::DBAR_END,     "]\n" },
			{ DBAR_DELIM::BLOCK_START,  "{"   },
			{ DBAR_DELIM::BLOCK_END,    "}"   },
			{ DBAR_DELIM::BLOCK_DELIM,  ",\n" },
			{ DBAR_DELIM::HEADER_START, " "   },
			{ DBAR_DELIM::HEADER_END,   ",\n" },
			{ DBAR_DELIM::TRACKS_START, "[\n" },
			{ DBAR_DELIM::TRACKS_END,   "]\n" },
			{ DBAR_DELIM::TRACK_START,  "{ "  },
			{ DBAR_DELIM::TRACK_END,    " }"  },
			{ DBAR_DELIM::PROP_DELIM1,  ", "  },
			{ DBAR_DELIM::PROP_DELIM2,  ", "  },
			{ DBAR_DELIM::TRACK_DELIM,  ",\n" },
			{ DBAR_DELIM::LABEL_DELIM,  ": "  },
			{ DBAR_DELIM::NAME_DELIM,   "\""  },
			{ DBAR_DELIM::VAL_DELIM,    "\""  }
		}
	}
{
	// empty
}


std::string JsonFormat::do_name() const
{
	return "json";
}


// PrintParseHandler


PrintParseHandler::PrintParseHandler()
	: format_ { std::make_unique<TextDecoratedFormat>() }
{
	// empty
}


PrintParseHandler::~PrintParseHandler() noexcept
= default;


void PrintParseHandler::set_format(std::unique_ptr<DBAROutputFormat> format)
{
	format_ = std::move(format);
}


const DBAROutputFormat& PrintParseHandler::format() const
{
	return *format_;
}


DBAROutputFormat* PrintParseHandler::format()
{
	return format_.get();
}


void PrintParseHandler::print(const std::string& str) const
{
	if (!str.empty())
	{
		Output::instance().output(str);
	}
}


void PrintParseHandler::do_start_input()
{
	this->print(format()->start_input());
}


void PrintParseHandler::do_start_block()
{
	this->print(format()->start_block());
}


void PrintParseHandler::do_header(const uint8_t track_count,
		const uint32_t disc_id1, const uint32_t disc_id2,
		const uint32_t cddb_id)
{
	this->print(format()->header(track_count, disc_id1, disc_id2, cddb_id));
}


void PrintParseHandler::do_start_triplets()
{
	this->print(format()->start_triplets());
}


void PrintParseHandler::do_triplet(const uint32_t arcs,
		const uint8_t confidence, const uint32_t frame450_arcs)
{
	this->print(format()->triplet(arcs, confidence, frame450_arcs));
}


void PrintParseHandler::do_end_triplets()
{
	this->print(format()->end_triplets());
}


void PrintParseHandler::do_end_block()
{
	this->print(format()->end_block());
}


void PrintParseHandler::do_end_input()
{
	this->print(format()->end_input());
}


} // namespace dbar
} // namespace v_1_0_0
} // namespace arcsapp

