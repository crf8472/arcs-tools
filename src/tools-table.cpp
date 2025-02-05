/**
 * \file
 *
 * \brief Implements symbols from tools-table.hpp.
 */

#ifndef __ARCSTOOLS_TOOLS_TABLE_HPP__
#include "tools-table.hpp"
#endif

#include <algorithm>    // for find
#include <cstddef>      // for size_t
#include <iterator>     // for begin, end
#include <memory>       // for unique_ptr, make_unique
#include <string>       // for string, to_string
#include <type_traits>  // for underlying_type_t
#include <utility>      // for move
#include <vector>       // for vector
#include <iostream>

#ifndef __LIBARCSTK_IDENTIFIER_HPP__
#include <arcstk/identifier.hpp>  // for ARId
#endif
#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>   // for Checksum, Checksums
#endif
#ifndef __LIBARCSTK_METADATA_HPP__
#include <arcstk/metadata.hpp>    // for ToC
#endif
#ifndef __LIBARCSTK_VERIFY_HPP__
#include <arcstk/verify.hpp>      // for VerificationResult
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>     // for ARCS_LOG* (_DEBUG, _ERROR)
#endif

#ifndef __ARCSTOOLS_TABLE_HPP__
#include "table.hpp"              // for StringTable, StringTableLayout
                                  // CellDecorator, DecoratedStringTable
#endif
#ifndef __ARCSTOOLS_TOOLS_ARID_HPP__ // for ARIdLayout
#include "tools-arid.hpp"
#endif
#ifndef __ARCSTOOLS_TOOLS_CALC_HPP__ // for ChecksumLayout
#include "tools-calc.hpp"
#endif

