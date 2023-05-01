/**
 * \file
 *
 * \brief Formatting interfaces and classes
 */

#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#include "layouts.hpp"
#endif
#ifndef __ARCSTOOLS_TABLE_HPP__
#include "table.hpp"
#endif
#ifndef __ARCSTOOLS_RESULT_HPP__
#include "result.hpp"                // for ResultObject
#endif

#include <algorithm>           // for copy_if, find
#include <cstddef>             // for size_t
#include <cstdint>             // for uint32_t, uint16_t
#include <iomanip>             // for operator<<, setw, setfill
#include <ios>                 // for left, showbase, hex, uppercase
#include <iterator>            // for back_inserter, begin, end
#include <memory>              // for unique_ptr, make_unique
#include <ostream>             // for ostream, endl
#include <sstream>             // for ostringstream
#include <stdexcept>           // for out_of_range, invalid_argument
#include <string>              // for string, to_string
#include <tuple>               // for get
#include <utility>             // for move
#include <vector>              // for vector


namespace arcsapp
{

template<>
std::string DefaultLabel<ATTR::TRACK>() { return "Track"; };

template<>
std::string DefaultLabel<ATTR::OFFSET>() { return "Offset"; };

template<>
std::string DefaultLabel<ATTR::LENGTH>() { return "Length"; };

template<>
std::string DefaultLabel<ATTR::FILENAME>() { return "Filename"; };

template<>
std::string DefaultLabel<ATTR::CHECKSUM_ARCS1>()
{
	return arcstk::checksum::type_name(arcstk::checksum::type::ARCS2);
};

template<>
std::string DefaultLabel<ATTR::CHECKSUM_ARCS2>()
{
	return arcstk::checksum::type_name(arcstk::checksum::type::ARCS2);
};

template<>
std::string DefaultLabel<ATTR::THEIRS_ARCS1>()
{
	return "Theirs1";
};

template<>
std::string DefaultLabel<ATTR::MINE_ARCS1>() { return "Mine1"; };

template<>
std::string DefaultLabel<ATTR::THEIRS_ARCS2>()
{
	return "Theirs2";
};

template<>
std::string DefaultLabel<ATTR::MINE_ARCS2>() { return "Mine2"; };


// ResultComposer


StringTable ResultComposer::table() const
{
	return object();
}


ResultComposer::ResultComposer(const std::vector<ATTR>& fields,
		StringTable&& table)
	: RecordInterface<StringTable, ATTR>  { std::move(table) }
	, fields_ { fields }
	, labels_ {
			{ ATTR::TRACK,    DefaultLabel<ATTR::TRACK>()    },
			{ ATTR::OFFSET,   DefaultLabel<ATTR::OFFSET>()   },
			{ ATTR::LENGTH,   DefaultLabel<ATTR::LENGTH>()   },
			{ ATTR::FILENAME, DefaultLabel<ATTR::FILENAME>() },
			{ ATTR::CHECKSUM_ARCS2,
				arcstk::checksum::type_name(arcstk::checksum::type::ARCS2) },
			{ ATTR::CHECKSUM_ARCS1,
				arcstk::checksum::type_name(arcstk::checksum::type::ARCS1) },
			{ ATTR::THEIRS_ARCS1, DefaultLabel<ATTR::THEIRS_ARCS1>() },
			{ ATTR::MINE_ARCS1,   DefaultLabel<ATTR::MINE_ARCS1>()   },
			{ ATTR::THEIRS_ARCS2, DefaultLabel<ATTR::THEIRS_ARCS2>() },
			{ ATTR::MINE_ARCS2,   DefaultLabel<ATTR::MINE_ARCS2>()   }
	}
{
	// empty
}


std::string& ResultComposer::value(const int record, const int attribute)
{
	return in_table()(get_row(record, attribute), get_col(record, attribute));
}


StringTable& ResultComposer::in_table()
{
	return object();
}


const StringTable& ResultComposer::from_table() const
{
	return object();
}


void ResultComposer::assign_labels()
{
	using std::begin;
	using std::end;

	// Assign each field declared its precached label
	auto fp { end(labels_) };
	for (const auto& f : fields_)
	{
		fp = labels_.find(f);
		if (end(labels_) != fp)
		{
			this->set_label(f, fp->second);
			// set_label is used polymorphically, therefore this is not in ctor
		}
	}
}


void ResultComposer::set_layout(std::unique_ptr<StringTableLayout> layout)
{
	in_table().set_layout(std::move(layout));
}


void ResultComposer::do_set_field(const int record_idx,
		const ATTR& field_type, const std::string& str)
{
	this->value(record_idx, this->field_idx(field_type)) = str;
}


const std::string& ResultComposer::do_field(const int i,
		const ATTR& field_type) const
{
	const auto j = int { this->field_idx(field_type) };
	return this->from_table().ref(this->get_row(i, j), this->get_col(i, j));
}


void ResultComposer::do_set_label(const ATTR& field_type,
		const std::string& label)
{
	this->set_field_label(this->field_idx(field_type), label);
}


std::string ResultComposer::do_label(const ATTR& field_type) const
{
	return this->field_label(this->field_idx(field_type));
}


int ResultComposer::do_field_idx(const ATTR& field_type) const
{
	using std::begin;
	using std::end;
	using std::find;

	const auto i { find(begin(fields_), end(fields_), field_type) };
	if (end(fields_) == i)
	{
		return -1;
	}
	return i - begin(fields_);
}


bool ResultComposer::do_has_field(const ATTR& field_type) const
{
	return this->field_idx(field_type) != -1;
}


std::unique_ptr<Result> ResultComposer::do_result() const
{
	return std::make_unique<ResultObject<StringTable>>(table());
}


// RowResultComposer


RowResultComposer::RowResultComposer(const std::size_t entries,
		const std::vector<ATTR>& order, const bool with_labels)
	: ResultComposer(order, StringTable { static_cast<int>(entries),
			static_cast<int>(order.size()) })
{
	// Attributes are columns since their alignment depends on their type
	for(const auto& c : { this->field_idx(ATTR::TRACK),
			this->field_idx(ATTR::OFFSET), this->field_idx(ATTR::LENGTH) })
	{
		if (c >= 0)
		{
			in_table().set_align(c, table::Align::RIGHT);
		}
	}

	if (with_labels)
	{
		this->assign_labels(); // XXX virtual call of set_label() (final)
	}
}


ResultComposer::size_type RowResultComposer::do_total_records() const
{
	return this->from_table().rows();
}


ResultComposer::size_type RowResultComposer::do_fields_per_record() const
{
	return this->from_table().cols();
}


void RowResultComposer::set_field_label(const int field_idx, const std::string& label)
{
	this->in_table().set_col_label(field_idx, label);
}


std::string RowResultComposer::field_label(const int field_idx) const
{
	return this->from_table().col_label(field_idx);
}


int RowResultComposer::get_row(const int i, const int j) const
{
	return i;
}


int RowResultComposer::get_col(const int i, const int j) const
{
	return j;
}


// ColResultComposer


ColResultComposer::ColResultComposer(const std::size_t total_records,
		const std::vector<ATTR>& attrs, const bool with_labels)
	: ResultComposer(attrs, StringTable {
			static_cast<int>(attrs.size()), static_cast<int>(total_records) })
{
	// Each column contains each type, therefore each column is RIGHT
	for (auto col = int { 0 }; col < from_table().cols(); ++col)
	{
		in_table().set_align(col, table::Align::RIGHT);
	}

	if (with_labels)
	{
		this->assign_labels(); // XXX virtual call of set_label() (final)
	}
}


ResultComposer::size_type ColResultComposer::do_total_records() const
{
	return from_table().cols();
}


ResultComposer::size_type ColResultComposer::do_fields_per_record() const
{
	return from_table().rows();
}


void ColResultComposer::set_field_label(const int field_idx,
		const std::string& label)
{
	this->in_table().set_row_label(field_idx, label);
}


std::string ColResultComposer::field_label(const int field_idx) const
{
	return this->from_table().row_label(field_idx);
}


int ColResultComposer::get_row(const int i, const int j) const
{
	return j;
}


int ColResultComposer::get_col(const int i, const int j) const
{
	return i;
}


// ResultComposerBuilder


std::unique_ptr<ResultComposer> ResultComposerBuilder::create_composer(
		const std::size_t entries,
		const std::vector<ATTR>& attributes, const bool with_labels) const
{
	return do_create_composer(entries, attributes, with_labels);
}


std::unique_ptr<ResultComposer> RowResultComposerBuilder::do_create_composer(
		const std::size_t entries,
		const std::vector<ATTR>& attributes, const bool with_labels) const
{
	return std::make_unique<RowResultComposer>(entries, attributes,
			with_labels);
}


std::unique_ptr<ResultComposer> ColResultComposerBuilder::do_create_composer(
		const std::size_t entries,
		const std::vector<ATTR>& attributes, const bool with_labels) const
{
	return std::make_unique<ColResultComposer>(entries, attributes,
			with_labels);
}


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

