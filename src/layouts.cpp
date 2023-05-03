/**
 * \file
 *
 * \brief Formatting interfaces and classes
 */

#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#include "layouts.hpp"
#endif

#include <cstdint>             // for uint32_t, uint16_t
#include <iomanip>             // for setw, setfill
#include <ios>                 // for left, showbase, hex, uppercase
#include <memory>              // for unique_ptr, make_unique
#include <sstream>             // for ostringstream
#include <string>              // for string, to_string
#include <tuple>               // for get

#ifndef __LIBARCSTK_IDENTIFIER_HPP__
#include <arcstk/identifier.hpp>  // for ARId
#endif
#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>   // for Checksum
#endif
#ifndef __LIBARCSTK_PARSE_HPP__
#include <arcstk/parse.hpp>       // for ARTriplet
#endif

#ifndef __ARCSTOOLS_RESULT_HPP__
#include "result.hpp"             // for ResultObject
#endif

namespace arcsapp
{


// InternalFlags


InternalFlags::InternalFlags(const uint32_t flags)
	: flags_(flags)
{
	// empty
}


void InternalFlags::set_flag(const int idx, const bool value)
{
	if (value)
	{
		flags_ |= (1 << idx);  // <= true
	} else
	{
		flags_ &= ~(1 << idx); // <= false
	}
}


bool InternalFlags::flag(const int idx) const
{
	return flags_ & (1 << idx);
}


bool InternalFlags::no_flags() const
{
	return flags_ == 0;
}


bool InternalFlags::only_one_flag() const
{
	return flags_ && !(flags_ & (flags_ - 1));
}


bool InternalFlags::only(const int idx) const
{
	return flag(idx) && only_one_flag();
}


// HexLayout


HexLayout::HexLayout()
	: WithInternalFlags()
{
	set_show_base(false);
	set_uppercase(true);
}


void HexLayout::set_show_base(const bool base)
{
	flags().set_flag(0, base);
}


bool HexLayout::shows_base() const
{
	return flags().flag(0);
}


void HexLayout::set_uppercase(const bool uppercase)
{
	flags().set_flag(1, uppercase);
}


bool HexLayout::is_uppercase() const
{
	return flags().flag(1);
}


std::string HexLayout::do_format(InputTuple t) const
{
	auto checksum = std::get<0>(t);
	auto width    = std::get<1>(t);

	std::ostringstream ss;

	if (shows_base())
	{
		ss << std::showbase;
	}

	if (is_uppercase())
	{
		ss << std::uppercase;
	}

	ss << std::hex << std::setw(width) << std::setfill('0') << checksum.value();

	return ss.str();
}


// ARTripletLayout

using arcstk::Checksum;

std::string ARTripletLayout::do_format(InputTuple t) const
{
	const auto track   = std::get<0>(t);
	const auto triplet = std::get<1>(t);

	HexLayout hex; // TODO Make this configurable, inherit from WithChecksums...
	hex.set_show_base(false);
	hex.set_uppercase(true);

	const int width_arcs = 8;
	const int width_conf = 2;

	const auto unparsed_value = std::string { "????????" };

	std::ostringstream out;

	// TODO Make label configurable
	out << "Track " << std::setw(2) << std::setfill('0') << track << ": ";

	if (triplet.arcs_valid())
	{
		out << std::setw(width_arcs)
			<< hex.format(Checksum { triplet.arcs() }, width_arcs);
	} else
	{
		out << std::setw(width_arcs) << unparsed_value;
	}

	out << " ";

	out << "(";
	if (triplet.confidence_valid())
	{
		out << std::setw(width_conf) << std::setfill('0')
			<< static_cast<unsigned int>(triplet.confidence());
	} else
	{
		out << "??";
	}
	out << ") ";

	if (triplet.frame450_arcs_valid())
	{
		out << std::setw(width_arcs)
			<< hex.format(Checksum { triplet.frame450_arcs() }, width_arcs);
	} else
	{
		out << std::setw(width_arcs) << unparsed_value;
	}

	out << '\n';

	return out.str();
}


// ARIdLayout


ARIdLayout::ARIdLayout()
	: WithInternalFlags { 0xFFFFFFFF } // all flags true
	, field_labels_ { true }
{
	// empty
}


ARIdLayout::ARIdLayout(const bool labels, const bool id, const bool url,
		const bool filename, const bool track_count, const bool disc_id_1,
		const bool disc_id_2, const bool cddb_id)
	: WithInternalFlags(
			0
			| (id          << details::to_underlying(ARID_FLAG::ID))
			| (url         << details::to_underlying(ARID_FLAG::URL))
			| (filename    << details::to_underlying(ARID_FLAG::FILENAME))
			| (track_count << details::to_underlying(ARID_FLAG::TRACKS))
			| (disc_id_1   << details::to_underlying(ARID_FLAG::ID1))
			| (disc_id_2   << details::to_underlying(ARID_FLAG::ID2))
			| (cddb_id     << details::to_underlying(ARID_FLAG::CDDBID))
		)
	, field_labels_ { labels }
{
	// empty
}


ARIdLayout::~ARIdLayout() noexcept = default;


bool ARIdLayout::fieldlabels() const
{
	return field_labels_;
}


void ARIdLayout::set_fieldlabels(const bool labels)
{
	field_labels_ = labels;
}


bool ARIdLayout::id() const
{
	return flags().flag(details::to_underlying(ARID_FLAG::ID));
}


void ARIdLayout::set_id(const bool id)
{
	flags().set_flag(details::to_underlying(ARID_FLAG::ID), id);
}


bool ARIdLayout::url() const
{
	return flags().flag(details::to_underlying(ARID_FLAG::URL));
}


void ARIdLayout::set_url(const bool url)
{
	flags().set_flag(details::to_underlying(ARID_FLAG::URL), url);
}


bool ARIdLayout::filename() const
{
	return flags().flag(details::to_underlying(ARID_FLAG::FILENAME));
}


void ARIdLayout::set_filename(const bool filename)
{
	flags().set_flag(details::to_underlying(ARID_FLAG::FILENAME), filename);
}


bool ARIdLayout::track_count() const
{
	return flags().flag(details::to_underlying(ARID_FLAG::TRACKS));
}


void ARIdLayout::set_trackcount(const bool trackcount)
{
	flags().set_flag(details::to_underlying(ARID_FLAG::TRACKS), trackcount);
}


bool ARIdLayout::disc_id_1() const
{
	return flags().flag(details::to_underlying(ARID_FLAG::ID1));
}


void ARIdLayout::set_disc_id_1(const bool disc_id_1)
{
	flags().set_flag(details::to_underlying(ARID_FLAG::ID1), disc_id_1);
}


bool ARIdLayout::disc_id_2() const
{
	return flags().flag(details::to_underlying(ARID_FLAG::ID2));
}


void ARIdLayout::set_disc_id_2(const bool disc_id_2)
{
	flags().set_flag(details::to_underlying(ARID_FLAG::ID2), disc_id_2);
}


bool ARIdLayout::cddb_id() const
{
	return flags().flag(details::to_underlying(ARID_FLAG::CDDBID));
}


void ARIdLayout::set_cddb_id(const bool cddb_id)
{
	flags().set_flag(details::to_underlying(ARID_FLAG::CDDBID), cddb_id);
}


bool ARIdLayout::has_only(const ARID_FLAG flag) const
{
	return flags().only(details::to_underlying(flag));
}


std::unique_ptr<ARIdLayout> ARIdLayout::clone() const
{
	return do_clone();
}


auto ARIdLayout::show_flags() const -> decltype( show_flags_ )
{
	return show_flags_;
}


auto ARIdLayout::labels() const -> decltype( labels_ )
{
	return labels_;
}


std::string ARIdLayout::hex_id(const uint32_t id) const
{
	std::ostringstream out;

	out << std::hex << std::uppercase << std::setfill('0')
		<< std::setw(8) << id;

	return out.str();
}


// ARIdTableLayout


std::string ARIdTableLayout::do_format(InputTuple t) const
{
	auto arid       = std::get<0>(t);
	auto alt_prefix = std::get<1>(t);

	if (flags().no_flags()) // return ARId as default
	{
		return arid.to_string();
	}

	auto stream = std::ostringstream {};
	auto value = std::string {};

	// TODO Use optimal_label_width?
	//auto label_width = fieldlabels() ? optimal_width(labels()) : 0;
	auto label_width = fieldlabels() ? 8 : 0;

	for (const auto& sflag : show_flags())
	{
		if (not flags().flag(details::to_underlying(sflag))) { continue; }

		if (!stream.str().empty()) { stream << '\n'; }

		if (fieldlabels())
		{
			stream << std::setw(label_width)
				<< std::left
				<< labels()[details::to_underlying(sflag)]
				<< " ";
		}

		switch (sflag)
		{
			case ARID_FLAG::ID:
				value = arid.to_string();
				break;
			case ARID_FLAG::URL:
				value = arid.url();
				if (not alt_prefix.empty())
				{
					value.replace(0, arid.prefix().length(), alt_prefix);
					// FIXME If alt_prefix does not end with '/' ?
				}
				break;
			case ARID_FLAG::FILENAME:
				value = arid.filename();
				break;
			case ARID_FLAG::TRACKS:
				value = std::to_string(arid.track_count());
				break;
			case ARID_FLAG::ID1:
				value = hex_id(arid.disc_id_1());
				break;
			case ARID_FLAG::ID2:
				value = hex_id(arid.disc_id_2());
				break;
			case ARID_FLAG::CDDBID:
				value = hex_id(arid.cddb_id());
				break;
			default:
				break;
		}

		stream << std::setw(value.length()) << value;
	}

	if (true) { stream << '\n'; } // TODO Make configurable

	return stream.str();
}


std::unique_ptr<ARIdLayout> ARIdTableLayout::do_clone() const
{
	return std::make_unique<ARIdTableLayout>(*this);
}



} // namespace arcsapp

