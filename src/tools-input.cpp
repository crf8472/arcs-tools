/**
 * \internal
 *
 * \file
 *
 * \brief Implements symbols from tools-input.hpp.
 */

#ifndef __ARCSTOOLS_TOOLS_INPUT_HPP__
#include "tools-input.hpp"
#endif

// TODO Windows stuff is untested
#ifdef _WIN32            // XXX These includes + clause is completely untested
#include <io.h>          // for stdin
#include <fcntl.h>       // for _setmode, 0_BINARY
#endif

#include <algorithm>     // for replace
#include <any>           // for any
#include <cerrno>        // for errno
#include <cstddef>       // for size_t
#include <cstdint>       // for uint32_t, uint8_t
#include <cstdio>        // for feof, ferror, fread, freopen
#include <cstring>       // for strerror
#include <istream>       // for istream
#include <ostream>       // for ostream, endl, operator<<
#include <sstream>       // for istringstream, ostringstream
#include <stdexcept>     // for runtime_error
#include <string>        // for string

#ifndef __LIBARCSTK_CHECKSUM_HPP__
#include <arcstk/checksum.hpp>    // for Checksum, EmptyChecksum
#endif
#ifndef __LIBARCSTK_DBAR_HPP__
#include <arcstk/dbar.hpp>        // for DBAR, ChecksumSource, ParseHandler, ...
#endif
#ifndef __LIBARCSTK_IDENTIFIER_HPP__
#include "identifier.hpp"         // for ARId, EmptyARId
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>     // for ARCS_LOG,...
#endif

#ifndef __ARCSTOOLS_CLITOKENS_HPP__
#include "clitokens.hpp"          // for CallSyntaxException
#endif