	out << std::endl;

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

		if (!stream.str().empty()) { stream << std::endl; }

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

	if (true) { stream << std::endl; } // TODO Make configurable

	return stream.str();
}


std::unique_ptr<ARIdLayout> ARIdTableLayout::do_clone() const
{
	return std::make_unique<ARIdTableLayout>(*this);
}


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


// ResultFormatter


void ResultFormatter::set_builder_creator(
		std::unique_ptr<ResultComposerBuilder> c)
{
	builder_creator_ = std::move(c);
}


const ResultComposerBuilder* ResultFormatter::builder_creator() const
{
	return builder_creator_.get();
}


std::unique_ptr<ResultComposer> ResultFormatter::create_composer(
		const std::size_t entries,
		const std::vector<ATTR>& attributes, const bool with_labels) const
{
	return builder_creator()->create_composer(entries, attributes,
			with_labels);
}


void ResultFormatter::set_table_layout(const StringTableLayout& l)
{
	table_layout_ = l;
}


StringTableLayout ResultFormatter::table_layout() const
{
	return table_layout_;
}


void ResultFormatter::set_arid_layout(std::unique_ptr<ARIdLayout> format)
{
	arid_layout_ = std::move(format);
}


const ARIdLayout* ResultFormatter::arid_layout() const
{
	return arid_layout_ ? arid_layout_.get() : nullptr;
}


