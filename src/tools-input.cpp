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

#include <any>           // for any
#include <string>        // for string

#ifndef __LIBARCSTK_CHECKSUM_HPP__
#include <arcstk/checksum.hpp>    // for Checksum, EmptyChecksum
#endif
#ifndef __LIBARCSTK_DBAR_HPP__
#include <arcstk/dbar.hpp>        // for ChecksumSource
#endif
#ifndef __LIBARCSTK_IDENTIFIER_HPP__
#include "identifier.hpp"         // for ARId, EmptyARId
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>     // for ARCS_LOG,...
#endif

namespace arcsapp
{
inline namespace v_1_0_0
{

using arcstk::ARId;
using arcstk::Checksum;
using arcstk::ChecksumSource;
using arcstk::EmptyARId;
using arcstk::EmptyChecksum;


// StringParser


std::any StringParser::parse(const std::string& s) const
{
	ARCS_LOG(DEBUG1) << "=> " << start_message();
	return this->do_parse(s);
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

} // namespace v_1_0_0
} // namespace arcsapp

