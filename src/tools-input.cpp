/**
 * \internal
 *
 * \file
 *
 * \brief Implements symbols from tools-input.hpp.
 */

#ifndef ARCSTOOLS_TOOLS_INPUT_HPP_
#include "tools-input.hpp"
#endif

// TODO Windows stuff is untested
#ifdef _WIN32            // XXX These includes + clause is completely untested
#include <io.h>          // for stdin
#include <fcntl.h>       // for _setmode, 0_BINARY
#endif

#include <algorithm>     // for min
#include <cerrno>        // for errno
#include <cstddef>       // for size_t
#include <cstdint>       // for uint32_t, uint8_t
#include <cstdio>        // for feof, ferror, fread, freopen
#include <cstring>       // for strerror
#include <exception>     // for exception
#include <functional>    // for functional
#include <istream>       // for istream
#include <memory>        // for unique_ptr
#include <ostream>       // for ostream, endl, operator<<
#include <sstream>       // for istringstream, ostringstream
#include <stdexcept>     // for runtime_error
#include <string>        // for string, stoul, getline
#include <utility>       // for make_unique, move
#include <vector>        // for vector

#ifndef LIBARCSTK_CHECKSUM_HPP_
#include <arcstk/checksum.hpp>    // for Checksum, EmptyChecksum
#endif
#ifndef LIBARCSTK_DBAR_HPP_
#include <arcstk/dbar.hpp>        // for DBAR, ChecksumSource, ParseHandler, ...
#endif
#ifndef LIBARCSTK_IDENTIFIER_HPP_
#include <arcstk/identifier.hpp>  // for ARId
#endif
#ifndef LIBARCSTK_LOGGING_HPP_
#include <arcstk/logging.hpp>     // for ARCS_LOG,...
#endif

#ifndef ARCSTOOLS_CLITOKENS_HPP_
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

	// freopen returnes a FILE ptr to stdin. We do not use it, but we check
	// whether it was correctly overhanded.
	// NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
	if (std::freopen(nullptr, "rb", stdin) == nullptr)
	{
		auto msg = std::ostringstream {};
		msg << "Error while opening stdin for binary mode: "
			<< std::strerror(errno) << " (errno " << errno << ")";

		throw std::runtime_error(msg.str());
	}
#endif

	// Binary Mode From Here On

	if (std::ferror(stdin))
	{
		auto msg = std::ostringstream {};
		msg << "Error while opening stdin for input: " << std::strerror(errno)
			<< " (errno " << errno << ")";

		throw std::runtime_error(msg.str());
	}

	const auto MAX_KB_INPUT = MAX_KB_ * 1024; // maximum input bytes to accept

	auto bytes = std::vector<char> {}; // collects the input bytes
	bytes.reserve(std::min(static_cast<size_t>(MAX_KB_INPUT), buf_size()));

	// As long as there are any bytes, read them

	// NOLINTNEXTLINE(*-avoid-c-arrays)
	auto buf { std::make_unique<char[]>(buf_size()) }; // input buffer
	auto len = std::size_t { 0 }; // number of bytes read from stdin

	while((len = std::fread(buf.get(), sizeof(buf[0]), buf_size(), stdin)) > 0)
	{
		if (std::ferror(stdin)) // and not std::feof(stdin))
		{
			auto msg = std::ostringstream {};
			msg << "While reading from stdin: " << std::strerror(errno)
				<< " (errno " << errno << ")";

			throw std::runtime_error(msg.str());
		}

		if (bytes.size() + len >= MAX_KB_INPUT)
		{
			const auto space_left { MAX_KB_INPUT - bytes.size() };

			// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
			bytes.insert(bytes.end(), buf.get(), buf.get() + space_left);

			// FIXME return bytes + flag instead of exception

			auto msg = std::ostringstream {};
			msg << "Input exceeds maximum size of " << MAX_KB_
				<< " kilobytes, abort.";
			throw std::runtime_error(msg.str());
		}

		// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
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
	VectorIStream<char> input_data { input_bytes };
	std::istream input_stream { &input_data };
	return arcstk::parse_stream(input_stream, p, e);
}


// parse_list


