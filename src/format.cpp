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
#ifndef __LIBARCSTK_VERIFY_HPP__
#include <arcstk/verify.hpp>      // for VerificationResult
#endif
#ifndef __LIBARCSTK_DBAR_HPP__
#include <arcstk/dbar.hpp>        // for DBAR
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


// DefaultLabel


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
	return arcstk::checksum::type_name(arcstk::checksum::type::ARCS1);
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

template<>
std::string DefaultLabel<ATTR::CONFIDENCE>() { return "cnf"; };


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


bool TableComposer::has_field(const ATTR f) const
{
	using std::begin;
	using std::end;
	using std::find;
	return find(begin(fields_), end(fields_), f) != end(fields_);
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


void TableComposer::do_set_label_by_type(const ATTR& field_type,
		const std::string& label)
{
	this->set_label(this->field_idx(field_type), label);
}


std::string TableComposer::do_label_by_type(const ATTR& field_type) const
{
	return this->label(this->field_idx(field_type));
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
		const std::vector<ATTR>& field_types)
	: TableComposer(field_types, std::make_unique<DecoratedStringTable>(
				static_cast<int>(entries), static_cast<int>(field_types.size())))
{
	// Attributes are columns thus their alignment depends on their type

	// Columns that appear exactly once
	for(const auto& c : { this->field_idx(ATTR::TRACK),
			this->field_idx(ATTR::OFFSET), this->field_idx(ATTR::LENGTH) })
	{
		if (c >= 0)
		{
			in_table().set_align(c, table::Align::RIGHT);
		}
	}

	// Columns that may appear multiple times
	for (auto i = int { 0 }; i < this->from_table().cols(); ++i)
	{
		// Stretch the "theirs" columns to a width of 8
		if (ATTR::THEIRS == field_types[i])
		{
			in_table().set_align(i, table::Align::BLOCK);
			// BLOCK makes the table respect max_width for this column,
			// whose default is 8.
		}

		// Align confidence columns
		if (ATTR::CONFIDENCE == field_types[i])
		{
			in_table().set_align(i, table::Align::RIGHT);
		}
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


void RowTableComposer::do_set_label_by_index(const int field_idx,
		const std::string& label)
{
	this->in_table().set_col_label(field_idx, label);
}


std::string RowTableComposer::do_label_by_index(const int field_idx) const
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
		const std::vector<ATTR>& field_types)
	: TableComposer(field_types, std::make_unique<DecoratedStringTable>(
			static_cast<int>(field_types.size()),
			static_cast<int>(total_records)))
{
	// Each column contains each type, therefore each column is RIGHT
	for (auto col = int { 0 }; col < from_table().cols(); ++col)
	{
		in_table().set_align(col, table::Align::RIGHT);
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


void ColTableComposer::do_set_label_by_index(const int field_idx,
		const std::string& label)
{
	this->in_table().set_row_label(field_idx, label);
}


std::string ColTableComposer::do_label_by_index(const int field_idx) const
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


TableComposerBuilder::TableComposerBuilder()
	: labels_ {
		{ ATTR::TRACK,          DefaultLabel<ATTR::TRACK>()          },
		{ ATTR::OFFSET,         DefaultLabel<ATTR::OFFSET>()         },
		{ ATTR::LENGTH,         DefaultLabel<ATTR::LENGTH>()         },
		{ ATTR::FILENAME,       DefaultLabel<ATTR::FILENAME>()       },
		{ ATTR::CHECKSUM_ARCS2, DefaultLabel<ATTR::CHECKSUM_ARCS2>() },
		{ ATTR::CHECKSUM_ARCS1, DefaultLabel<ATTR::CHECKSUM_ARCS1>() },
		{ ATTR::THEIRS,         DefaultLabel<ATTR::THEIRS>()         },
		{ ATTR::CONFIDENCE,     DefaultLabel<ATTR::CONFIDENCE>()     },
	}
{
	// empty
}


void TableComposerBuilder::set_label(ATTR type,
		const std::string& label)
{
	auto [ pos, success ] = labels_.insert(std::make_pair(type, label));

	if (!success)
	{
		if (pos->second == label)
		{
			return;
		}
		// TODO throw?
	}
}


std::string TableComposerBuilder::label(ATTR type)
{
	using std::end;

	const auto it { labels_.find(type) };
	if (it != end(labels_))
	{
		return it->second;
	}

	return std::string{}; // TODO Use constant
}


void TableComposerBuilder::assign_default_labels(TableComposer& c,
		const std::vector<ATTR>& field_types) const
{
	using std::begin;
	using std::end;

	auto p { end(labels_) };

	for (auto i = int { 0 }; i < field_types.size(); ++i)
	{
		p = labels_.find(field_types.at(i));

		if (end(labels_) == p)
		{
			c.set_label(i, "?"); // Could not find label
		} else
		{
			c.set_label(i, p->second);
		}
	}
}


std::unique_ptr<TableComposer> TableComposerBuilder::build(
		const std::size_t records,
		const std::vector<ATTR>& field_types, const bool with_labels) const
{
	auto composer { do_build(records, field_types, with_labels) };

	if (with_labels)
	{
		this->assign_default_labels(*composer.get(), field_types);
		// Default labels may be updated by the application subclass when
		// calling AddField
	}

	return composer;
}


// RowTableComposerBuilder


std::unique_ptr<TableComposer> RowTableComposerBuilder::do_build(
	const std::size_t records,
	const std::vector<ATTR>& field_types, const bool /*with_labels*/) const
{
	return std::make_unique<RowTableComposer>(records, field_types);
}


// ColTableComposerBuilder


std::unique_ptr<TableComposer> ColTableComposerBuilder::do_build(
		const std::size_t records,
		const std::vector<ATTR>& field_types, const bool /*with_labels*/) const
{
	return std::make_unique<ColTableComposer>(records, field_types);
}


// RefvaluesSource


ARId RefvaluesSource::do_id(const ChecksumSource::size_type block_idx) const
{
	return arcstk::EmptyARId;
}


Checksum RefvaluesSource::do_checksum(const ChecksumSource::size_type /*b*/,
		const ChecksumSource::size_type idx) const
{
	return source()->at(idx);
}


const uint32_t& RefvaluesSource::do_arcs_value(
		const ChecksumSource::size_type /*b*/,
		const ChecksumSource::size_type track) const
{
	return source()->at(track);
}


const uint32_t& RefvaluesSource::do_confidence(
		const ChecksumSource::size_type /*b*/,
		const ChecksumSource::size_type /*t*/) const
{
	static const auto zero = uint32_t { 0 };
	return zero;
}


const uint32_t& RefvaluesSource::do_frame450_arcs_value(
		const ChecksumSource::size_type /*b*/,
		const ChecksumSource::size_type /*t*/) const
{
	static const auto zero = uint32_t { 0 };
	return zero;
}


std::size_t RefvaluesSource::do_size(
		const ChecksumSource::size_type block_idx) const
{
	if (block_idx > 0)
	{
		throw std::invalid_argument("Only index 0 is legal, cannot access index"
				+ std::to_string(block_idx));
	}

	return source()->size();
}


std::size_t RefvaluesSource::do_size() const
{
	return 1;
}


// EmptyChecksumSource


const uint32_t EmptyChecksumSource::zero;


EmptyChecksumSource::EmptyChecksumSource() = default;


ARId EmptyChecksumSource::do_id(const ChecksumSource::size_type block_idx) const
{
	return arcstk::EmptyARId;
}


Checksum EmptyChecksumSource::do_checksum(const ChecksumSource::size_type /*b*/,
		const ChecksumSource::size_type idx) const
{
	return arcstk::EmptyChecksum;
}


const uint32_t& EmptyChecksumSource::do_arcs_value(
		const ChecksumSource::size_type /*b*/,
		const ChecksumSource::size_type track) const
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
		const ChecksumSource::size_type track) const
{
	return zero;
}


std::size_t EmptyChecksumSource::do_size(
		const ChecksumSource::size_type /* block_idx */) const
{
	return 0;
}


std::size_t EmptyChecksumSource::do_size() const
{
	return 0;
}


// FieldCreator


int FieldCreator::track(const int record_idx) const
{
	return record_idx + 1;
}


void FieldCreator::create(TableComposer* c, const int record_idx) const
{
	this->do_create(c, record_idx);
}


// RecordCreator


RecordCreator::RecordCreator(TableComposer* c)
	: fields_    { /* empty */ }
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


/**
 * \brief Worker for implementing \c do_create() in AddField subclasses.
 */
void add_field(TableComposer* c, const int record_idx, const ATTR a,
		const std::string& s)
{
	c->set_field(record_idx, a, s);
}


/**
 * \brief Worker for implementing \c do_create() in AddField subclasses.
 */
void add_field(TableComposer* c, const int record_idx, const int field_idx,
		const std::string& s)
{
	c->set_field(record_idx, field_idx, s);
}


// AddField


// Specializations for AddField

void AddField<ATTR::TRACK>::do_create(TableComposer* c, const int record_idx)
	const
{
	using std::to_string;
	add_field(c, record_idx, ATTR::TRACK,
			to_string(this->track(record_idx)));
}


void AddField<ATTR::OFFSET>::do_create(TableComposer* c, const int record_idx) const
{
	using std::to_string;
	add_field(c, record_idx, ATTR::OFFSET,
			to_string(toc_->offset(track(record_idx))));
}

AddField<ATTR::OFFSET>::AddField(const TOC* toc)
	: toc_ { toc }
{
	/* empty */
}


void AddField<ATTR::LENGTH>::do_create(TableComposer* c, const int record_idx) const
{
	using std::to_string;
	add_field(c, record_idx, ATTR::LENGTH,
			to_string((*checksums_).at(record_idx).length()));
}

AddField<ATTR::LENGTH>::AddField(const Checksums* checksums)
	: checksums_ { checksums }
{
	/* empty */
}


void AddField<ATTR::FILENAME>::do_create(TableComposer* c,
		const int record_idx) const
{
	// TODO if (filenames_.empty())
	add_field(c, record_idx, ATTR::FILENAME,
		(filenames_->size() > 1) ? filenames_->at(record_idx) /* by index */
								 : *(filenames_->begin())/* single one */);
}


AddField<ATTR::FILENAME>::AddField(const std::vector<std::string>* filenames)
	: filenames_ { filenames }
{
	/* empty */
}


void AddField<ATTR::CHECKSUM_ARCS1>::do_create(TableComposer* c,
		const int record_idx) const
{
	formatter_->mine_checksum(
			checksums_->at(record_idx).get(arcstk::checksum::type::ARCS1),
			record_idx, c->field_idx(ATTR::CHECKSUM_ARCS1), c);
}


AddField<ATTR::CHECKSUM_ARCS1>::AddField(const Checksums* checksums,
		const ResultFormatter* formatter)
	: checksums_ { checksums }
	, formatter_ { formatter }
{
	/* empty */
}


void AddField<ATTR::CHECKSUM_ARCS2>::do_create(TableComposer* c,
		const int record_idx) const
{
	formatter_->mine_checksum(
			checksums_->at(record_idx).get(arcstk::checksum::type::ARCS2),
			record_idx, c->field_idx(ATTR::CHECKSUM_ARCS2), c);
}


AddField<ATTR::CHECKSUM_ARCS2>::AddField(const Checksums* checksums,
		const ResultFormatter* formatter)
	: checksums_ { checksums }
	, formatter_ { formatter }
{
	/* empty */
}



void AddField<ATTR::THEIRS>::do_create(TableComposer* c, const int record_idx)
	const
{
	auto block_idx  = int { 0 }; // Iterate over blocks of checksums
	auto curr_type  { types_to_print_->at(0) }; // Current checksum type
	auto does_match = bool { false }; // Is current checksum a match?

	// Total number of THEIRS fields in the entire record type
	const auto total_theirs =
		total_theirs_per_block_ * types_to_print_->size();

	// 1-based number of the reference block to print
	auto idx_label = int { 0 };

	// field index of the "theirs"-column
	auto field_idx = int { 0 };

	// Create all "theirs" fields
	for (auto b = int { 0 }; b < total_theirs; ++b)
	{
		// Enumerate one or more blocks
		// (If block_ < 0 PRINTALL is present)
		block_idx =  block_ >= 0  ? block_  : b % total_theirs_per_block_;

		curr_type =
			types_to_print_->at(std::ceil(b / total_theirs_per_block_));

		does_match = vresult_->track(block_idx, record_idx,
						curr_type == arcstk::checksum::type::ARCS2);

		idx_label = block_idx + 1;
		field_idx = c->field_idx(ATTR::THEIRS, b + 1);

		// Update field label to show best block index
		c->set_label(field_idx, DefaultLabel<ATTR::THEIRS>()
					+ (idx_label < 10 ? std::string{" "} : std::string{})
					// XXX Block index greater than 99 will screw up labels
					+ std::to_string(idx_label));

		formatter_->their_checksum(
				checksums_->checksum(block_idx, record_idx), does_match,
				record_idx, field_idx, c);

		if (print_confidence_)
		{
			using std::to_string;
			add_field(c, record_idx, field_idx + 1,
				to_string(checksums_->confidence(block_idx, record_idx)));
		}
	}
}


AddField<ATTR::THEIRS>::AddField(
		const std::vector<arcstk::checksum::type>* types,
		const VerificationResult* vresult,
		const int block,
		const ChecksumSource* checksums,
		const ResultFormatter* formatter,
		const int total_theirs_per_block,
		const bool print_confidence)
	: types_to_print_         { types     }
	, vresult_                { vresult   }
	, block_                  { block     }
	, checksums_              { checksums }
	, formatter_              { formatter }
	, total_theirs_per_block_ { total_theirs_per_block }
	, print_confidence_       { print_confidence       }
{
	/* empty */
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


std::vector<ATTR> ResultFormatter::create_optional_fields(
		const print_flag_t print_flags) const
{
	std::vector<ATTR> fields;

	for (const auto& f : {
			// Optional fields: presence is defined by request via print_flag
			// TODO: generate list in dedicated member function
			ATTR::TRACK, ATTR::OFFSET, ATTR::LENGTH, ATTR::FILENAME } )
	{
		if (print_flags(f)) { fields.emplace_back(f); }
	}

	return fields;
}



void ResultFormatter::populate_common_creators(
			std::vector<std::unique_ptr<FieldCreator>>& creators,
			const std::vector<ATTR>& fields, const TOC& toc,
			const Checksums& checksums,
			const std::vector<std::string>& filenames) const
{

	creators.reserve(fields.size());

	// do not repeat the find mechanism
	const auto required = [](const std::vector<ATTR>& fields, const ATTR f)
			{
				using std::begin;
				using std::end;
				using std::find;
				return find(begin(fields), end(fields), f) != end(fields);
			};

	if (required(fields, ATTR::TRACK))
	{
		creators.emplace_back(
				std::make_unique<AddField<ATTR::TRACK>>());
	}

	if (required(fields, ATTR::OFFSET))
	{
		creators.emplace_back(
				std::make_unique<AddField<ATTR::OFFSET>>(&toc));
	}

	if (required(fields, ATTR::LENGTH))
	{
		creators.emplace_back(
				std::make_unique<AddField<ATTR::LENGTH>>(&checksums));
	}

	if (required(fields, ATTR::FILENAME))
	{
		creators.emplace_back(
				std::make_unique<AddField<ATTR::FILENAME>>(&filenames));
	}
}


void ResultFormatter::do_init_composer(TableComposer& /*c*/) const
{
	// default implementation does nothing
}


std::unique_ptr<TableComposer> ResultFormatter::create_composer(
		const std::size_t total_entries,
		const std::vector<ATTR>& field_types, const bool with_labels) const
{
	return builder()->build(total_entries, field_types, with_labels);
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

	// Optional default flags

	flags.set(ATTR::TRACK,      has_toc && is_requested(ATTR::TRACK));
	flags.set(ATTR::OFFSET,     has_toc && is_requested(ATTR::OFFSET));
	flags.set(ATTR::LENGTH,     has_toc && is_requested(ATTR::LENGTH));
	flags.set(ATTR::FILENAME,   has_filenames && is_requested(ATTR::FILENAME));
	flags.set(ATTR::CONFIDENCE, is_requested(ATTR::CONFIDENCE));

	ARCS_LOG(DEBUG1) << "Request flags for printing:";
	ARCS_LOG(DEBUG1) << " tracks=      " << flags(ATTR::TRACK);
	ARCS_LOG(DEBUG1) << " offsets=     " << flags(ATTR::OFFSET);
	ARCS_LOG(DEBUG1) << " lengths=     " << flags(ATTR::LENGTH);
	ARCS_LOG(DEBUG1) << " filenames=   " << flags(ATTR::FILENAME);
	ARCS_LOG(DEBUG1) << " confidences= " << flags(ATTR::CONFIDENCE);

	return flags;
}


std::unique_ptr<Result> ResultFormatter::format_table(
		const std::vector<ATTR>& field_list,
		const std::size_t total_records,
		const bool with_labels,
		std::vector<std::unique_ptr<FieldCreator>>& field_creators) const
{
	// Create table composer (requires field_types only for alignment)
	auto composer { create_composer(total_records, field_list, with_labels) };
	init_composer(*composer);

	// Execute FieldCreators and populate table
	{
		RecordCreator rcreator { composer.get() };
		for (auto& field : field_creators)
		{
			rcreator.add_fields(std::move(field));
		}
		rcreator.create_records();
	}

	composer->set_layout(
			std::make_unique<StringTableLayout>(copy_table_layout()));

	return std::make_unique<ResultObject<std::unique_ptr<PrintableTable>>>(
			std::move(composer->table()));
}


void ResultFormatter::init_composer(TableComposer& c) const
{
	do_init_composer(c);
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

} // namespace arcsapp