void ResultFormatter::set_checksum_layout(
		std::unique_ptr<ChecksumLayout> layout)
{
	checksum_layout_ = std::move(layout);
}


void ResultFormatter::set_types_to_print(
		std::vector<arcstk::checksum::type> types)
{
	types_ = types;
}


std::vector<arcstk::checksum::type> ResultFormatter::types_to_print() const
{
	return types_;
}


const ChecksumLayout* ResultFormatter::checksum_layout() const
{
	return checksum_layout_ ? checksum_layout_.get() : nullptr;
}


bool ResultFormatter::label() const
{
	return flags().flag(0);
}


void ResultFormatter::set_label(const bool &label)
{
	flags().set_flag(0, label);
}


bool ResultFormatter::track() const
{
	return flags().flag(1);
}


void ResultFormatter::set_track(const bool &track)
{
	flags().set_flag(1, track);
}


bool ResultFormatter::offset() const
{
	return flags().flag(2);
}


void ResultFormatter::set_offset(const bool &offset)
{
	flags().set_flag(2, offset);
}


bool ResultFormatter::length() const
{
	return flags().flag(3);
}


void ResultFormatter::set_length(const bool &length)
{
	flags().set_flag(3, length);
}


bool ResultFormatter::filename() const
{
	return flags().flag(4);
}


void ResultFormatter::set_filename(const bool &filename)
{
	flags().set_flag(4, filename);
}


void ResultFormatter::validate(const Checksums& checksums, const TOC* toc,
		const ARId& arid, const std::vector<std::string>& filenames) const
{
	const auto total_tracks = checksums.size();

	if (total_tracks == 0)
	{
		throw std::invalid_argument("Missing value: "
				"Need some Checksums to print");
	}

	if (checksums.at(0).empty() || checksums.at(0).types().empty())
	{
		throw std::invalid_argument("Missing value: "
				"Checksums seem to hold no checksums");
	}

	if (!toc && filenames.empty())
	{
		throw std::invalid_argument("Missing value: "
				"Need either TOC data or filenames to print results");
	}

	if (toc && static_cast<uint16_t>(toc->total_tracks()) != total_tracks)
	{
		throw std::invalid_argument("Mismatch: "
				"Checksums for " + std::to_string(total_tracks)
				+ " files/tracks, but TOC specifies "
				+ std::to_string(toc->total_tracks()) + " tracks.");
	}

	if (!(filenames.empty()
				|| filenames.size() == total_tracks || filenames.size() == 1))
	{
		throw std::invalid_argument("Mismatch: "
				"Checksums for " + std::to_string(total_tracks)
				+ " files/tracks, but " + std::to_string(filenames.size())
				+ " files.");
	}

	if (!(arid.empty()
		|| static_cast<uint16_t>(arid.track_count()) == total_tracks))
	{
		throw std::invalid_argument("Mismatch: "
				"Checksums for " + std::to_string(total_tracks)
				+ " files/tracks, but AccurateRip id specifies "
				+ std::to_string(arid.track_count()) + " tracks.");
	}
}


std::vector<ATTR> ResultFormatter::create_attributes(
		const bool p_tracks, const bool p_offsets, const bool p_lengths,
		const bool p_filenames,
		const std::vector<arcstk::checksum::type>& types_to_print) const
{
	return do_create_attributes(p_tracks, p_offsets, p_lengths, p_filenames,
			types_to_print);
}


