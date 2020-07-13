#ifndef __ARCSTOOLS_TOOLS_PARSE_HPP__
#include "tools-parse.hpp"
#endif

#include <fstream>                // for ostream, dec, ios_base, ios_base::f...
#include <iostream>               // for cout
#include <ostream>                // for operator<<, basic_ostream<>::__ostr...
#include <utility>                // for move

#ifndef __LIBARCSTK_IDENTIFIER_HPP__
#include <arcstk/identifier.hpp>  // for ARId
#endif

#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#include "layouts.hpp"            // for ARTripletLayout
#endif

namespace arcsapp
{

using arcstk::ARId;
using arcstk::ARTriplet;
using arcstk::ContentHandler;


// ARParserContentPrintHandler


ARParserContentPrintHandler::ARParserContentPrintHandler(
		const std::string &filename)
	: block_counter_(0)
	, track_(0)
	, arid_layout_(std::make_unique<ARIdTableLayout>(
				false, false, false, false, false, false, false))
	, triplet_layout_(std::make_unique<ARTripletLayout>())
	, out_file_stream_(filename)
	, out_stream_(
			filename.empty() ? std::cout.rdbuf() : out_file_stream_.rdbuf())
{
	// empty
}


ARParserContentPrintHandler::~ARParserContentPrintHandler() noexcept
= default;


void ARParserContentPrintHandler::set_arid_layout(
		std::unique_ptr<ARIdLayout> format)
{
	arid_layout_ = std::move(format);
}


const ARIdLayout& ARParserContentPrintHandler::arid_layout() const
{
	return *arid_layout_;
}


void ARParserContentPrintHandler::set_triplet_layout(
		std::unique_ptr<ARTripletLayout> format)
{
	triplet_layout_ = std::move(format);
}


const ARTripletLayout& ARParserContentPrintHandler::triplet_layout() const
{
	return *triplet_layout_;
}


ARIdLayout* ARParserContentPrintHandler::arid_layout()
{
	return arid_layout_.get();
}


ARTripletLayout* ARParserContentPrintHandler::triplet_layout()
{
	return triplet_layout_.get();
}


void ARParserContentPrintHandler::do_start_input()
{
	// empty
}


void ARParserContentPrintHandler::do_start_block()
{
	++block_counter_;

	std::ios_base::fmtflags prev_settings = out_stream_.flags();
	out_stream_ << "---------- Block " << std::dec << block_counter_ << " : ";
	out_stream_.flags(prev_settings);
}


void ARParserContentPrintHandler::do_id(const uint8_t track_count,
		const uint32_t disc_id1, const uint32_t disc_id2,
		const uint32_t cddb_id)
{
	ARId id(track_count, disc_id1, disc_id2, cddb_id);

	out_stream_ << arid_layout()->format(id, std::string{}) << std::endl;
}


void ARParserContentPrintHandler::do_triplet(const uint32_t arcs,
		const uint8_t confidence, const uint32_t frame450_arcs)
{
	++track_;
	const ARTriplet triplet(arcs, confidence, frame450_arcs);

	out_stream_ << triplet_layout()->format(track_, triplet);
}


void ARParserContentPrintHandler::do_triplet(const uint32_t arcs,
		const uint8_t confidence, const uint32_t frame450_arcs,
		const bool arcs_valid, const bool confidence_valid,
		const bool frame450_arcs_valid)
{
	++track_;
	const ARTriplet triplet(arcs, confidence, frame450_arcs, arcs_valid,
			confidence_valid, frame450_arcs_valid);

	out_stream_ << triplet_layout()->format(track_, triplet);
}


void ARParserContentPrintHandler::do_end_block()
{
	track_ = 0;
}


void ARParserContentPrintHandler::do_end_input()
{
	std::ios_base::fmtflags prev_settings = out_stream_.flags();
	out_stream_ << "EOF======= Blocks: " <<
		std::dec << block_counter_ << std::endl;
	out_stream_.flags(prev_settings);
}

} // namespace arcsapp

