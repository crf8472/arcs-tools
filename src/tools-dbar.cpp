/**
 * \internal
 *
 * \file
 *
 * \brief Implements symbols from tools-dbar.hpp.
 */

#ifndef __ARCSTOOLS_TOOLS_DBAR_HPP__
#include "tools-dbar.hpp"
#endif

#include <cstdint>           // for uint32_t, uint8_t
#include <iomanip>           // for setw, setfill
#include <memory>            // for unique_ptr, make_unique
#include <sstream>           // for ostringstream
#include <string>            // for string
#include <utility>           // for move, swap

#ifndef __LIBARCSTK_IDENTIFIER_HPP__
#include <arcstk/identifier.hpp>     // for ARId
#endif
#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>      // for Checksum
#endif
#ifndef __LIBARCSTK_DBAR_HPP__
#include <arcstk/dbar.hpp>           // for DBARTriplet, parse_stream
#endif

#ifndef __ARCSTOOLS_APPLICATION_HPP__
#include "application.hpp"           // for Output
#endif
#ifndef __ARCSTOOLS_TOOLS_ARID_HPP__ // for ARIdLayout
#include "tools-arid.hpp"
#endif
#ifndef __ARCSTOOLS_TOOLS_CALC_HPP__
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
using arcstk::parse_stream;

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

using arcsapp::details::flag_operand;

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


std::string DBAROutputFormat::do_header(const uint8_t track_count,
			const uint32_t id1,
			const uint32_t id2,
			const uint32_t cddb_id) const
{
	return default_header(track_count, id1, id2, cddb_id);
}


std::string DBAROutputFormat::do_triplet(const uint32_t arcs,
			const uint8_t confidence,
			const uint32_t frame450_arcs) const
{
	return default_triplet(arcs, confidence, frame450_arcs);
}


