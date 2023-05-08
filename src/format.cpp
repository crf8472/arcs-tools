/**
 * \file
 *
 * \brief Implements symbols from format.hpp.
 */

#ifndef __ARCSTOOLS_FORMAT_HPP__
#include "format.hpp"
#endif

#include <algorithm>    // for find
#include <cstddef>      // for size_t
#include <cstdint>      // for uint16_t
#include <iterator>     // for begin, end
#include <memory>       // for unique_ptr, make_unique
#include <ostream>      // for ostream
#include <sstream>      // for ostringstream
#include <stdexcept>    // for invalid_argument
#include <string>       // for string, to_string
#include <utility>      // for move
#include <vector>       // for vector

#ifndef __LIBARCSTK_IDENTIFIER_HPP__
#include <arcstk/identifier.hpp>  // for ARId
#endif
#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>   // for Checksum
#endif
#ifndef __LIBARCSTK_MATCH_HPP__
#include <arcstk/match.hpp>       // for Match
#endif
#ifndef __LIBARCSTK_PARSE_HPP__
#include <arcstk/parse.hpp>       // for ARResponse
#endif

#ifndef __ARCSTOOLS_TABLE_HPP__
#include "table.hpp"              // for StringTable, StringTableLayout
#endif
#ifndef __ARCSTOOLS_RESULT_HPP__
#include "result.hpp"             // for Result, ResultObject
#endif

namespace arcsapp
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
std::string DefaultLabel<ATTR::THEIRS>()
{
	return "Theirs";
};


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
			{ ATTR::THEIRS,   DefaultLabel<ATTR::THEIRS>() },
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


void ResultComposer::assign_labels(const std::vector<ATTR>& attributes)
{
	using std::begin;
	using std::end;

	// THEIRS is the only attribute that could occurr multiple times.
	// If it does, we want to append a counter in the label
	const auto has_multiple_theirs = [&attributes]()
		{
			auto theirs = bool { false };
			for (const auto& a : attributes)
			{
				if (ATTR::THEIRS == a)
				{
					if (theirs) { return true; }
					else { theirs = true; }
				}
			}
			return false;
		}();

	// Add labels from internal label store
	auto theirs = int { 0 };
	auto label_p { end(labels_) };
	for (auto a = int { 0 }; a < attributes.size(); ++a)
	{
		label_p = labels_.find(attributes.at(a));
		if (end(labels_) != label_p)
		{
			if (has_multiple_theirs && ATTR::THEIRS == attributes.at(a))
			{
				this->set_field_label(a, label_p->second
						+ (theirs < 10 ? " " : "")
						+ std::to_string(theirs));
				++theirs;
			} else
			{
				this->set_field_label(a, label_p->second);
			}
			// set_field_label is used polymorphically, but is called in ctor
		}
	}
}


void ResultComposer::set_layout(std::unique_ptr<StringTableLayout> layout)
{
	in_table().set_layout(std::move(layout));
}


