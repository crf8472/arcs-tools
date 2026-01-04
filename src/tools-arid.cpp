#ifndef __ARCSTOOLS_TOOLS_ARID_HPP__
#include "tools-arid.hpp"
#endif

/**
 * \file
 *
 * \brief Implements symbols from tools-arid.hpp.
 */

#include <cstddef>      // for size_t
#include <iomanip>      // for setw, setfill
#include <memory>       // for unique_ptr
#include <sstream>      // for ostringstream
#include <tuple>        // for get
#include <utility>      // for move

#ifndef __LIBARCSTK_IDENTIFIER_HPP__
#include <arcstk/identifier.hpp>  // for ARId
#endif

#ifndef __LIBARCSTK_METADATA_HPP__
#include <arcstk/metadata.hpp>    // for ToC
#endif

namespace arcsapp
{
inline namespace v_1_0_0
{
namespace arid
{

// RichARId


RichARId::RichARId(const ARId& id, std::unique_ptr<ARIdLayout> layout,
			const std::string& alt_prefix)
	: id_ { id }
	, layout_ { std::move(layout) }
	, alt_prefix_ { alt_prefix }
{
	// empty
}


RichARId::RichARId(const ARId& id, std::unique_ptr<ARIdLayout> layout)
	: RichARId { id, std::move(layout), std::string{} }
{
	// empty
}


const ARId& RichARId::id() const
{
	return id_;
}


const ARIdLayout& RichARId::layout() const
{
	return *layout_;
}


const std::string& RichARId::alt_prefix() const
{
	return alt_prefix_;
}


std::ostream& operator << (std::ostream& o, const RichARId& a)
{
	o << a.layout().format(a.id(), a.alt_prefix());
	return o;
}


// ARIdLayout


ARIdLayout::ARIdLayout()
	: PropertyStore { 0xFFFFFFFF } // all flags true
	, field_labels_ { true }
{
	// empty
}


ARIdLayout::ARIdLayout(const bool labels, const bool id, const bool url,
		const bool filename, const bool track_count, const bool disc_id_1,
		const bool disc_id_2, const bool cddb_id)
	: PropertyStore {
			static_cast<uint32_t>(0)
			| details::flag_operand(ARID_FLAG::ID,       id)
			| details::flag_operand(ARID_FLAG::URL,      url)
			| details::flag_operand(ARID_FLAG::FILENAME, filename)
			| details::flag_operand(ARID_FLAG::TRACKS,   track_count)
			| details::flag_operand(ARID_FLAG::ID1,      disc_id_1)
			| details::flag_operand(ARID_FLAG::ID2,      disc_id_2)
			| details::flag_operand(ARID_FLAG::CDDBID,   cddb_id)
		}
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


auto ARIdLayout::label(const ARID_FLAG flag) const -> std::string
{
	return labels_[array_idx(flag)];
}


void ARIdLayout::set_label(const ARID_FLAG flag, const std::string& label)
{
	labels_[array_idx(flag)] = label;
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


auto ARIdLayout::array_idx(const ARID_FLAG flag) const -> unsigned
{
	return static_cast<unsigned>(details::to_underlying(flag));
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
	if (no_properties())
	{
		// no properties set means nothing to print
		return std::string{};
	}

	const auto arid       = std::get<0>(t);
	const auto alt_prefix = std::get<1>(t);

	auto stream = std::ostringstream {};
	auto value  = std::string {};

	// TODO Use optimal_label_width?
	//auto label_width = fieldlabels() ? optimal_width(labels()) : 0;
	auto label_width = fieldlabels() ? 8 : 0;

	for (const auto& sflag : show_flags())
	{
		if (not has_property(sflag)) { continue; }

		if (!stream.str().empty()) { stream << '\n'; }

		if (fieldlabels())
		{
			stream << std::setw(label_width) << std::left << label(sflag)
				<< " "; // always one whitespace between label and value
		}

		switch (sflag)
		{
			case ARID_FLAG::ID:
				using std::to_string;
				value = to_string(arid);
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


// build_id


RichARId build_id(const ToC* /*toc*/, const ARId& arid,
		const std::string& alt_prefix, const ARIdLayout& layout)
{
	return RichARId { arid, layout.clone(), alt_prefix };
}


// validate


void validate(const ARId& arid, const std::size_t total_tracks, const ToC* toc)
{
	if (arid.empty())
	{
		return;
		//otherwise:
		//throw std::invalid_argument("AccurateRip id must not be empty");
	}

	using std::to_string;

	const auto arid_total_tracks = static_cast<std::size_t>(arid.track_count());

	if (arid_total_tracks != total_tracks)
	{
		throw std::invalid_argument("Mismatch: "
			"Checksums for " + to_string(total_tracks)
			+ " files/tracks, but AccurateRip id specifies "
			+ to_string(arid.track_count()) + " tracks.");
	}

	if (toc)
	{
		const auto toc_total_tracks = toc->total_tracks();

		if (arid.track_count() != toc_total_tracks)
		{
			throw std::invalid_argument("Mismatch: "
				"Checksums for " + to_string(toc_total_tracks)
				+ " files/tracks, but AccurateRip id specifies "
				+ to_string(arid.track_count()) + " tracks.");
		}
	}
}

} // namespace arid
} // namespace v_1_0_0
} // namespace arcsapp