std::unique_ptr<Result> ResultFormatter::build_result(
		const Checksums& checksums, const std::vector<Checksum>& refsums,
		const Match* match, int block, const TOC* toc, const ARId& arid,
		const std::string& alt_prefix,
		const std::vector<std::string>& filenames,
		const std::vector<arcstk::checksum::type>& types_to_print) const
{
	// Flags to indicate whether requested attributes should actually
	// be printed

	// Only if a TOC is present, we print track information as requested
	const auto p_tracks  = toc ? track()  : false;
	const auto p_offsets = toc ? offset() : false;
	const auto p_lengths = toc ? length() : false;

	// Only if filenames are actually present, we print them as requested
	const auto p_filenames = !filenames.empty() ? filename() : false;

	// Construct result objects

	auto table { build_table(checksums, refsums, match, block, toc, arid,
			filenames, p_tracks, p_offsets, p_lengths, p_filenames,
			types_to_print) };

	if (!arid.empty() && arid_layout())
	{
		auto id { build_id(toc, arid, alt_prefix) };

		return std::make_unique<ResultObject<RichARId, StringTable>>(
				std::move(id), std::move(table));
	}

	return std::make_unique<ResultObject<StringTable>>(std::move(table));
}


RichARId ResultFormatter::build_id(const TOC* /*toc*/, const ARId& arid,
		const std::string& alt_prefix) const
{
	if (arid_layout())
	{
		return RichARId { arid, arid_layout()->clone(), alt_prefix };
	}

	return RichARId { arid, std::make_unique<ARIdTableLayout>(/* default */
				label(), /* field label */
				true,  /* print ID */
				true,  /* print URL */
				false, /* no filenames */
				false, /* no tracks */
				false, /* no id 1 */
				false, /* no id 2 */
				false),/* no cddb id */
		alt_prefix };
}


StringTable ResultFormatter::build_table(const Checksums& checksums,
		const std::vector<Checksum>& refsums, const Match* match,
		const int block, const TOC* toc, const ARId& arid,
		const std::vector<std::string>& filenames,
		const bool p_tracks, const bool p_offsets, const bool p_lengths,
		const bool p_filenames,
		const std::vector<arcstk::checksum::type>& types_to_print) const
{
	const auto attributes { create_attributes(p_tracks, p_offsets, p_lengths,
			p_filenames, types_to_print) };

	auto c { create_composer(checksums.size(), attributes, label()) };

	configure_composer(*c);

	using TYPE = arcstk::checksum::type;
	using std::to_string;

	auto does_match = bool { false };
	auto track = int { 1 }; // is always i + 1
	for (auto i = int { 0 }; i < c->total_records(); ++track, ++i)
	{
		if (p_tracks)
		{
			c->set_field(i, ATTR::TRACK, to_string(track));
		}

		if (p_offsets)
		{
			c->set_field(i, ATTR::OFFSET, to_string(toc->offset(track)));
		}

		if (p_lengths)
		{
			c->set_field(i, ATTR::LENGTH, to_string((checksums)[i].length()));
		}

		if (p_filenames)
		{
			if (filenames.size() > 1)
			{
				c->set_field(i, ATTR::FILENAME, filenames.at(i));
			} else
			{
				c->set_field(i, ATTR::FILENAME, *filenames.begin());
			}
		}

		for (const auto& t : types_to_print)
		{
			their_checksum(refsums, t, i, c.get());
			if (match)
			{
				does_match = match->track(block, i, t == TYPE::ARCS2);
			}
			// If there is only one attribute that contains checksum values,
			// this is considered a "mine".
			mine_checksum(checksums, t, i, c.get(), does_match);
		}
	} // entries

	c->set_layout(std::make_unique<table::StringTableLayout>(table_layout()));

	return c->table();
}


void ResultFormatter::their_checksum(const std::vector<Checksum>& checksums,
		const arcstk::checksum::type t, const int record, ResultComposer* b)
		const
{
	do_their_checksum(checksums, t, record, b);
}


void ResultFormatter::mine_checksum(const Checksums& checksums,
		const arcstk::checksum::type t, const int record, ResultComposer* b,
		const bool does_match) const
{
	do_mine_checksum(checksums, t, record, b, does_match);
}


void ResultFormatter::checksum_worker(const int record, ATTR a,
		const Checksum& checksum, ResultComposer* b) const
{
	if (checksum_layout())
	{
		b->set_field(record, a, checksum_layout()->format(checksum, 8));
	} else
	{
		std::ostringstream out;
		out << checksum; // via libarcstk's <<
		b->set_field(record, a, out.str());
	}
}

} // namespace arcsapp

