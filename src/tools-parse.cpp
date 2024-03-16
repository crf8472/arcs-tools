#ifndef __ARCSTOOLS_TOOLS_PARSE_HPP__
#include "tools-parse.hpp"
#endif

#ifdef _WIN32 // XXX This is completely untested

#include <io.h>     // for stdin
#include <fcntl.h>  // for _setmode, 0_BINARY

#endif

#include <cstddef>           // for size_t
#include <cstdint>           // for uint32_t, uint8_t
#include <cstdio>            // for feof, ferror, fread, freopen
#include <cstring>           // for strerror
#include <exception>         // for exception
#include <fstream>           // for ifstream
#include <ios>               // for dec
#include <iostream>          // for cin
#include <istream>           // for istream
#include <memory>            // for unique_ptr, make_unique
#include <sstream>           // for ostringstream
#include <stdexcept>         // for domain_error, runtime_error
#include <string>            // for string
#include <utility>           // for move, swap

#ifndef __LIBARCSTK_IDENTIFIER_HPP__
#include <arcstk/identifier.hpp>  // for ARId
#endif
#ifndef __LIBARCSTK_DBAR_HPP__
#include <arcstk/dbar.hpp>
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#include "layouts.hpp"            // for ARIdLayout, DBARTripletLayout
#endif
#ifndef __ARCSTOOLS_APPLICATION_HPP__
#include "application.hpp"        // for Output
#endif


namespace arcsapp
{

using arcstk::ARId;

using arcstk::DBAR;
using arcstk::DBARTriplet;
using arcstk::parse_stream;

// StdIn


StdIn::StdIn(const std::size_t buf_size)
	: buf_size_ { buf_size }
{
	// empty
}


std::vector<char> StdIn::bytes()
{
	// Note: all predefined iostreams (like std::cin) are _obligated_ to be
	// bound to corresponding C streams.
	// Confer: http://eel.is/c++draft/narrow.stream.objects
	// Therefore, it seems reasonable to just use freopen/fread for speed but
	// it feels a little bit odd to fallback to C-style stuff here.

	// Some systems may require to reopen stdin in binary mode. Even if this
	// maybe not required on some systems, it should be a portable solution to
	// just do it always:

#ifdef _WIN32

	// Note: freopen is portable by definition but windows may frown about the
	// nullptr for parameter 'path' as pointed out in the MSDN page on freopen:
	// https://msdn.microsoft.com/en-us/library/wk2h68td.aspx
	// We therefore use _setmode for windows. (It's '_setmode', not 'setmode'.)

	_setmode(_fileno(stdin), O_BINARY);
	// https://msdn.microsoft.com/en-us/library/tw4k6df8.aspx
#else

	if(std::freopen(nullptr, "rb", stdin)){/*empty*/};
	// Ignore returned FILE ptr to stdin while avoiding g++ warning
#endif

	// Binary Mode From Here On

	if (std::ferror(stdin))
	{
		auto msg = std::ostringstream {};
		msg << "Error while opening stdin for input: " << std::strerror(errno)
			<< " (errno " << errno << ")";

		throw std::runtime_error(msg.str());
	}

	auto bytes = std::vector<char> {}; // collects the input bytes
	auto len   = std::size_t { 0 }; // number of bytes read from stdin
	auto buf { std::make_unique<char[]>(buf_size()) }; // input buffer
	const auto MAX_KB_INPUT = MAX_KB_ * 1024; // maximum input bytes to accept

	// As long as there are any bytes, read them

	while((len = std::fread(buf.get(), sizeof(buf[0]), buf_size(), stdin)) > 0)
	{
		if (std::ferror(stdin) and not std::feof(stdin))
		{
			auto msg = std::ostringstream {};
			msg << "While reading from stdin: " << std::strerror(errno)
				<< " (errno " << errno << ")";

			throw std::runtime_error(msg.str());
		}

		if (bytes.size() >= MAX_KB_INPUT)
		{
			auto msg = std::ostringstream {};
			msg << "Input exceeds maximum size of " << MAX_KB_
				<< " kilobytes, abort.";
			throw std::runtime_error(msg.str());
		}

		bytes.insert(bytes.end(), buf.get(), buf.get() + len);
	}

	return bytes;
}


std::size_t StdIn::buf_size() const
{
	return buf_size_;
}


// read_from_stdin


unsigned read_from_stdin(const std::size_t amount_of_bytes, ParseHandler* p,
		ParseErrorHandler* e)
{
	auto input_bytes { StdIn(amount_of_bytes).bytes() };
	VectorIStream<char> input_data(input_bytes);
	std::istream input_stream(&input_data);
	return arcstk::parse_stream(input_stream, p, e);
}


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
	ss << "EOF======= Blocks: " << std::dec << block_counter_ << '\n';
	this->print(ss.str());
}

} // namespace arcsapp

