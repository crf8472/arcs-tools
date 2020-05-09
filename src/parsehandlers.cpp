#ifndef __ARCSTOOLS_PARSEHANDLERS_HPP__
#include "parsehandlers.hpp"
#endif

#include <iostream>
#include <fstream>
#include <sstream>

#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#ifndef __ARCSTOOLS_OUTPUTFORMATS_HPP__
#include "outputformats.hpp"
#endif


using arcstk::ARId;
using arcstk::ARTriplet;
using arcstk::ContentHandler;


// ARParserContentPrintHandler


ARParserContentPrintHandler::ARParserContentPrintHandler()
	: block_counter_(0)
	, track_(0)
	, arid_format_(std::make_unique<ARIdTableFormat>(
				false, false, true, true, true, true))
	, triplet_format_(std::make_unique<ARTripletFormat>())
	, out_file_stream_()
	, out_stream_(std::cout.rdbuf())
{
	// empty
}


ARParserContentPrintHandler::ARParserContentPrintHandler(
		const std::string &filename)
	: block_counter_(0)
	, track_(0)
	, arid_format_(std::make_unique<ARIdTableFormat>(
				false, false, true, true, true, true))
	, triplet_format_(std::make_unique<ARTripletFormat>())
	, out_file_stream_(filename)
	, out_stream_(out_file_stream_.rdbuf())
{
	// empty
}


ARParserContentPrintHandler::~ARParserContentPrintHandler() = default;


void ARParserContentPrintHandler::set_arid_format(
		std::unique_ptr<ARIdLayout> format)
{
	arid_format_ = std::move(format);
}


const ARIdLayout& ARParserContentPrintHandler::arid_format() const
{
	return *arid_format_;
}


void ARParserContentPrintHandler::set_triplet_format(
		std::unique_ptr<ARTripletFormat> format)
{
	triplet_format_ = std::move(format);
}


const ARTripletFormat& ARParserContentPrintHandler::triplet_format() const
{
	return *triplet_format_;
}


ARIdLayout* ARParserContentPrintHandler::arid_layout()
{
	return arid_format_.get();
}


ARTripletFormat* ARParserContentPrintHandler::triplet_format()
{
	return triplet_format_.get();
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

	out_stream_ << arid_layout()->format(id, "") << std::endl;
}


void ARParserContentPrintHandler::do_triplet(const uint32_t arcs,
		const uint8_t confidence, const uint32_t frame450_arcs)
{
	++track_;
	ARTriplet triplet(arcs, confidence, frame450_arcs);

	triplet_format()->out(out_stream_, track_, triplet);
}


void ARParserContentPrintHandler::do_triplet(const uint32_t arcs,
		const uint8_t confidence, const uint32_t frame450_arcs,
		const bool arcs_valid, const bool confidence_valid,
		const bool frame450_arcs_valid)
{
	++track_;
	ARTriplet triplet(arcs, confidence, frame450_arcs, arcs_valid,
			confidence_valid, frame450_arcs_valid);

	triplet_format()->out(out_stream_, track_, triplet);
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

