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


// PrintParseHandler


PrintParseHandler::PrintParseHandler()
	: block_counter_  { 0 }
	, track_          { 0 }
	, arid_layout_    { std::make_unique<ARIdTableLayout>(false, false, false,
							false, false, false, false, false) }
	, triplet_layout_ { std::make_unique<DBARTripletLayout>() }
{
	// empty
}


PrintParseHandler::~PrintParseHandler() noexcept
= default;


void PrintParseHandler::set_arid_layout(
		std::unique_ptr<ARIdLayout> format)
{
	arid_layout_ = std::move(format);
}


const ARIdLayout& PrintParseHandler::arid_layout() const
{
	return *arid_layout_;
}


void PrintParseHandler::set_triplet_layout(
		std::unique_ptr<DBARTripletLayout> format)
{
	triplet_layout_ = std::move(format);
}


void PrintParseHandler::print(const std::string& str) const
{
	Output::instance().output(str);
}


const DBARTripletLayout& PrintParseHandler::triplet_layout() const
{
	return *triplet_layout_;
}


ARIdLayout* PrintParseHandler::arid_layout()
{
	return arid_layout_.get();
}


DBARTripletLayout* PrintParseHandler::triplet_layout()
{
	return triplet_layout_.get();
}


void PrintParseHandler::do_start_input()
{
	// empty
}


void PrintParseHandler::do_start_block()
{
	++block_counter_;

	std::ostringstream ss;
	ss << "---------- Block " << std::dec << block_counter_ << " : ";
	this->print(ss.str());
}


void PrintParseHandler::do_header(const uint8_t track_count,
		const uint32_t disc_id1, const uint32_t disc_id2,
		const uint32_t cddb_id)
{
	ARId id(track_count, disc_id1, disc_id2, cddb_id);

	auto str = arid_layout()->format(id, std::string{});
	str += '\n';
	this->print(str);
}


void PrintParseHandler::do_triplet(const uint32_t arcs,
		const uint8_t confidence, const uint32_t frame450_arcs)
{
	++track_;
	const DBARTriplet triplet(arcs, confidence, frame450_arcs);

	auto str = triplet_layout()->format(track_, triplet);
	this->print(str);
}


void PrintParseHandler::do_end_block()
{
	track_ = 0;
}


void PrintParseHandler::do_end_input()
{
	std::ostringstream ss;
	ss << "========== Blocks: " << std::dec << block_counter_ << '\n';
	this->print(ss.str());
}


// DBARTripletLayout


std::string DBARTripletLayout::do_format(InputTuple t) const
{
	using arcstk::Checksum;

	const auto track   = std::get<0>(t);
	const auto triplet = std::get<1>(t);

	auto hex = calc::HexLayout {};

	const int width_arcs = 8;
	const int width_conf = 2;

	const auto unparsed_value = std::string { "????????" };

	std::ostringstream out;

	// TODO Make label configurable
	out << "Track " << std::setw(2) << std::setfill('0') << track << ": ";

	out << std::setw(width_arcs)
			<< hex.format(Checksum { triplet.arcs() }, width_arcs);

	out << " ";

	out << "(";
	out << std::setw(width_conf) << std::setfill('0')
			<< triplet.confidence();
	out << ") ";

	out << std::setw(width_arcs)
			<< hex.format(Checksum { triplet.frame450_arcs() }, width_arcs);

	out << '\n';

	return out.str();
}

} // namespace dbar
} // namespace v_1_0_0
} // namespace arcsapp

