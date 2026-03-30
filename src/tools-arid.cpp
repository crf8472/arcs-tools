/**
 * \internal
 *
 * \file
 *
 * \brief Implements symbols from tools-arid.hpp.
 */

#ifndef ARCSTOOLS_TOOLS_ARID_HPP_
#include "tools-arid.hpp"
#endif

#include <cstddef>      // for size_t
#include <iomanip>      // for setw, setfill
#include <memory>       // for unique_ptr
#include <sstream>      // for ostringstream
#include <tuple>        // for get
#include <utility>      // for move

#ifndef LIBARCSTK_IDENTIFIER_HPP_
#include <arcstk/identifier.hpp>  // for ARId
#endif
#ifndef LIBARCSTK_METADATA_HPP_
#include <arcstk/metadata.hpp>    // for ToC
#endif

#ifndef ARCSTOOLS_TOOLS_VALIDATE_HPP_
#include "tools-validate.hpp"     // for Validate
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
	: PropertyStore { Flags::ALL_TRUE }
	, field_labels_ { true }
{
	// empty
}


ARIdLayout::ARIdLayout(const bool labels, const bool id, const bool url,
		const bool filename, const bool track_count, const bool disc_id_1,
		const bool disc_id_2, const bool cddb_id)
	: PropertyStore {
			Flags::ALL_FALSE
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
					// FIXME If alt_prefix has a different length ?
					// FIXME Do not do this when printing, do it in the app
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


RichARId build_id(const ARId& arid, const std::string& alt_prefix,
		const ARIdLayout& layout)
{
	return RichARId { arid, layout.clone(), alt_prefix };
}


// validate


void validate(const ARId& arid, const std::size_t total_tracks, const ToC& toc)
{
	if (arid.empty())
	{
		return;
		//otherwise:
		//throw std::invalid_argument("AccurateRip id must not be empty");
	}

	using Validation  = valid::Validate<ARId, std::size_t, const ToC>;

	const std::vector<Validation> validations =
	{
		Validation
		{
			"ARId must not be empty",
			[](const ARId& a, const std::size_t, const ToC& /*t*/) noexcept
			{
				return not a.empty();
			},
			"ARId is unexpectedly empty"
		},
		Validation
		{
			"ARId has the declared number of tracks",
			[](const ARId& a, const std::size_t s, const ToC& /*t*/) noexcept
			{
				return s == static_cast<std::size_t>(a.track_count());
			},
			"ARId specifies another number of tracks than declared"
		},
		Validation
		{
			"ARId has the number of tracks specified by ToC",
			[](const ARId& a, const std::size_t /*s*/, const ToC& t) noexcept
			{
				if (!t) { return true; }

				return a.track_count() == t.total_tracks();
			},
			"ARId mismatches ToC: different total tracks specified"
		}
	};

	for (const auto& validation : validations)
	{
		validation.perform(arid, total_tracks, toc);
	}
}

} // namespace arid
} // namespace v_1_0_0
} // namespace arcsapp

