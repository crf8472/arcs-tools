/**
 * \file
 *
 * \brief Implements symbols from format.hpp.
 */

#ifndef __ARCSTOOLS_FORMAT_HPP__
#include "format.hpp"
#endif

#include <algorithm>    // for find
#include <cmath>        // for ceil
#include <cstddef>      // for size_t
#include <cstdint>      // for uint16_t
#include <iterator>     // for begin, end
#include <memory>       // for unique_ptr, make_unique
#include <ostream>      // for ostream
#include <sstream>      // for ostringstream
#include <stdexcept>    // for invalid_argument
#include <string>       // for string, to_string
#include <type_traits>  // for underlying_type_t
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
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>     // for ARCS_LOG* (_DEBUG, _ERROR)
#endif

#ifndef __ARCSTOOLS_TABLE_HPP__
#include "table.hpp"              // for StringTable, StringTableLayout
                                  // CellDecorator, DecoratedStringTable
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


// ResultProvider


std::unique_ptr<Result> ResultProvider::result() const
{
	return do_result();
}


// DecorationInterface


void DecorationInterface::register_to_record(const int record_idx,
		std::unique_ptr<CellDecorator> d)
{
	do_register_to_record(record_idx, std::move(d));
}


const CellDecorator* DecorationInterface::on_record(const int record_idx)
	const
{
	return do_on_record(record_idx);
}


void DecorationInterface::register_to_field(const int field_idx,
		std::unique_ptr<CellDecorator> d)
{
	do_register_to_field(field_idx, std::move(d));
}


const CellDecorator* DecorationInterface::on_field(const int field_idx)
	const
{
	return do_on_field(field_idx);
}


void DecorationInterface::mark(const int record_idx, const int field_idx)
{
	do_mark(record_idx, field_idx);
}


void DecorationInterface::unmark(const int record_idx, const int field_idx)
{
	do_unmark(record_idx, field_idx);
}


// TableComposer


const std::vector<ATTR>& TableComposer::fields() const
{
	return fields_;
}


int TableComposer::get_row(const int i, const int j) const
{
	return do_get_row(i, j);
}


int TableComposer::get_col(const int i, const int j) const
{
	return do_get_col(i, j);
}


std::unique_ptr<PrintableTable> TableComposer::table()
{
	// No decorators? Then we will prefer to return just the inner StringTable
	if (from_table().empty())
	{
		ARCS_LOG(DEBUG1) << "TableComposer returns undecorated string table";

		auto p { remove_object() };
		return p->remove_inner_table();
	}

	ARCS_LOG(DEBUG1) << "TableComposer returns decorated table";
	return remove_object();
}


void TableComposer::set_layout(std::unique_ptr<StringTableLayout> layout)
{
	in_table().set_layout(std::move(layout));
}


TableComposer::TableComposer(const std::vector<ATTR>& fields,
		std::unique_ptr<DecoratedStringTable> table)
	: RecordInterface<DecoratedStringTable, ATTR>  { std::move(table) }
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


std::string& TableComposer::value(const int record, const int field)
{
	return in_table()(get_row(record, field), get_col(record, field));
}


DecoratedStringTable& TableComposer::in_table()
{
	return to_object();
}


const DecoratedStringTable& TableComposer::from_table() const
{
	return object();
}


void TableComposer::assign_labels(const std::vector<ATTR>& fields)
{
	using std::begin;
	using std::end;

	// THEIRS is the only attribute that could occurr multiple times.
	// If it does, we want to append a counter in the label
	const auto total_theirs = std::count(begin(fields), end(fields),
			ATTR::THEIRS);

	// Add labels from internal label store
	auto theirs = int { 0 };
	auto label_p { end(labels_) };
	for (auto a = int { 0 }; a < fields.size(); ++a)
	{
		label_p = labels_.find(fields.at(a));
		if (end(labels_) != label_p)
		{
			// Name a THEIRS in the presence of multiple THEIRSs?
			if (total_theirs > 1 && ATTR::THEIRS == fields.at(a))
			{
				// XXX This will screw up the table for more than 99 blocks
				this->set_field_label(a, label_p->second
						+ (theirs < 10 ? " " : "")
						+ std::to_string(theirs));
				++theirs;
			} else
			{
				// If there is at least 1 THEIRS, we will name the
				// locally computed fields "Mine" instead of the default label
				if (total_theirs > 0)
				{
					if (ATTR::CHECKSUM_ARCS2 == fields.at(a))
					{
						this->set_field_label(a, "Mine(v2)");
					} else
					if (ATTR::CHECKSUM_ARCS1 == fields.at(a))
					{
						this->set_field_label(a, "Mine(v1)");
					} else
					{
						this->set_field_label(a, label_p->second);
					}
				} else
				{
					this->set_field_label(a, label_p->second);
				}

				theirs = 0;
			}
			// set_field_label is used polymorphically, but is called in ctor
		}
	}
}


