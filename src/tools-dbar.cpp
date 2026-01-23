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
#include <tuple>             // for get
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

} // namespace details


// DBAROutputFormat


DBAROutputFormat::DBAROutputFormat(std::unique_ptr<ARIdLayout> arid_layout,
			std::unique_ptr<DBARTripletLayout> triplet_layout)
	: block_counter_  { 0 }
	, track_counter_  { 0 }
	, arid_layout_    { std::move(arid_layout)    }
	, triplet_layout_ { std::move(triplet_layout) }
{
	// empty
}


DBAROutputFormat::DBAROutputFormat()
	: DBAROutputFormat(
		std::make_unique<ARIdTableLayout>(
			false, true, false, false, false, false, false, false),
		std::make_unique<TextDecoratedTripletLayout>())
{
	// empty
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


std::ostringstream DBAROutputFormat::create_stream() const
{
	return std::ostringstream {/*default*/};
}


ARIdLayout* DBAROutputFormat::arid_layout_ptr() const
{
	return arid_layout_.get();
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


DBARTripletLayout* DBAROutputFormat::triplet_layout_ptr() const
{
	return triplet_layout_.get();
}


std::string DBAROutputFormat::default_triplet(const uint32_t arcs,
			const uint8_t confidence,
			const uint32_t frame450_arcs) const
{
	const auto triplet = DBARTriplet { arcs, confidence, frame450_arcs };

	if (!triplet_layout_ptr())
	{
		using std::to_string;
		return to_string(triplet.arcs())
			+ ", " + to_string(triplet.confidence())
			+ ", " + to_string(triplet.frame450_arcs());
	}

	return triplet_layout().format(track_counter(), triplet);
}


std::string DBAROutputFormat::start_input() const
{
	block_counter_ = 0;

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


unsigned DBAROutputFormat::block_counter() const
{
	return block_counter_;
}


unsigned DBAROutputFormat::track_counter() const
{
	return track_counter_;
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


void DBAROutputFormat::set_triplet_layout(
		std::unique_ptr<DBARTripletLayout> format)
{
	triplet_layout_ = std::move(format);
}


const DBARTripletLayout& DBAROutputFormat::triplet_layout() const
{
	return *triplet_layout_;
}


const std::string& DBAROutputFormat::empty_string() const
{
	return EmptyString;
}


// TextDecoratedTripletLayout


TextDecoratedTripletLayout::TextDecoratedTripletLayout(
		const LabelStore<DBAR_TRIPLET_LABEL>::store_t labels,
		const flags_t properties)
	: LabelStore    { labels }
	, PropertyStore { properties }
{
	// empty
}


TextDecoratedTripletLayout::TextDecoratedTripletLayout()
	: TextDecoratedTripletLayout (
		{
			{ DBAR_TRIPLET_LABEL::TRIPLET,  "Track $TRACK" },
			{ DBAR_TRIPLET_LABEL::DELIM1,   ": " },
			{ DBAR_TRIPLET_LABEL::DELIM2,   " (" },
			{ DBAR_TRIPLET_LABEL::DELIM3,   ") " },
			{ DBAR_TRIPLET_LABEL::DELIM4,   "\n" },
			{ DBAR_TRIPLET_LABEL::UNPARSED, "????????" }
		},
		Flags::ALL_TRUE
	)
{
	// empty
}


int TextDecoratedTripletLayout::width_arcs() const
{
	return 8;
}


int TextDecoratedTripletLayout::width_conf() const
{
	return 2;
}


std::string TextDecoratedTripletLayout::do_format(InputTuple t) const
{
	const auto& track   = std::get<0>(t);
	const auto& triplet = std::get<1>(t);

	auto out = std::ostringstream {};

	// Print optional label for triplet on the left

	if (has_property(DBAR_TRIPLET_LABEL::TRIPLET))
	{
		out << details::evaluate(label(DBAR_TRIPLET_LABEL::TRIPLET), "$TRACK",
				track, 2, '0')
			<< label(DBAR_TRIPLET_LABEL::DELIM1);
	}

	// TODO configurable? However, do not create this on every call
	const auto hex   = calc::HexLayout {/*default*/};
	const auto width { width_arcs() };

	using arcstk::Checksum;

	// print a formatted ARCS
	const auto out_ = [&hex,&width,&out](const Checksum& c)
		{
			out << std::setw(width) << hex.format(c, width);
		};

	const auto delim = [&](const DBAR_TRIPLET_LABEL p)
		{
			out << (has_property(p) ? label(p) : " ");
		};


	out_(triplet.arcs());

	delim(DBAR_TRIPLET_LABEL::DELIM2);

	out << std::setw(width_conf()) << std::setfill('0') << triplet.confidence();

	delim(DBAR_TRIPLET_LABEL::DELIM3);

	out_(triplet.frame450_arcs());

	delim(DBAR_TRIPLET_LABEL::DELIM4);
	// FIXME Should not be part of TripletLayout

	return out.str();
}


// TextDecoratedFormat


TextDecoratedFormat::TextDecoratedFormat()
	: TextDecoratedFormat(
		{
			{ DBAR_LABEL::BLOCK,  "---------- Block $BLOCK" },
			{ DBAR_LABEL::DELIM1, ": " }
		},
		Flags::ALL_TRUE,
		std::make_unique<ARIdTableLayout>( /* print only ID */
			false, true, false, false, false, false, false, false),
		std::make_unique<TextDecoratedTripletLayout>(/* default */))
{
	// empty
}


TextDecoratedFormat::TextDecoratedFormat(const LabelStore::store_t labels,
		const flags_t properties, std::unique_ptr<ARIdLayout> arid_layout,
		std::unique_ptr<DBARTripletLayout> triplet_layout)
	: LabelStore       { labels }
	, PropertyStore    { properties }
	, DBAROutputFormat { std::move(arid_layout), std::move(triplet_layout) }
{
	// empty
}


std::string TextDecoratedFormat::do_start_input() const
{
	return empty_string();
}


std::string TextDecoratedFormat::do_start_block() const
{
	if (has_property(DBAR_LABEL::BLOCK))
	{
		return details::evaluate(label(DBAR_LABEL::BLOCK), "$BLOCK",
				block_counter(), 0/*no fixed width*/, ' ')
			+ label(DBAR_LABEL::DELIM1);
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

		if (has_property(DBAR_LABEL::DELIM2))
		{
			return to_string(id) + label(DBAR_LABEL::DELIM2);
		}

		return to_string(id) + " ";
	}

	return arid_layout().format(id, std::string{/*no alt prefix*/});
}


std::string TextDecoratedFormat::do_start_triplets() const
{
	return empty_string();
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


// YamlTripletLayout


std::string YamlTripletLayout::do_format(InputTuple t) const
{
	using arcstk::Checksum;

	//const auto track   = std::get<0>(t);
	const auto triplet = std::get<1>(t);

	using std::to_string;
	return "      - {arcs: " + to_string(Checksum { triplet.arcs() })
		+ ", confidence: "  + to_string(triplet.confidence())
		+ ", f450arcs: "    + to_string(Checksum { triplet.frame450_arcs() })
		+ "}\n";
}


// YamlFormat


YamlFormat::YamlFormat()
	: DBAROutputFormat(nullptr, std::make_unique<YamlTripletLayout>())
{
	// empty
}


std::string YamlFormat::do_start_input() const
{
	return "---\ndbar:\n";
}


std::string YamlFormat::do_start_block() const
{
	return "  - ";
}


std::string YamlFormat::do_header(const uint8_t track_count,
			const uint32_t id1,
			const uint32_t id2,
			const uint32_t cddb_id) const
{
	const auto id  = ARId { track_count, id1, id2, cddb_id };

	using std::to_string;
	return "id: " + to_string(id) + "\n    tracks:\n";
}


std::string YamlFormat::do_start_triplets() const
{
	return empty_string();
}


std::string YamlFormat::do_end_triplets() const
{
	return empty_string();
}


std::string YamlFormat::do_end_block() const
{
	return empty_string();
}


std::string YamlFormat::do_end_input() const
{
	return empty_string();
}


// JsonTripletLayout


std::string JsonTripletLayout::do_format(InputTuple t) const
{
	using arcstk::Checksum;

	//const auto track   = std::get<0>(t);
	const auto triplet = std::get<1>(t);

	using std::to_string;
	return "      { \"arcs\": \""   + to_string(Checksum { triplet.arcs() })
		+ "\", \"confidence\": \""  + to_string(triplet.confidence())
		+ "\", \"f450arcs\": \""    + to_string(Checksum { triplet.frame450_arcs() })
		+ "\" }";
}


// JsonFormat


JsonFormat::JsonFormat()
	: DBAROutputFormat(nullptr, std::make_unique<JsonTripletLayout>())
	, block_counter_   { 0 }
	, triplet_counter_ { 0 }
{
	// empty
}


std::string JsonFormat::do_start_input() const
{
	return "{\n\"dbar\": [\n";
}


std::string JsonFormat::do_start_block() const
{
	++block_counter_;

	static const auto start_block = std::string { "  {\n" };

	if (2 <= block_counter_)
	{
		return ",\n" + start_block;
	}

	return start_block;
}


std::string JsonFormat::do_header(const uint8_t track_count,
			const uint32_t id1,
			const uint32_t id2,
			const uint32_t cddb_id) const
{
	const auto id  = ARId { track_count, id1, id2, cddb_id };

	using std::to_string;
	return "    \"id\": \"" + to_string(id) + "\",\n";
}


std::string JsonFormat::do_start_triplets() const
{
	triplet_counter_ = 0;
	return "    \"tracks\": [\n";
}


std::string JsonFormat::do_triplet(const uint32_t arcs,
			const uint8_t confidence,
			const uint32_t frame450_arcs) const
{
	++triplet_counter_;

	const auto str = default_triplet(arcs, confidence, frame450_arcs);

	return (2 <= triplet_counter_ && !str.empty()) ? ",\n" + str : str;
}


std::string JsonFormat::do_end_triplets() const
{
	return "\n    ]\n";
}


std::string JsonFormat::do_end_block() const
{
	return "  }";
}


std::string JsonFormat::do_end_input() const
{
	return "\n]\n}\n";
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

