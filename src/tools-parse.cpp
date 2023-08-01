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
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#include "layouts.hpp"            // for ARIdLayout, ARTripletLayout
#endif
#ifndef __ARCSTOOLS_APPLICATION_HPP__
#include "application.hpp"        // for Output
#endif

namespace arcsapp
{

using arcstk::ARId;
using arcstk::ARTriplet;
using arcstk::ContentHandler;


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


// ARParserContentPrintHandler


ARParserContentPrintHandler::ARParserContentPrintHandler()
	: block_counter_  { 0 }
	, track_          { 0 }
	, arid_layout_    { std::make_unique<ARIdTableLayout>(false, false, false,
							false, false, false, false, false) }
	, triplet_layout_ { std::make_unique<ARTripletLayout>() }
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


void ARParserContentPrintHandler::print(const std::string& str) const
{
	Output::instance().output(str);
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

	std::ostringstream ss;
	ss << "---------- Block " << std::dec << block_counter_ << " : ";
	this->print(ss.str());
}


void ARParserContentPrintHandler::do_id(const uint8_t track_count,
		const uint32_t disc_id1, const uint32_t disc_id2,
		const uint32_t cddb_id)
{
	ARId id(track_count, disc_id1, disc_id2, cddb_id);

	auto str = arid_layout()->format(id, std::string{});
	str += '\n';
	this->print(str);
}


void ARParserContentPrintHandler::do_triplet(const Checksum arcs,
		const uint8_t confidence, const Checksum frame450_arcs)
{
	++track_;
	const ARTriplet triplet(arcs, confidence, frame450_arcs);

	auto str = triplet_layout()->format(track_, triplet);
	this->print(str);
}


void ARParserContentPrintHandler::do_triplet(const Checksum arcs,
		const uint8_t confidence, const Checksum frame450_arcs,
		const bool arcs_valid, const bool confidence_valid,
		const bool frame450_arcs_valid)
{
	++track_;
	const ARTriplet triplet(arcs, confidence, frame450_arcs, arcs_valid,
			confidence_valid, frame450_arcs_valid);

	auto str = triplet_layout()->format(track_, triplet);
	this->print(str);
}


void ARParserContentPrintHandler::do_end_block()
{
	track_ = 0;
}


void ARParserContentPrintHandler::do_end_input()
{
	std::ostringstream ss;
	ss << "EOF======= Blocks: " << std::dec << block_counter_ << '\n';
	this->print(ss.str());
}


// ARFileParser


ARFileParser::ARFileParser()
	: filename_ {}
{
	// empty
}


ARFileParser::ARFileParser(ARFileParser &&rhs) noexcept
	: filename_ { /* empty */ }
{
	this->swap(rhs);
}


ARFileParser::ARFileParser(const std::string &filename)
	: filename_ { filename }
{
	// empty
}


void ARFileParser::set_file(const std::string &filename)
{
	filename_ = filename;
}


std::string ARFileParser::file() const noexcept
{
	return filename_;
}


uint32_t ARFileParser::do_parse()
{
	ARCS_LOG(DEBUG1) << "Open file: " << this->file();

	std::ifstream file;

	file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		file.open(this->file(), std::ifstream::in | std::ifstream::binary);
	}
	catch (const std::ifstream::failure& f)
	{
		throw std::runtime_error(
			std::string("Failed to open file '") + this->file() +
			std::string("', got: ") + typeid(f).name() +
			std::string(", message: ") + f.what());
	}

	return ARStreamParser::parse_stream(file);
}


void ARFileParser::on_catched_exception(std::istream &istream,
		const std::exception & /* e */) const
{
	auto *filestream { dynamic_cast<std::ifstream*>(&istream) };

	if (filestream)
	{
		filestream->close();
		return;
	}

	ARCS_LOG_WARNING << "Could not close filestream";
}


ARFileParser& ARFileParser::operator = (ARFileParser &&rhs) noexcept
{
	this->swap(rhs);
	return *this;
}


void ARFileParser::do_swap(ARStreamParser &rhs)
{
	auto casted_rhs { dynamic_cast<ARFileParser*>(&rhs) };

	if (!casted_rhs)
	{
		throw std::domain_error("Type mismatch detected while swapping");
	}

	using std::swap;

	swap(this->filename_, casted_rhs->filename_);
}


// ARStdinParser


ARStdinParser::ARStdinParser() = default;


ARStdinParser::ARStdinParser(ARStdinParser &&rhs) noexcept
{
	this->swap(rhs);
}


uint32_t ARStdinParser::do_parse()
{
	auto response_data { StdIn(1024).bytes() };

	VectorIStream<char> response_data_w(response_data);
	std::istream stream(&response_data_w);

	return ARStreamParser::parse_stream(stream);
}


ARStdinParser& ARStdinParser::operator = (ARStdinParser &&rhs) noexcept
{
	this->swap(rhs);
	return *this;
}


void ARStdinParser::on_catched_exception(std::istream & /* istream */,
			const std::exception &/* e */) const
{
	// empty
}

} // namespace arcsapp