namespace arcsapp
{
inline namespace v_1_0_0
{
namespace input
{

using arcstk::ARId;
using arcstk::Checksum;
using arcstk::ChecksumSource;
using arcstk::DBAR;
using arcstk::DBARBuilder;
using arcstk::ParseHandler;
using arcstk::ParseErrorHandler;


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


// parse_list


void parse_list(const std::string& list, const char delim,
		std::function<void(const std::string&)> value_hook)
{
	if (list.empty())
	{
		return;
	}

	auto in { list }; // copy

	// replace delimiters by spaces
	if (delim != ' ')
	{
		using std::begin;
		using std::end;
		std::replace(begin(in), end(in), delim, ' ');
	}
	// FIXME If the cli input list contains spaces, parsing will break
	// Like "a:b,c:d e,f:g" (with quotes containing spaces)?
	// Parsed as: a:b,c:d,e,f:g
	// In this example, "c" will not have the value "d e" but "d" and "e" will
	// be a name instead of a value having no value by itself.

	auto input = std::istringstream { in };
	auto value = std::string {};

	while (input >> value)
	{
		value_hook(value);
		value.clear();
	}
}


// StringParser


std::any StringParser::parse(const std::string& s) const
{
	ARCS_LOG(DEBUG1) << "=> " << start_message();
	return this->do_parse(s);
}


// DBARParser


DBAR DBARParser::load_data(const std::string& responsefile) const
{
	using cli::CallSyntaxException;

	auto builder = DBARBuilder {};

	try
	{
		if (!responsefile.empty())
		{
			arcstk::parse_file(responsefile, &builder, nullptr);
		} else
		{
			read_from_stdin(1024, &builder, nullptr);
		}
	} catch (const std::exception& e)
	{
		throw CallSyntaxException(e.what());
	}

	return builder.result();
}


std::string DBARParser::start_message() const
{
	return "AccurateRip reference checksums (=\"Theirs\")";
}


DBAR DBARParser::do_parse_empty() const
{
	return this->load_data("");
}


DBAR DBARParser::do_parse_nonempty(const std::string& s) const
{
	return this->load_data(s);
}


// ChecksumValuesParser


std::string ChecksumValuesParser::start_message() const
{
	return "List of local reference checksums (=\"Theirs\")";
}


ChecksumValuesType ChecksumValuesParser::do_parse_nonempty(
		const std::string& checksum_list) const
{
	auto i = int { 0 };
	auto refvals = input::parse_list_to_objects<uint32_t>(
				checksum_list,
				',',
				[&i](const std::string& s) -> uint32_t
				{
					const uint32_t value = std::stoul(s, nullptr, 16);
					ARCS_LOG(DEBUG1) << "Parse checksum: " << Checksum { value }
						<< " (Track " << ++i << ")";
					return value;
				});

	ARCS_LOG(DEBUG1) << "Parsed " << refvals.size() << " checksums";
	return refvals;
}


// ChecksumValuesSource


ARId ChecksumValuesSource::do_id(const ChecksumSource::size_type /*block_idx*/)
	const
{
	return arcstk::EmptyARId;
}


Checksum ChecksumValuesSource::do_checksum(
		const ChecksumSource::size_type /*block_idx*/,
		const ChecksumSource::size_type track_idx) const
{
	return source()->at(track_idx);
}


const uint32_t& ChecksumValuesSource::do_arcs_value(
		const ChecksumSource::size_type /*block_idx*/,
		const ChecksumSource::size_type track_idx) const
{
	return source()->at(track_idx);
}


const uint32_t& ChecksumValuesSource::do_confidence(
		const ChecksumSource::size_type /*block_idx*/,
		const ChecksumSource::size_type /*t*/) const
{
	return EmptyChecksumSource::zero;
}


const uint32_t& ChecksumValuesSource::do_frame450_arcs_value(
		const ChecksumSource::size_type /*block_idx*/,
		const ChecksumSource::size_type /*t*/) const
{
	return EmptyChecksumSource::zero;
}


std::size_t ChecksumValuesSource::do_size(
		const ChecksumSource::size_type block_idx) const
{
	if (block_idx > 0)
	{
		throw std::invalid_argument("Only index 0 is legal, cannot access index"
				+ std::to_string(block_idx));
	}

	return source()->size();
}


std::size_t ChecksumValuesSource::do_size() const
{
	return 1;
}


std::unique_ptr<ChecksumSource> ChecksumValuesSource::do_clone() const
{
	return std::make_unique<ChecksumValuesSource>(*this);
}


// EmptyChecksumSource


const uint32_t EmptyChecksumSource::zero;


EmptyChecksumSource::EmptyChecksumSource() = default;


ARId EmptyChecksumSource::do_id(const ChecksumSource::size_type /*block_idx*/)
	const
{
	return arcstk::EmptyARId;
}


Checksum EmptyChecksumSource::do_checksum(const ChecksumSource::size_type /*b*/,
		const ChecksumSource::size_type /*idx*/) const
{
	return arcstk::EmptyChecksum;
}


const uint32_t& EmptyChecksumSource::do_arcs_value(
		const ChecksumSource::size_type /*b*/,
		const ChecksumSource::size_type /*track*/) const
{
	return zero;
}


const uint32_t& EmptyChecksumSource::do_confidence(
		const ChecksumSource::size_type /*b*/,
		const ChecksumSource::size_type /*t*/) const
{
	return zero;
}


const uint32_t& EmptyChecksumSource::do_frame450_arcs_value(
		const ChecksumSource::size_type /*b*/,
		const ChecksumSource::size_type /*track*/) const
{
	return zero;
}


std::size_t EmptyChecksumSource::do_size(
		const ChecksumSource::size_type /*block_idx*/) const
{
	return 0;
}


std::size_t EmptyChecksumSource::do_size() const
{
	return 0;
}


std::unique_ptr<ChecksumSource> EmptyChecksumSource::do_clone() const
{
	return std::make_unique<EmptyChecksumSource>(*this);
}

} // namespace input
} // namespace v_1_0_0
} // namespace arcsapp