void TableComposer::do_set_field(const int record_idx,
		const int field_idx, const std::string& str)
{
	this->value(record_idx, field_idx) = str;
}


const std::string& TableComposer::do_field(const int i,
		const ATTR& field_type) const
{
	const auto j = int { this->field_idx(field_type) };
	return this->from_table().ref(this->get_row(i, j), this->get_col(i, j));
}


void TableComposer::do_set_label(const ATTR& field_type,
		const std::string& label)
{
	this->set_field_label(this->field_idx(field_type), label);
}


std::string TableComposer::do_label(const ATTR& field_type) const
{
	return this->field_label(this->field_idx(field_type));
}


int TableComposer::do_field_idx(const ATTR& field_type, const int i) const
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


bool TableComposer::do_has_field(const ATTR& field_type) const
{
	return this->field_idx(field_type) != -1;
}


void TableComposer::do_mark(const int record_idx, const int field_idx)
{
	in_table().mark_decorated(record_idx, field_idx);
}


void TableComposer::do_unmark(const int record_idx, const int field_idx)
{
	in_table().unmark_decorated(record_idx, field_idx);
}


// RowTableComposer


RowTableComposer::RowTableComposer(const std::size_t entries,
		const std::vector<ATTR>& order, const bool with_labels)
	: TableComposer(order, std::make_unique<DecoratedStringTable>(
				static_cast<int>(entries), static_cast<int>(order.size())))
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


TableComposer::size_type RowTableComposer::do_total_records() const
{
	return this->from_table().rows();
}


TableComposer::size_type RowTableComposer::do_fields_per_record() const
{
	return this->from_table().cols();
}


void RowTableComposer::set_field_label(const int field_idx,
		const std::string& label)
{
	this->in_table().set_col_label(field_idx, label);
}


std::string RowTableComposer::field_label(const int field_idx) const
{
	return this->from_table().col_label(field_idx);
}


int RowTableComposer::do_get_row(const int i, const int j) const
{
	return i;
}


int RowTableComposer::do_get_col(const int i, const int j) const
{
	return j;
}


void RowTableComposer::do_register_to_record(const int record_idx,
			std::unique_ptr<CellDecorator> d)
{
	in_table().register_to_row(record_idx, std::move(d));
}


const CellDecorator* RowTableComposer::do_on_record(const int record_idx) const
{
	return from_table().row_decorator(record_idx);
}


void RowTableComposer::do_register_to_field(const int field_idx,
			std::unique_ptr<CellDecorator> d)
{
	in_table().register_to_col(field_idx, std::move(d));
}


const CellDecorator* RowTableComposer::do_on_field(const int field_idx) const
{
	return from_table().col_decorator(field_idx);
}


// ColTableComposer


ColTableComposer::ColTableComposer(const std::size_t total_records,
		const std::vector<ATTR>& field_types, const bool with_labels)
	: TableComposer(field_types, std::make_unique<DecoratedStringTable>(
			static_cast<int>(field_types.size()),
			static_cast<int>(total_records)))
{
	// Each column contains each type, therefore each column is RIGHT
	for (auto col = int { 0 }; col < from_table().cols(); ++col)
	{
		in_table().set_align(col, table::Align::RIGHT);
	}

	if (with_labels)
	{
		this->assign_labels(field_types);
		// XXX virtual call of set_field_label()
	}
}


