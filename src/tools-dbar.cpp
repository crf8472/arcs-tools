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


// DBAROutputFormat


DBAROutputFormat::DBAROutputFormat()
	: block_counter_  { 0 }
	, track_counter_  { 0 }
	, arid_layout_    { std::make_unique<ARIdTableLayout>(false, true, false,
							false, false, false, false, false) }
	, triplet_layout_ { std::make_unique<TextDecoratedTripletLayout>() }
{
	// empty
}


std::ostringstream DBAROutputFormat::create_stream() const
{
	return std::ostringstream {/*default*/};
}


ARIdLayout* DBAROutputFormat::arid_layout()
{
	return arid_layout_.get();
}


DBARTripletLayout* DBAROutputFormat::triplet_layout()
{
	return triplet_layout_.get();
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


std::string DBAROutputFormat::triplet(const uint32_t arcs,
		const uint8_t confidence,
		const uint32_t frame450_arcs) const
{
	++track_counter_;

	return do_triplet(arcs, confidence, frame450_arcs);
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


// TextDecoratedTripletLayout


TextDecoratedTripletLayout::TextDecoratedTripletLayout()
	: width_arcs_     { 8 }
	, width_conf_     { 2 }
	, unparsed_value_ { "????????" }
	, with_label_     { true }
{
	// empty
}


void TextDecoratedTripletLayout::set_width_arcs(const int width)
{
	width_arcs_ = width;
}


int TextDecoratedTripletLayout::width_arcs() const
{
	return width_arcs_;
}


void TextDecoratedTripletLayout::set_width_conf(const int width)
{
	width_conf_ = width;
}


int TextDecoratedTripletLayout::width_conf() const
{
	return width_conf_;
}


void TextDecoratedTripletLayout::set_unparsed_value_symbol(const std::string& s)
{
	unparsed_value_ = s;
}


std::string TextDecoratedTripletLayout::set_unparsed_value_symbol() const
{
	return unparsed_value_;
}


void TextDecoratedTripletLayout::set_with_label(const bool flag)
{
	with_label_ = flag;
}


bool TextDecoratedTripletLayout::with_label() const
{
	return with_label_;
}


std::string TextDecoratedTripletLayout::do_format(InputTuple t) const
{
	using arcstk::Checksum;

	const auto track   = std::get<0>(t);
	const auto triplet = std::get<1>(t);

	auto out = std::ostringstream {};

	if (with_label())
	{
		const auto label = std::string { "Track" };

		out << label << " " << std::setw(2) << std::setfill('0') << track
			<< ": ";
	}

	const auto hex = calc::HexLayout {/*default*/};

	out << std::setw(width_arcs())
			<< hex.format(Checksum { triplet.arcs() }, width_arcs());

	out << " ";

	out << "(";
	out << std::setw(width_conf()) << std::setfill('0')
			<< triplet.confidence();
	out << ") ";

	out << std::setw(width_arcs())
			<< hex.format(Checksum { triplet.frame450_arcs() }, width_arcs());

	out << '\n';

	return out.str();
}


// TextDecoratedFormat


std::string TextDecoratedFormat::do_start_input() const
{
	return std::string {/*empty*/};
}


std::string TextDecoratedFormat::do_start_block() const
{
	auto ss { create_stream() };
	ss << "---------- Block " << std::dec << block_counter() << ": ";
	return ss.str();
}


std::string TextDecoratedFormat::do_header(const uint8_t track_count,
			const uint32_t id1,
			const uint32_t id2,
			const uint32_t cddb_id) const
{
	const auto id  = ARId { track_count, id1, id2, cddb_id };

	return arid_layout().format(id, std::string{/*no alt prefix*/});
}


std::string TextDecoratedFormat::do_triplet(const uint32_t arcs,
			const uint8_t confidence,
			const uint32_t frame450_arcs) const
{
	const auto triplet = DBARTriplet { arcs, confidence, frame450_arcs };

	return triplet_layout().format(track_counter(), triplet);
}


std::string TextDecoratedFormat::do_end_block() const
{
	return std::string {/*empty*/};
}


std::string TextDecoratedFormat::do_end_input() const
{
	auto ss { create_stream() };
	ss << "========== Parsed Blocks: " << std::dec << block_counter() << '\n';
	return ss.str();
}



// PrintParseHandler


PrintParseHandler::PrintParseHandler()
	: format_         { std::make_unique<TextDecoratedFormat>() }
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


void PrintParseHandler::do_triplet(const uint32_t arcs,
		const uint8_t confidence, const uint32_t frame450_arcs)
{
	this->print(format()->triplet(arcs, confidence, frame450_arcs));
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