void ResultComposer::do_set_field(const int record_idx,
		const int field_idx, const std::string& str)
{
	this->value(record_idx, field_idx) = str;
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


int ResultComposer::do_field_idx(const ATTR& field_type, const int i) const
{
	using std::begin;
	using std::end;
	using std::find;

	auto o { begin(fields_) };
	for (auto k = int { 0 }; k < i; ++k)
	{
		o = find(o, end(fields_), field_type);
		if (end(fields_) == o)
		{
			return -1;
		}
		++o;
	}
	return o - begin(fields_) - 1;
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
	// Attributes are columns thus their alignment depends on their type
	for(const auto& c : { this->field_idx(ATTR::TRACK),
			this->field_idx(ATTR::OFFSET), this->field_idx(ATTR::LENGTH) })
	{
		if (c >= 0)
		{
			in_table().set_align(c, table::Align::RIGHT);
		}
	}

	// Stretch the "theirs" columns to a width of 8
	for (auto i = int { 0 }; i < this->from_table().cols(); ++i)
	{
		if (ATTR::THEIRS == order[i])
		{
			in_table().set_align(i, table::Align::BLOCK);
			// BLOCK makes the table respect max_width for this column,
			// whose default is 8.
		}
	}

	if (with_labels)
	{
		this->assign_labels(order); // XXX virtual call of set_field_label()
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


void RowResultComposer::set_field_label(const int field_idx,
		const std::string& label)
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
		this->assign_labels(attrs); // XXX virtual call of set_field_label()
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
		const std::vector<arcstk::checksum::type>& types_to_print,
		const int total_theirs) const
{
	return do_create_attributes(p_tracks, p_offsets, p_lengths, p_filenames,
			types_to_print, total_theirs);
}


std::unique_ptr<Result> ResultFormatter::build_result(
		const Checksums& checksums, const ARResponse* response,
		const std::vector<Checksum>* refsums,
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

	auto table { build_table(checksums, response, refsums, match, block,
			toc, arid, filenames, types_to_print,
			p_tracks, p_offsets, p_lengths, p_filenames) };

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
		const ARResponse* response, const std::vector<Checksum>* refvals,
		const Match* match, const int block, const TOC* toc, const ARId& arid,
		const std::vector<std::string>& filenames,
		const std::vector<arcstk::checksum::type>& types_to_print,
		const bool p_tracks, const bool p_offsets, const bool p_lengths,
		const bool p_filenames) const
{
	// Determine whether to use the ARResponse
	const auto use_response { response != nullptr && response->size() };

	// Determine total number of 'theirs' attributes to print
	const auto total_theirs {
		block < 0  // print all?
			? (use_response ? response->size() : (refvals ? 1 : 0))
			: 1
	};

	const auto attributes { create_attributes(
			p_tracks, p_offsets, p_lengths, p_filenames,
			types_to_print, total_theirs) };

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
			// If there is only one attribute that contains checksum values,
			// this is considered a "mine".
			mine_checksum(checksums, t, i, c.get());
		}

		if (use_response)
		{
			for (auto b = int { 0 }; b < total_theirs; ++b)
			{
				does_match = match->track(b, i, true)
					|| match->track(b, i, false);

				their_checksum(response->at(b).at(i).arcs(),
						does_match, i, b+1, c.get());
			}
		} else
		{
			if (refvals && match)
			{
				does_match = match->track(block, i, true)
					|| match->track(block, i, false);

				their_checksum(refvals->at(i), does_match, i, 1, c.get());
			}
		}
	} // records

	c->set_layout(std::make_unique<StringTableLayout>(table_layout()));

	return configure_table(c->table());
}


void ResultFormatter::their_checksum(const Checksum& checksum,
		const bool does_match, const int record, const int thrs_idx,
		ResultComposer* b) const
{
	do_their_checksum(checksum, does_match, record, thrs_idx, b);
}


void ResultFormatter::mine_checksum(const Checksums& checksums,
		const arcstk::checksum::type t, const int record, ResultComposer* b)
		const
{
	do_mine_checksum(checksums, t, record, b);
}


std::string ResultFormatter::checksum(const Checksum& checksum) const
{
	if (checksum_layout())
	{
		return checksum_layout()->format(checksum, 8);
	}

	std::ostringstream out;
	out << checksum; // via libarcstk's <<
	return out.str();
}


StringTable ResultFormatter::configure_table(StringTable&& table) const
{
	return table;
}


void ResultFormatter::do_mine_checksum(const Checksums& checksums,
		const arcstk::checksum::type type, const int record, ResultComposer* b)
		const
{
	ATTR attr = type == arcstk::checksum::type::ARCS2
			? ATTR::CHECKSUM_ARCS2
			: ATTR::CHECKSUM_ARCS1;

	b->set_field(record, b->field_idx(attr),
			this->checksum(checksums.at(record).get(type)));
}


void ResultFormatter::do_their_checksum(
		const Checksum& /* checksum */, const bool /* does_match */,
		const int /* record */, const int /* thrs_idx */,
		ResultComposer* /* b */) const
{
	// do nothing
}

} // namespace arcsapp