TableComposer::size_type ColTableComposer::do_total_records() const
{
	return from_table().cols();
}


TableComposer::size_type ColTableComposer::do_fields_per_record() const
{
	return from_table().rows();
}


void ColTableComposer::set_field_label(const int field_idx,
		const std::string& label)
{
	this->in_table().set_row_label(field_idx, label);
}


std::string ColTableComposer::field_label(const int field_idx) const
{
	return this->from_table().row_label(field_idx);
}


int ColTableComposer::do_get_row(const int i, const int j) const
{
	return j;
}


int ColTableComposer::do_get_col(const int i, const int j) const
{
	return i;
}


void ColTableComposer::do_register_to_record(const int record_idx,
			std::unique_ptr<CellDecorator> d)
{
	in_table().register_to_col(record_idx, std::move(d));
}


const CellDecorator* ColTableComposer::do_on_record(const int record_idx) const
{
	return from_table().col_decorator(record_idx);
}


void ColTableComposer::do_register_to_field(const int field_idx,
			std::unique_ptr<CellDecorator> d)
{
	in_table().register_to_row(field_idx, std::move(d));
}


const CellDecorator* ColTableComposer::do_on_field(const int field_idx) const
{
	return from_table().col_decorator(field_idx);
}


// TableComposerBuilder


std::unique_ptr<TableComposer> TableComposerBuilder::build(
		const std::size_t records,
		const std::vector<ATTR>& field_types, const bool with_labels) const
{
	return do_build(records, field_types, with_labels);
}


// RowTableComposerBuilder


std::unique_ptr<TableComposer> RowTableComposerBuilder::do_build(
		const std::size_t records,
		const std::vector<ATTR>& field_types, const bool with_labels) const
{
	return std::make_unique<RowTableComposer>(records, field_types,
			with_labels);
}


// ColTableComposerBuilder


std::unique_ptr<TableComposer> ColTableComposerBuilder::do_build(
		const std::size_t records,
		const std::vector<ATTR>& field_types, const bool with_labels) const
{
	return std::make_unique<ColTableComposer>(records, field_types,
			with_labels);
}


Checksum FromResponse::do_read(const int block_idx, const int idx) const
{
	return source()->at(block_idx).at(idx).arcs();
}


Checksum FromRefvalues::do_read(const int /* block_idx */, const int idx) const
{
	return source()->at(idx);
}


Checksum EmptyChecksums::do_read(const int /* block_idx */,
		const int /* idx */ ) const
{
	return arcstk::EmptyChecksum;
}


// ResultFormatter


void ResultFormatter::set_builder(
		std::unique_ptr<TableComposerBuilder> c)
{
	builder_creator_ = std::move(c);
}


const TableComposerBuilder* ResultFormatter::builder() const
{
	return builder_creator_.get();
}


void ResultFormatter::set_table_layout(std::unique_ptr<StringTableLayout> l)
{
	table_layout_ = std::move(l);
}


