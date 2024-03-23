#ifndef __ARCSTOOLS_TOOLS_ARID_HPP__
#include "tools-arid.hpp"
#endif

/**
 * \file
 *
 * \brief Implements symbols from tools-arid.hpp.
 */

#include <iomanip>      // for setw, setfill
#include <memory>       // for unique_ptr
#include <utility>      // for move

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


// build_id


RichARId build_id(const TOC* /*toc*/, const ARId& arid,
		const std::string& alt_prefix, const ARIdLayout& layout)
{
	return RichARId { arid, layout.clone(), alt_prefix };
}


// default_arid_layout


std::unique_ptr<ARIdLayout> default_arid_layout(const bool& with_labels)
{
	return std::make_unique<ARIdTableLayout>(
				with_labels, /* field label */
				true,  /* print ID */
				true,  /* print URL */
				false, /* no filenames */
				false, /* no tracks */
				false, /* no id 1 */
				false, /* no id 2 */
				false  /* no cddb id */
	);
}

} // namespace arid
} // namespace v_1_0_0
} // namespace arcsapp