void parse_list(const std::string& input_list, const char delim,
		const std::function<void(const std::string&)>& entry_hook)
{
	if (input_list.empty())
	{
		return;
	}

	if (input_list.length() > 500)  // TODO magic number, give up on big input
	{
		return;
	}

	auto input_stream = std::istringstream { input_list };
	auto string_part  = std::string {};

	while ( std::getline( input_stream, string_part, delim ) )
	{
		entry_hook(string_part);
	}

	if (input_stream.eof())
	{
		ARCS_LOG(DEBUG2) << "Input stream hit eof, parsing ended normally";
		return;
	}

	if (input_stream.fail())
	{
		ARCS_LOG_WARNING << "Input stream failed!"
			<< " Maybe not all parts have been parsed. Expect problems.";
		// TODO throw something
	}
}


// DBARParser


DBARParser::DBARParser()
	: result_ {/*empty*/}
{
	// empty
}


DBAR DBARParser::load_data(const std::string& dbar_file) const
{
	using cli::CallSyntaxException;

	auto builder = DBARBuilder {};

	try
	{
		if (!dbar_file.empty())
		{
			arcstk::parse_file(dbar_file, &builder, nullptr);
		} else
		{
			// TODO Maximum should be configurable
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


void DBARParser::do_parse_empty() const
{
	result_ = load_data("");
}


void DBARParser::do_parse_nonempty(const std::string& s) const
{
	result_ = load_data(s);
}


DBAR DBARParser::provide_object() const
{
	return result_;
}


// ChecksumValuesParser


ChecksumValuesParser::ChecksumValuesParser()
	: values_ {/*empty*/}
{
	// empty
}


std::string ChecksumValuesParser::start_message() const
{
	return "List of local reference checksums (=\"Theirs\")";
}


void ChecksumValuesParser::do_parse_empty() const
{
	ARCS_LOG_DEBUG << "Empty value, return default object";
}


void ChecksumValuesParser::do_parse_nonempty(
		const std::string& checksum_list) const
{
	values_ = ChecksumValuesSource { input::parse_list_to_objects<uint32_t>(
				checksum_list,
				',' /*delimiter for values*/,
				[](const std::string& s) -> uint32_t
				{
					return std::stoul(s, nullptr, 16);
				},
				counter()) };
}


ChecksumValuesSource ChecksumValuesParser::provide_object() const
{
	return values_;
}


// ChecksumValuesSource


ChecksumValuesSource::ChecksumValuesSource(ChecksumValuesType values)
	: reference_source_ { std::move(values) }
{
	// empty
}


ChecksumValuesSource::ChecksumValuesSource()
	: reference_source_ {/*empty*/}
{
	// empty
}


ARId ChecksumValuesSource::do_id(const ChecksumSource::size_type /*block_idx*/)
	const
{
	return ARId{};
}


Checksum ChecksumValuesSource::do_checksum(
		const ChecksumSource::size_type /*block_idx*/,
		const ChecksumSource::size_type track_idx) const
{
	return Checksum { reference_source_.at(track_idx) };
}


const uint32_t& ChecksumValuesSource::do_arcs_value(
		const ChecksumSource::size_type /*block_idx*/,
		const ChecksumSource::size_type track_idx) const
{
	return reference_source_.at(track_idx);
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
		using std::to_string;
		throw std::invalid_argument("Only index 0 is legal, cannot access index"
				+ to_string(block_idx));
	}

	return reference_source_.size();
}


std::size_t ChecksumValuesSource::do_size() const
{
	return 1;
}


std::unique_ptr<ChecksumSource> ChecksumValuesSource::do_clone() const
{
	return std::make_unique<ChecksumValuesSource>(*this);
}


ChecksumValuesType ChecksumValuesSource::values() const
{
	return reference_source_;
}


// EmptyChecksumSource


const uint32_t EmptyChecksumSource::zero;


EmptyChecksumSource::EmptyChecksumSource() = default;


ARId EmptyChecksumSource::do_id(const ChecksumSource::size_type /*block_idx*/)
	const
{
	return ARId{};
}


Checksum EmptyChecksumSource::do_checksum(const ChecksumSource::size_type /*b*/,
		const ChecksumSource::size_type /*idx*/) const
{
	return arcstk::Checksum {};
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