StringTableLayout ResultFormatter::copy_table_layout() const
{
	return *table_layout_;
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


const ChecksumLayout* ResultFormatter::checksum_layout() const
{
	return checksum_layout_ ? checksum_layout_.get() : nullptr;
}


bool ResultFormatter::formats_label() const
{
	return flags().flag(MAX_ATTR + 1);
}


void ResultFormatter::format_label(const bool &value)
{
	flags().set_flag(MAX_ATTR + 1, value);
}


bool ResultFormatter::formats_data(const ATTR a) const
{
	return flags().flag(std::underlying_type_t<ATTR>(a));
}


void ResultFormatter::format_data(const ATTR a, const bool value)
{
	flags().set_flag(std::underlying_type_t<ATTR>(a), value);
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
		const print_flag_t print_flags,
		const std::vector<arcstk::checksum::type>& types_to_print,
		const int total_theirs) const
{
	return do_create_attributes(print_flags, types_to_print, total_theirs);
}


std::unique_ptr<TableComposer> ResultFormatter::create_composer(
		const std::size_t entries,
		const std::vector<ATTR>& field_types, const bool with_labels) const
{
	return builder()->build(entries, field_types, with_labels);
}


bool ResultFormatter::is_requested(const ATTR a) const
{
	return this->formats_data(a);
}


ResultFormatter::print_flag_t ResultFormatter::create_print_flags(
		const TOC* toc, const std::vector<std::string>& filenames) const
{
	const bool has_toc       { toc != nullptr };
	const bool has_filenames { !filenames.empty() };

	auto flags = print_flag_t {};

	flags.set(ATTR::TRACK,    has_toc && is_requested(ATTR::TRACK));
	flags.set(ATTR::OFFSET,   has_toc && is_requested(ATTR::OFFSET));
	flags.set(ATTR::LENGTH,   has_toc && is_requested(ATTR::LENGTH));
	flags.set(ATTR::FILENAME, has_filenames && is_requested(ATTR::FILENAME));

	return flags;
}


std::unique_ptr<Result> ResultFormatter::build_result(
		const std::vector<arcstk::checksum::type>& types_to_print,
		const Match* match,
		const int block,
		const Checksums& checksums,
		const ARId& arid,
		const TOC* toc,
		const ARResponse& response,
		const std::vector<Checksum>& refvalues,
		const std::vector<std::string>& filenames,
		const std::string& alt_prefix) const
{
	// Flags to indicate whether requested field_types should actually
	// be printed

	const auto print_flags { create_print_flags(toc, filenames) };

	ARCS_LOG(DEBUG1) << "Print flags:";
	ARCS_LOG(DEBUG1) << " tracks=    " << print_flags(ATTR::TRACK);
	ARCS_LOG(DEBUG1) << " offsets=   " << print_flags(ATTR::OFFSET);
	ARCS_LOG(DEBUG1) << " lengths=   " << print_flags(ATTR::LENGTH);
	ARCS_LOG(DEBUG1) << " filenames= " << print_flags(ATTR::FILENAME);

	// Construct result objects

	auto table { build_table(types_to_print, match, block, checksums, arid, toc,
			response, refvalues, filenames, print_flags) };

	ARCS_LOG(DEBUG2) << "build_result(): build_table() returned";

	if (!arid.empty() && arid_layout())
	{
		auto id { build_id(toc, arid, alt_prefix) };

		return std::make_unique<
			ResultObject<RichARId, std::unique_ptr<PrintableTable>>>(
				std::move(id), std::move(table));
	}

	return std::make_unique<ResultObject<std::unique_ptr<PrintableTable>>>(
			std::move(table));
}


void ResultFormatter::init_composer(TableComposer* c) const
{
	// do nothing
}


RichARId ResultFormatter::build_id(const TOC* /*toc*/, const ARId& arid,
		const std::string& alt_prefix) const
{
	if (arid_layout())
	{
		return RichARId { arid, arid_layout()->clone(), alt_prefix };
	}

	return RichARId { arid, std::make_unique<ARIdTableLayout>(/* default */
				formats_label(), /* field label */
				true,  /* print ID */
				true,  /* print URL */
				false, /* no filenames */
				false, /* no tracks */
				false, /* no id 1 */
				false, /* no id 2 */
				false),/* no cddb id */
		alt_prefix };
}


std::unique_ptr<PrintableTable> ResultFormatter::build_table(
		const std::vector<arcstk::checksum::type>& types_to_print,
		const Match* match,
		const int block,
		const Checksums& checksums,
		const ARId& arid,
		const TOC* toc,
		const ARResponse& response,
		const std::vector<Checksum>& refvalues,
		const std::vector<std::string>& filenames,
		const print_flag_t print) const
{
	ARCS_LOG(DEBUG2) << "build_table(): start";

	// Determine whether to use the ARResponse
	const auto use_response { response.size() };

	// Determine total number of 'theirs' field_types per reference block
	// (Maybe 0 for empty response and empty refvalues)
	const auto total_theirs_per_block {
		block < 0  // print all match results?
			? (use_response ? response.size() : (refvalues.empty() ? 0 : 1))
			: 1 // no best match declared
	};

	// Create field layout / record type
	const auto fields { create_attributes(print, types_to_print,
			total_theirs_per_block) };

	// Create table composer
	auto c { create_composer(checksums.size(), fields, formats_label()) };
	this->init_composer(c.get());

	// Create and populate container for field builders

	RecordCreator record_builder { c.get() };

	if (print(ATTR::TRACK))
	{
		record_builder.add_fields(std::make_unique<AddField<ATTR::TRACK>>());
	}

	if (print(ATTR::OFFSET))
	{
		record_builder.add_fields(
				std::make_unique<AddField<ATTR::OFFSET>>(toc));
	}

	if (print(ATTR::LENGTH))
	{
		record_builder.add_fields(
				std::make_unique<AddField<ATTR::LENGTH>>(&checksums));
	}

	if (print(ATTR::FILENAME))
	{
		record_builder.add_fields(
				std::make_unique<AddField<ATTR::FILENAME>>(&filenames));
	}

	for (const auto& t : types_to_print)
	{
		if (t == arcstk::checksum::type::ARCS2)
		{
			record_builder.add_fields(
					std::make_unique<AddField<ATTR::CHECKSUM_ARCS2>>(&checksums,
						this));
		} else
		{
			record_builder.add_fields(
					std::make_unique<AddField<ATTR::CHECKSUM_ARCS1>>(&checksums,
						this));
		}

	}

	std::unique_ptr<const ChecksumSource> reference;

	if (match) // Will we print matches?
	{
		// Configure source of checksums to print
		if (use_response)
		{
			reference = std::make_unique<FromResponse>(&response);
		} else
		{
			if (!refvalues.empty())
			{
				reference = std::make_unique<FromRefvalues>(&refvalues);
			} else
			{
				reference = std::make_unique<EmptyChecksums>();
			}
		}

		record_builder.add_fields(std::make_unique<AddField<ATTR::THEIRS>>(
					match, block, reference.get(), &types_to_print, this,
					total_theirs_per_block));
	}

	// Create each record of the entire table
	record_builder.create_records();

	c->set_layout(std::make_unique<StringTableLayout>(copy_table_layout()));

	ARCS_LOG(DEBUG2) << "build_table(): end";

	return c->table();
}


void ResultFormatter::mine_checksum(const Checksum& checksum,
		const int record, const int field, TableComposer* c) const
{
	do_mine_checksum(checksum, record, field, c);
}


void ResultFormatter::their_checksum(const Checksum& checksum,
		const bool does_match, const int record, const int field,
		TableComposer* c) const
{
	if (does_match)
	{
		do_their_match(checksum, record, field, c);
	} else
	{
		do_their_mismatch(checksum, record, field, c);
	}
}


std::string ResultFormatter::checksum(const Checksum& checksum) const
{
	if (checksum_layout())
	{
		return checksum_layout()->format(checksum, 8);
		// 8 chars for hexadecimal represented 32 bit checksums
	}

	std::ostringstream out;
	out << checksum; // via libarcstk's <<
	return out.str();
}


void ResultFormatter::do_mine_checksum(const Checksum& checksum,
		const int record, const int field, TableComposer* c)
		const
{
	c->set_field(record, field, this->checksum(checksum));
}


void ResultFormatter::do_their_match(const Checksum& checksum, const int record,
		const int field, TableComposer* c) const
{
	// do nothing
}


void ResultFormatter::do_their_mismatch(const Checksum& checksum,
		const int record, const int field, TableComposer* c) const
{
	// do nothing
}


// RecordCreator


RecordCreator::RecordCreator(TableComposer* c)
	: fields_ { /* empty */ }
	, composer_  { c }
{
	fields_.reserve(c->total_records());
}


void RecordCreator::add_fields(std::unique_ptr<FieldCreator> f)
{
	fields_.emplace_back(std::move(f));
}


void RecordCreator::create_record(const int record_idx) const
{
	for (const auto& field : fields_)
	{
		field->create(composer_, record_idx);
	}
}


void RecordCreator::create_records() const
{
	for (auto i = int { 0 }; i < composer_->total_records(); ++i)
	{
		this->create_record(i);
	}
}


} // namespace arcsapp