std::string DBAROutputFormat::default_header(const uint8_t track_count,
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


std::string DBAROutputFormat::default_triplet(const uint32_t arcs,
			const uint8_t confidence,
			const uint32_t frame450_arcs) const
{
	const auto triplet = DBARTriplet { arcs, confidence, frame450_arcs };

	using std::to_string;
	return to_string(triplet.arcs())
			+ ", " + to_string(triplet.confidence())
			+ ", " + to_string(triplet.frame450_arcs());
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


// TextDecoratedFormat


TextDecoratedFormat::TextDecoratedFormat()
	: TextDecoratedFormat(
		{
			{ DBAR_TEXT::UNPARSED, "????????" },
			{ DBAR_TEXT::DELIM1,   ": "  },
			{ DBAR_TEXT::DELIM2,   ":\n" },
			{ DBAR_TEXT::DELIM3,   " ("  },
			{ DBAR_TEXT::DELIM4,   ") "  },
			{ DBAR_TEXT::DELIM5,   "\n"  },
			{ DBAR_TEXT::BLOCK,    "---------- Block $BLOCK" },
			{ DBAR_TEXT::TRIPLET,  "Track $TRACK" }
		},
		Flags::ALL_TRUE,
		std::make_unique<ARIdTableLayout>( /* print only ID */
			false, true, false, false, false, false, false, false))
{
	// empty
}


TextDecoratedFormat::TextDecoratedFormat(const LabelStore::store_t labels,
		const flags_t properties, std::unique_ptr<ARIdLayout> arid_layout)
	: LabelStore       { labels }
	, PropertyStore    { properties }
	, DBAROutputFormat { std::move(arid_layout) }
{
	// empty
}


std::string TextDecoratedFormat::do_start_input() const
{
	return empty_string();
}


std::string TextDecoratedFormat::do_start_block() const
{
	if (has_property(DBAR_TEXT::BLOCK))
	{
		return details::evaluate(label(DBAR_TEXT::BLOCK), "$BLOCK",
				block_counter(), 0/*no fixed width*/, ' ')
			+ label(DBAR_TEXT::DELIM1);
	}

	return empty_string();
}


std::string TextDecoratedFormat::do_header(const uint8_t track_count,
			const uint32_t id1,
			const uint32_t id2,
			const uint32_t cddb_id) const
{
	const auto id = ARId { track_count, id1, id2, cddb_id };

	if (!arid_layout_ptr())
	{
		using std::to_string;

		if (has_property(DBAR_TEXT::DELIM2))
		{
			return to_string(id) + label(DBAR_TEXT::DELIM2);
		}

		return to_string(id) + " ";
	}

	return arid_layout().format(id, empty_string()/*no alt prefix*/);
}


std::string TextDecoratedFormat::do_start_triplets() const
{
	return empty_string();
}


std::string TextDecoratedFormat::do_triplet(const uint32_t arcs,
			const uint8_t confidence,
			const uint32_t frame450_arcs) const
{
	const auto track = track_counter();
	const auto triplet = DBARTriplet { arcs, confidence, frame450_arcs };

	auto out = std::ostringstream {};

	// Print optional label for triplet on the left

	if (has_property(DBAR_TEXT::TRIPLET))
	{
		out << details::evaluate(label(DBAR_TEXT::TRIPLET), "$TRACK",
				track, 2, '0')
			<< label(DBAR_TEXT::DELIM1);
	}

	// TODO configurable? However, do not create this on every call
	const auto hex = calc::HexLayout {/*default*/};

	const auto width_arcs { 8 };
	const auto width_conf { 2 };

	using arcstk::Checksum;

	// print a formatted ARCS
	const auto out_ = [&hex,&width_arcs,&out](const Checksum& c)
		{
			out << std::setw(width_arcs) << hex.format(c, width_arcs);
		};

	const auto delim = [&](const DBAR_TEXT p)
		{
			out << (has_property(p) ? label(p) : " ");
		};


	out_(triplet.arcs());

	delim(DBAR_TEXT::DELIM3);

	out << std::setw(width_conf) << std::setfill('0') << triplet.confidence();

	delim(DBAR_TEXT::DELIM4);

	out_(triplet.frame450_arcs());

	delim(DBAR_TEXT::DELIM5);

	return out.str();
}


std::string TextDecoratedFormat::do_end_triplets() const
{
	return empty_string();
}


std::string TextDecoratedFormat::do_end_block() const
{
	return empty_string();
}


std::string TextDecoratedFormat::do_end_input() const
{
	// auto ss { create_stream() };
	// ss << "========== Parsed Blocks: " << std::dec << block_counter() << '\n';
	// return ss.str();
	return empty_string();
}


std::string TextDecoratedFormat::do_name() const
{
	return "text_decorated";
}


// LabelledDBAROutputFormat


LabelledDBAROutputFormat::LabelledDBAROutputFormat(
		const LabelStore<DBAR_LABEL>::store_t& labels,
		const LabelStore<DBAR_DELIM>::store_t& delims,
		const flags_t properties,
		std::unique_ptr<ARIdLayout> arid_layout)
	: DBAROutputFormat { std::move(arid_layout) }
	, PropertyStore    { properties }
	, labels_      { labels }
	, delims_      { delims }
	, indent_      { 0 }
	, indent_step_ { 2 }
{
	// empty
}


LabelledDBAROutputFormat::LabelledDBAROutputFormat(
		const LabelStore<DBAR_DELIM>::store_t& delims,
		const flags_t properties)
	: LabelledDBAROutputFormat {
		{
			{ DBAR_LABEL::DBAR,   "dbar"          },
			{ DBAR_LABEL::ID,     "id"            },
			{ DBAR_LABEL::TRACKS, "tracks"        },
			{ DBAR_LABEL::ARCS,   "arcs"          },
			{ DBAR_LABEL::CONF,   "conf"          },
			{ DBAR_LABEL::F450,   "frame450_arcs" }
		},
		delims,
		properties,
		std::make_unique<ARIdTableLayout>( /* print only ID */
			false, true, false, false, false, false, false, false)
	}
{
	// empty
}


std::string LabelledDBAROutputFormat::label(const DBAR_LABEL& label) const
{
	if (has_property(DBAR_DELIM::NAME_DELIM))
	{
		return delim(DBAR_DELIM::NAME_DELIM) + labels_.label(label)
			+ delim(DBAR_DELIM::NAME_DELIM);
	}

	return labels_.label(label);
}


std::string LabelledDBAROutputFormat::value(const std::string& s) const
{
	if (has_property(DBAR_DELIM::VAL_DELIM))
	{
		return delim(DBAR_DELIM::VAL_DELIM) + s + delim(DBAR_DELIM::VAL_DELIM);
	}

	return s;
}


std::string LabelledDBAROutputFormat::delim(const DBAR_DELIM& delim) const
{
	return delims_.label(delim);
}


std::string LabelledDBAROutputFormat::indent() const
{
	return std::string ( indent_, ' ' );
}


int LabelledDBAROutputFormat::inc_indent() const
{
	indent_ += indent_step_;
	return indent_;
}


int LabelledDBAROutputFormat::dec_indent() const
{
	indent_ -= indent_step_;
	return indent_;
}


std::string LabelledDBAROutputFormat::do_start_input() const
{
	const auto doc_start = std::string {
			delim(DBAR_DELIM::DOC_START) + "\n"
			+ label(DBAR_LABEL::DBAR) + delim(DBAR_DELIM::LABEL_DELIM)
	};

	if (has_property(DBAR_DELIM::DBAR_START))
	{
		return doc_start + delim(DBAR_DELIM::DBAR_START) + "\n";
	}

	return doc_start;
}


std::string LabelledDBAROutputFormat::do_start_block() const
{
	inc_indent();

	auto block_start = empty_string();

	if (has_property(DBAR_DELIM::BLOCK_DELIM) && 2 <= block_counter())
	{
		block_start += delim(DBAR_DELIM::BLOCK_DELIM) + "\n";
	}

	if (has_property(DBAR_DELIM::BLOCK_START))
	{
		block_start += indent() + delim(DBAR_DELIM::BLOCK_START);
	}

	inc_indent();

	return block_start;
}


std::string LabelledDBAROutputFormat::do_header(const uint8_t track_count,
			const uint32_t id1,
			const uint32_t id2,
			const uint32_t cddb_id) const
{
	const auto arid = ARId { track_count, id1, id2, cddb_id };

	using std::to_string;

	auto header = label(DBAR_LABEL::ID) + delim(DBAR_DELIM::LABEL_DELIM)
		+ value(to_string(arid));

	if (has_property(DBAR_DELIM::HEADER_START))
	{
		header = delim(DBAR_DELIM::HEADER_START) + header;
	}

	if (has_property(DBAR_DELIM::HEADER_END))
	{
		header += delim(DBAR_DELIM::HEADER_END);
	}

	return header + "\n";
}


std::string LabelledDBAROutputFormat::do_start_triplets() const
{
	auto tracks_start = indent() + label(DBAR_LABEL::TRACKS)
		+ delim(DBAR_DELIM::LABEL_DELIM);

	if (has_property(DBAR_DELIM::TRACKS_START))
	{
		tracks_start += delim(DBAR_DELIM::TRACKS_START);
	}

	inc_indent();

	return tracks_start;
}


std::string LabelledDBAROutputFormat::do_triplet(const uint32_t arcs,
			const uint8_t confidence,
			const uint32_t frame450_arcs) const
{
	const auto triplet = DBARTriplet { arcs, confidence, frame450_arcs };

	auto str = empty_string();

	if (has_property(DBAR_DELIM::TRACK_DELIM) && 2 <= track_counter())
	{
		str += delim(DBAR_DELIM::TRACK_DELIM);
	}

	str += "\n" + indent();

	if (has_property(DBAR_DELIM::TRACK_START))
	{
		str += delim(DBAR_DELIM::TRACK_START);
	}

	{
		using std::to_string;
		using arcstk::Checksum;

		str += label(DBAR_LABEL::ARCS) + delim(DBAR_DELIM::LABEL_DELIM)
			+ value(to_string(Checksum { triplet.arcs() }))
			+ delim(DBAR_DELIM::PROP_DELIM)
			+ label(DBAR_LABEL::CONF) + delim(DBAR_DELIM::LABEL_DELIM)
			+ value(to_string(triplet.confidence()))
			+ delim(DBAR_DELIM::PROP_DELIM)
			+ label(DBAR_LABEL::F450) + delim(DBAR_DELIM::LABEL_DELIM)
			+ value(to_string(Checksum { triplet.frame450_arcs() }));
	}

	if (has_property(DBAR_DELIM::TRACK_END))
	{
		str += delim(DBAR_DELIM::TRACK_END);
	}

	return str;
}


std::string LabelledDBAROutputFormat::do_end_triplets() const
{
	dec_indent();

	if (has_property(DBAR_DELIM::TRACKS_END))
	{
		return "\n" + indent() + delim(DBAR_DELIM::TRACKS_END) + "\n";
	}

	return empty_string();
}


std::string LabelledDBAROutputFormat::do_end_block() const
{
	dec_indent();

	auto block_end = empty_string();

	if (has_property(DBAR_DELIM::BLOCK_END))
	{
		block_end = indent() + delim(DBAR_DELIM::BLOCK_END);
	}

	if (not has_property(DBAR_DELIM::BLOCK_DELIM))
	{
		block_end += "\n";
	}

	dec_indent();

	return block_end;
}


std::string LabelledDBAROutputFormat::do_end_input() const
{
	auto doc_end = empty_string();

	if (has_property(DBAR_DELIM::BLOCK_DELIM))
	{
		doc_end += "\n";
	}

	if (has_property(DBAR_DELIM::DBAR_END))
	{
		doc_end += delim(DBAR_DELIM::DBAR_END) + "\n";
	}

	if (has_property(DBAR_DELIM::DOC_END))
	{
		doc_end += delim(DBAR_DELIM::DOC_END) + "\n";
	}

	return doc_end + "\n";
}


// YamlFormat


YamlFormat::YamlFormat()
	: LabelledDBAROutputFormat{
		{
			{ DBAR_DELIM::UNPARSED,     "????????" },
			{ DBAR_DELIM::DOC_START,    "---"  },
			//{ DBAR_DELIM::DOC_END,      ""   }, // not required
			{ DBAR_DELIM::DBAR_START,   ""     }, // enforce newline
			//{ DBAR_DELIM::DBAR_END,     ""   }, // not required
			{ DBAR_DELIM::BLOCK_START,  "- "   },
			//{ DBAR_DELIM::BLOCK_END,    ""   }, // not required
			//{ DBAR_DELIM::BLOCK_DELIM,  ""   }, // not required
			//{ DBAR_DELIM::HEADER_START, ""   }, // not required
			//{ DBAR_DELIM::HEADER_END,   ""   }, // not required
			//{ DBAR_DELIM::TRACKS_START, ""   }, // not required
			//{ DBAR_DELIM::TRACKS_END,   ""   }, // not required
			{ DBAR_DELIM::TRACK_START,  "- { " },
			{ DBAR_DELIM::TRACK_END,    " }"   },
			{ DBAR_DELIM::PROP_DELIM,   ", "   },
			//{ DBAR_DELIM::TRACK_DELIM,   ""   } // not required
			{ DBAR_DELIM::LABEL_DELIM,  ": "   },
			//{ DBAR_DELIM::NAME_DELIM,   ""   }, // not required
			{ DBAR_DELIM::VAL_DELIM,    "\""   }
		},
		Flags::ALL_FALSE | details::flag_operand(DBAR_DELIM::DOC_START,   true)
						 | details::flag_operand(DBAR_DELIM::DBAR_START,  true)
						 | details::flag_operand(DBAR_DELIM::BLOCK_START, true)
						 | details::flag_operand(DBAR_DELIM::TRACK_START, true)
						 | details::flag_operand(DBAR_DELIM::TRACK_END,   true)
						 | details::flag_operand(DBAR_DELIM::PROP_DELIM,  true)
						 | details::flag_operand(DBAR_DELIM::LABEL_DELIM, true)
						 | details::flag_operand(DBAR_DELIM::VAL_DELIM,   true)
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
			{ DBAR_DELIM::DOC_START,    "{"   },
			{ DBAR_DELIM::DOC_END,      "}"   },
			{ DBAR_DELIM::DBAR_START,   "["   },
			{ DBAR_DELIM::DBAR_END,     "]"   },
			{ DBAR_DELIM::BLOCK_START,  "{"   },
			{ DBAR_DELIM::BLOCK_END,    "}"   },
			{ DBAR_DELIM::BLOCK_DELIM,  ","   },
			{ DBAR_DELIM::HEADER_START, " "   },
			{ DBAR_DELIM::HEADER_END,   ","   },
			{ DBAR_DELIM::TRACKS_END,   "]"   },
			{ DBAR_DELIM::TRACKS_START, "["   },
			{ DBAR_DELIM::TRACKS_END,   "]"   },
			{ DBAR_DELIM::TRACK_START,  "{ "  },
			{ DBAR_DELIM::TRACK_END,    " }"  },
			{ DBAR_DELIM::PROP_DELIM,   ", "  },
			{ DBAR_DELIM::TRACK_DELIM,  ","   },
			{ DBAR_DELIM::LABEL_DELIM,  ": "  },
			{ DBAR_DELIM::NAME_DELIM,   "\""  },
			{ DBAR_DELIM::VAL_DELIM,    "\""  }
		},
		Flags::ALL_TRUE
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