namespace arcsapp
{
inline namespace v_1_0_0
{
namespace table
{

// arcsapp
using arcsapp::arid::ARIdTableLayout;


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
	for (auto i = std::size_t { 0 }; i < this->from_table().cols(); ++i)
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


int RowTableComposer::do_get_row(const int i, const int /*j*/) const
{
	return i;
}


int RowTableComposer::do_get_col(const int /*i*/, const int j) const
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
	for (auto col = std::size_t { 0 }; col < from_table().cols(); ++col)
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


int ColTableComposer::do_get_row(const int /*i*/, const int j) const
{
	return j;
}


int ColTableComposer::do_get_col(const int i, const int /*j*/) const
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

	for (auto i = std::size_t { 0 }; i < field_types.size(); ++i)
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


// FieldCreator


int FieldCreator::track(const int record_idx) const
{
	return record_idx + 1;
}


void FieldCreator::create(TableComposer* c, const int record_idx) const
{
	this->do_create(c, record_idx);
}


// AddRecord


AddRecords::AddRecords(TableComposer* composer)
	: current_  { 0 }
	, composer_ { composer }
{
	// empty
}


void AddRecords::reset_current_record()
{
	current_ = 0;
}


void AddRecords::inc_current_record()
{
	++current_;
}


std::size_t AddRecords::current_record() const
{
	return current_;
}


void AddRecords::add_field(const FieldCreator& field) const
{
	field.create(composer_, current_record());
}


void AddRecords::add_record(
			const std::vector<std::unique_ptr<FieldCreator>>& fields)
			const
{
	using std::cbegin;
	using std::cend;

	std::for_each(cbegin(fields), cend(fields),
			[this](const std::unique_ptr<FieldCreator>& f){
				this->add_field(*f);
			});
}


void AddRecords::add_records(
		const std::vector<std::unique_ptr<FieldCreator>>& field_creators)
{
	const auto total_records { composer_->total_records() };

	while (current_record() < total_records)
	{
		add_record(field_creators);
		inc_current_record();
	}
}


void AddRecords::operator()(
		const std::vector<std::unique_ptr<FieldCreator>>& field_creators)
{
	reset_current_record();
	add_records(field_creators);
}


// add_field


void add_field(TableComposer* c, const int record_idx, const ATTR a,
		const std::string& s)
{
	c->set_field(record_idx, a, s);
}


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


void AddField<ATTR::OFFSET>::do_create(TableComposer* c, const int record_idx)
	const
{
	// TODO Fix this mess
	const auto offsets { toc_->offsets() };
	const auto t {
		static_cast<decltype( offsets )::size_type>(record_idx) };

	using std::to_string;
	add_field(c, record_idx, ATTR::OFFSET,
			to_string(toc_->offsets().at(t).frames()));
}


AddField<ATTR::OFFSET>::AddField(const ToC* toc)
	: toc_ { toc }
{
	/* empty */
}


void AddField<ATTR::LENGTH>::do_create(TableComposer* c, const int record_idx)
	const
{
	using std::to_string;
	using index_type = arcstk::Checksums::size_type;

	add_field(c, record_idx, ATTR::LENGTH, to_string(
			(*checksums_).at(static_cast<index_type>(record_idx)).length()));
}


AddField<ATTR::LENGTH>::AddField(const Checksums* checksums)
	: checksums_ { checksums }
{
	/* empty */
}


void AddField<ATTR::FILENAME>::do_create(TableComposer* c,
		const int record_idx) const
{
	if (filenames_->empty())
	{
		return;
	}

	// XXX decltype(filenames_)::size_type
	using index_type = std::vector<std::string>::size_type;
	const auto r { static_cast<index_type>(record_idx) };

	const auto element { filenames_->size() > r
		? filenames_->at(r)      /*by index: get element*/
		: *(filenames_->begin()) /*no index: get first*/
	};

	add_field(c, record_idx, ATTR::FILENAME, element);
}


AddField<ATTR::FILENAME>::AddField(const std::vector<std::string>* filenames)
	: filenames_ { filenames }
{
	/* empty */
}


void AddField<ATTR::CHECKSUM_ARCS1>::do_create(TableComposer* c,
		const int record_idx) const
{
	using arcstk::checksum::type;
	using index_type = arcstk::Checksums::size_type;

	add_field(c, record_idx, ATTR::CHECKSUM_ARCS1, formatted(
		checksums_->at(static_cast<index_type>(record_idx)).get(type::ARCS1),
		*layout_));
}


AddField<ATTR::CHECKSUM_ARCS1>::AddField(const Checksums* checksums,
		const ChecksumLayout* layout)
	: checksums_ { checksums }
	, layout_    { layout    }
{
	/* empty */
}


void AddField<ATTR::CHECKSUM_ARCS2>::do_create(TableComposer* c,
		const int record_idx) const
{
	using arcstk::checksum::type;
	using index_type = arcstk::Checksums::size_type;

	add_field(c, record_idx, ATTR::CHECKSUM_ARCS2, formatted(
		checksums_->at(static_cast<index_type>(record_idx)).get(type::ARCS2),
		*layout_));
}


AddField<ATTR::CHECKSUM_ARCS2>::AddField(const Checksums* checksums,
		const ChecksumLayout* layout)
	: checksums_ { checksums }
	, layout_    { layout    }

{
	/* empty */
}


// formatted


std::string formatted(const Checksum& checksum,
		const ChecksumLayout& layout)
{
	return layout.format(checksum, 8);
}


// TableCreator


TableCreator::TableCreator()
	: table_composer_builder_ { /* empty */ }
	, table_layout_           { /* empty */ }
	, arid_layout_            { /* empty */ }
	, checksum_layout_        { /* empty */ }
{
	// empty
}


void TableCreator::set_builder(
		std::unique_ptr<TableComposerBuilder> c)
{
	table_composer_builder_ = std::move(c);
}


const TableComposerBuilder* TableCreator::builder() const
{
	return table_composer_builder_.get();
}


void TableCreator::set_table_layout(std::unique_ptr<StringTableLayout> l)
{
	table_layout_ = std::move(l);
}


StringTableLayout TableCreator::copy_table_layout() const
{
	return *table_layout_;
}


void TableCreator::set_arid_layout(std::unique_ptr<ARIdLayout> format)
{
	arid_layout_ = std::move(format);
}


const ARIdLayout* TableCreator::arid_layout() const
{
	return arid_layout_ ? arid_layout_.get() : nullptr;
}


void TableCreator::set_checksum_layout(
		std::unique_ptr<ChecksumLayout> layout)
{
	checksum_layout_ = std::move(layout);
}


const ChecksumLayout* TableCreator::checksum_layout() const
{
	return checksum_layout_ ? checksum_layout_.get() : nullptr;
}


bool TableCreator::formats_labels() const
{
	return flags().flag(MAX_ATTR + 1);
}


void TableCreator::set_format_labels(const bool &value)
{
	flags().set_flag(MAX_ATTR + 1, value);
}


bool TableCreator::formats_field(const ATTR a) const
{
	return flags().flag(std::underlying_type_t<ATTR>(a));
}


void TableCreator::set_format_field(const ATTR a, const bool value)
{
	flags().set_flag(std::underlying_type_t<ATTR>(a), value);
}


std::vector<ATTR> TableCreator::create_field_types(
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



void TableCreator::populate_creators_list(
			std::vector<std::unique_ptr<FieldCreator>>& creators,
			const std::vector<ATTR>& field_types, const ToC& toc,
			const Checksums& checksums,
			const std::vector<std::string>& filenames) const
{

	creators.reserve(field_types.size());

	// do not repeat the find mechanism
	const auto required = [](const std::vector<ATTR>& fields, const ATTR f)
			{
				using std::cbegin;
				using std::cend;
				using std::find;
				return find(cbegin(fields), cend(fields), f) != cend(fields);
			};

	if (required(field_types, ATTR::TRACK))
	{
		creators.emplace_back(
				std::make_unique<AddField<ATTR::TRACK>>());
	}

	if (required(field_types, ATTR::OFFSET))
	{
		creators.emplace_back(
				std::make_unique<AddField<ATTR::OFFSET>>(&toc));
	}

	if (required(field_types, ATTR::LENGTH))
	{
		creators.emplace_back(
				std::make_unique<AddField<ATTR::LENGTH>>(&checksums));
	}

	if (required(field_types, ATTR::FILENAME))
	{
		creators.emplace_back(
				std::make_unique<AddField<ATTR::FILENAME>>(&filenames));
	}
}


void TableCreator::do_init_composer(TableComposer& /*c*/) const
{
	// default implementation does nothing
}


std::unique_ptr<TableComposer> TableCreator::create_composer(
		const std::size_t total_entries,
		const std::vector<ATTR>& field_types, const bool with_labels) const
{
	return builder()->build(total_entries, field_types, with_labels);
}


bool TableCreator::is_requested(const ATTR a) const
{
	return this->formats_field(a);
}


TableCreator::print_flag_t TableCreator::create_field_requests(
		const ToC* toc, const std::vector<std::string>& filenames) const
{
	const bool has_toc       { toc != nullptr };
	const bool has_filenames { !filenames.empty() };

	auto flags = print_flag_t {};

	// Optional default flags

	flags.set(ATTR::TRACK,      has_toc && is_requested(ATTR::TRACK));
	flags.set(ATTR::OFFSET,     has_toc && is_requested(ATTR::OFFSET));
	flags.set(ATTR::LENGTH,     has_toc && is_requested(ATTR::LENGTH));
	flags.set(ATTR::FILENAME,   has_filenames && is_requested(ATTR::FILENAME));

	ARCS_LOG(DEBUG1) << "Request flags for printing:";
	ARCS_LOG(DEBUG1) << " tracks    = " << flags(ATTR::TRACK);
	ARCS_LOG(DEBUG1) << " offsets   = " << flags(ATTR::OFFSET);
	ARCS_LOG(DEBUG1) << " lengths   = " << flags(ATTR::LENGTH);
	ARCS_LOG(DEBUG1) << " filenames = " << flags(ATTR::FILENAME);

	return flags;
}


std::unique_ptr<PrintableTable> TableCreator::format_table(
		const std::vector<ATTR>& field_list,
		const std::size_t total_records,
		const bool with_labels,
		std::vector<std::unique_ptr<FieldCreator>>& field_creators) const
{
	auto composer { create_composer(total_records, field_list, with_labels) };

	init_composer(*composer); // Hook implemented by subclass

	AddRecords(composer.get())(field_creators);

	composer->set_layout(
			std::make_unique<StringTableLayout>(copy_table_layout()));

	return composer->table();
}


void TableCreator::init_composer(TableComposer& c) const
{
	do_init_composer(c);
}

} // namespace table
} // namespace v_1_0_0
} // namespace arcsapp

