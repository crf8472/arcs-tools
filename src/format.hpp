#ifndef __ARCSTOOLS_FORMAT_HPP__
#define __ARCSTOOLS_FORMAT_HPP__

/**
 * \file
 *
 * \brief Formatter for result objects.
 */

#include <cmath>        // for ceil
#include <cstddef>      // for size_t
#include <cstdint>      // for uint32_t
#include <memory>       // for unique_ptr
#include <ostream>      // for ostream
#include <string>       // for string
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
#include <arcstk/dbar.hpp>       // for DBAR
#endif

#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#include "layouts.hpp"      // for WithInternalFlags, ARIdLayout, ChecksumLayout
#endif
#ifndef __ARCSTOOLS_RESULT_HPP__
#include "result.hpp"       // for Result, ResultObject
#endif
#ifndef __ARCSTOOLS_TABLE_HPP__
#include "table.hpp"        // for PrintableTable, StringTable,
                            // StringTableLayout, CellDecorator,
                            // DecoratedStringTable
#endif


namespace arcsapp
{

using arcstk::DBAR;


/**
 * \brief An ARId accompanied by a layout and an optional URL prefix.
 *
 * This object contains all information necessary to be printed.
 */
class RichARId
{
public:

	/**
	 * \brief An ARId with every information required for printing.
	 *
	 * \param[in] id      ARId to print
	 * \param[in] layout  Layout to use for printing
	 */
	RichARId(const arcstk::ARId& id, std::unique_ptr<ARIdLayout> layout);

	/**
	 * \brief An ARId with every information required for printing.
	 *
	 * \param[in] id         ARId to print
	 * \param[in] layout     Layout to use for printing
	 * \param[in] alt_prefix Optional alternative URL prefix
	 */
	RichARId(const arcstk::ARId& id, std::unique_ptr<ARIdLayout> layout,
			const std::string& alt_prefix);


	const arcstk::ARId& id() const;

	const ARIdLayout& layout() const;

	const std::string& alt_prefix() const;

private:

	arcstk::ARId id_;

	std::unique_ptr<ARIdLayout> layout_;

	std::string alt_prefix_;
};


/**
 * \brief Stream insertion operator for RichARId.
 */
std::ostream& operator << (std::ostream& o, const RichARId& a);


/**
 * \brief Interface for composing a container object holding records.
 *
 * \tparam T Type of the result object
 * \tparam F Type of the field keys
 */
template<typename T, typename F>
class RecordInterface
{
	/**
	 * \brief Internal result object.
	 */
	std::unique_ptr<T> object_;

public:

	/**
	 * \brief Size type of the records.
	 */
	using size_type = std::size_t;
	// TODO Define as T::size_type if it exists, otherwise define as size_t

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~RecordInterface() noexcept = default;

	/**
	 * \brief Set the value for the specified field in record \c i.
	 */
	void set_field(const int i, const F& field_type,
			const std::string& value)
	{
		this->do_set_field(i, this->field_idx(field_type), value);
	}

	/**
	 * \brief Set the value for the specified field in record \c i, field \c j.
	 */
	void set_field(const int i, const int j,
			const std::string& value)
	{
		this->do_set_field(i, j, value);
	}

	/**
	 * \brief The value of the specified field.
	 */
	std::string field(const int i, const F& field_type) const
	{
		return this->do_field(i, field_type);
	}

	/**
	 * \brief Set the label for the specified field type.
	 */
	void set_label(const F& field_type, const std::string& label)
	{
		this->do_set_label_by_type(field_type, label);
	}

	/**
	 * \brief Label for the specified field type.
	 */
	std::string label(const F& field_type) const
	{
		return this->do_label_by_type(field_type);
	}

	/**
	 * \brief Set the label for the specified field index.
	 */
	void set_label(const int field_idx, const std::string& label)
	{
		return this->do_set_label_by_index(field_idx, label);
	}

	/**
	 * \brief Label for the specified field index.
	 */
	std::string label(const int field_idx) const
	{
		return do_label_by_index(field_idx);
	}

	/**
	 * \brief First index of specified field type.
	 *
	 * \param[in] field_type  Type of the field
	 */
	int field_idx(const F& field_type) const
	{
		return this->do_field_idx(field_type, 1);
	}

	/**
	 * \brief I-th index of specified field type.
	 *
	 * \param[in] field_type  Type of the field
	 * \param[in] i           Occurrence of the field_type
	 */
	int field_idx(const F& field_type, const int i) const
	{
		return this->do_field_idx(field_type, i);
	}

	/**
	 * \brief TRUE iff specified field type is part of the result.
	 */
	bool has_field(const F& field_type) const
	{
		return this->do_has_field(field_type);
	}

	/**
	 * \brief Total number of records.
	 */
	size_type total_records() const
	{
		return this->do_total_records();
	}

	/**
	 * \brief Total number of fields per record.
	 */
	size_type fields_per_record() const
	{
		return this->do_fields_per_record();
	}

	/**
	 * \brief Return the object holding the records.
	 *
	 * \return Object with records
	 */
	const T& object() const
	{
		return *object_;
	}

	/**
	 * \brief Return and remove the object holding the records.
	 *
	 * \return Object with records
	 */
	std::unique_ptr<T> remove_object()
	{
		auto p { std::move(object_) };
		return p;
	}

protected:

	RecordInterface(std::unique_ptr<T> object)
		: object_ { std::move(object) }
	{
		// empty
	}

	/**
	 * \brief Read or manipulate the object holding the records.
	 */
	T& to_object()
	{
		return *object_;
	}

private:

	virtual void do_set_field(const int record_idx, const int field_idx,
			const std::string& str)
	= 0;

	virtual const std::string& do_field(const int i, const F& field_type) const
	= 0;

	virtual void do_set_label_by_type(const F& field_type,
			const std::string& label)
	= 0;

	virtual std::string do_label_by_type(const F& field_type) const
	= 0;

	virtual void do_set_label_by_index(const int field_idx,
			const std::string& label)
	= 0;

	virtual std::string do_label_by_index(const int field_idx) const
	= 0;

	virtual int do_field_idx(const F& field_type, const int i) const
	= 0;

	virtual bool do_has_field(const F& field_type) const
	= 0;

	virtual size_type do_total_records() const
	= 0;

	virtual size_type do_fields_per_record() const
	= 0;
};


/**
 * \brief Interface for providing a Result object.
 */
class ResultProvider
{
	virtual std::unique_ptr<Result> do_result() const
	= 0;

public:

	/**
	 * \brief Return the result object.
	 */
	std::unique_ptr<Result> result() const;
};


/**
 * \brief Attributes for representing result data.
 *
 * Use this field_types to define a layout for printing the result. The concrete
 * TableComposer will know whether the field_types are rows or columns in the
 * result table to build.
 */
enum class ATTR: int
{
	TRACK,
	OFFSET,
	LENGTH,
	FILENAME,
	CHECKSUM_ARCS1,
	CHECKSUM_ARCS2,
	THEIRS,
	CONFIDENCE
};


/**
 * \brief Maximal occurring value for an ATTR.
 *
 * Must be less than sizeof(print_flag_t).
 */
constexpr int MAX_ATTR = 7;


/**
 * \brief Produce default label for a specified attribute.
 */
template<ATTR A>
std::string DefaultLabel();


using table::PrintableTable;
using table::StringTableLayout;
using table::CellDecorator;
using table::DecoratedStringTable;


/**
 * \brief Record-based interface for decorating a PrintableTable.
 *
 * This interface allows to decorate a table by declaring decoration on a
 * record- or field-level. He caller is not required to know whether the records
 * are rows or columns.
 */
class DecorationInterface
{
	virtual void do_register_to_record(const int record_idx,
			std::unique_ptr<CellDecorator> d)
	= 0;

	virtual const CellDecorator* do_on_record(const int record_idx) const
	= 0;

	virtual void do_register_to_field(const int field_idx,
			std::unique_ptr<CellDecorator> d)
	= 0;

	virtual const CellDecorator* do_on_field(const int field_idx) const
	= 0;

	virtual void do_mark(const int record_idx, const int field_idx)
	= 0;

	virtual void do_unmark(const int record_idx, const int field_idx)
	= 0;

public:

	/**
	 * \brief Register a decorator for a certain record.
	 *
	 * \param[in] record_idx Record index
	 * \param[in] d          Decorator to register
	 */
	void register_to_record(const int record_idx,
			std::unique_ptr<CellDecorator> d);

	/**
	 * \brief Get decorator for record.
	 *
	 * Iff there is no decorator registered for the specified record,
	 * \c nullptr is returned.
	 *
	 * \param[in] record_idx Record index to get decorator for, if any
	 */
	const CellDecorator* on_record(const int record_idx) const;

	/**
	 * \brief Register a decorator for a certain field.
	 *
	 * \param[in] field_idx Field index
	 * \param[in] d         Decorator to register
	 */
	void register_to_field(const int field_idx,
			std::unique_ptr<CellDecorator> d);

	/**
	 * \brief Get decorator for field.
	 *
	 * Iff there is no decorator registered for the specified field,
	 * \c nullptr is returned.
	 *
	 * \param[in] field_idx Field index to get decorator for, if any
	 */
	const CellDecorator* on_field(const int field_idx) const;

	/**
	 * \brief Mark cell as decorated.
	 *
	 * \param[in] record_idx Record index
	 * \param[in] field_idx  Field index
	 */
	void mark(const int record_idx, const int field_idx);

	/**
	 * \brief Unmark cell, e.g. mark cell as undecorated.
	 *
	 * \param[in] record_idx Record index
	 * \param[in] field_idx  Field index
	 */
	void unmark(const int record_idx, const int field_idx);
};


/**
 * \brief Interface for constructing a result table.
 *
 * A TableComposer provides an API by which the caller can construct and fill a
 * table via referring to records and their respective fields instead of rows
 * and columns.
 *
 * Whether the records are rows or columns in the resulting table is defined by
 * the concrete subclasses.
 */
class TableComposer :   public RecordInterface<DecoratedStringTable, ATTR>
					  , public DecorationInterface
{
public:

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~TableComposer() noexcept = default;

	/**
	 * \brief Get the fields of the table to construct.
	 *
	 * \return Field types in the order they appear in the table
	 */
	const std::vector<ATTR>& fields() const;

	/**
	 * \brief TRUE iff table to be composed has field \c a, otherwise FALSE.
	 *
	 * \param[in] f Field to check for
	 *
	 * \return TRUE iff instance will produce field \c a, otherwise FALSE
	 */
	bool has_field(const ATTR f) const;

	/**
	 * \brief Get the table row index.
	 *
	 * \param[in] record_idx Index of the record
	 * \param[in] field_idx  Index of the field
	 *
	 * \return Row index
	 */
	int get_row(const int record_idx, const int field_idx) const;

	/**
	 * \brief Get the table column index.
	 *
	 * \param[in] record_idx Index of the record
	 * \param[in] field_idx  Index of the field
	 *
	 * \return Column index
	 */
	int get_col(const int record_idx, const int field_idx) const;

	/**
	 * \brief Return the StringTable instance.
	 *
	 * \return StringTable
	 */
	std::unique_ptr<PrintableTable> table();

	/**
	 * \brief Set the layout for the result table.
	 *
	 * \param[in] layout The layout to use for the result table
	 */
	void set_layout(std::unique_ptr<StringTableLayout> layout);

protected:

	/**
	 * \brief Common constructor.
	 *
	 * \param[in] fields   Ordering of field_types
	 * \param[in] table    Table with dimensions for records and field_types
	 */
	TableComposer(const std::vector<ATTR>& fields,
			std::unique_ptr<DecoratedStringTable> table);

	/**
	 * \brief Write value to specified field in record.
	 *
	 * \param[in] record_idx  The record to modify
	 * \param[in] field_idx   The field to modify
	 *
	 * \return The string to write
	 */
	std::string& value(const int record_idx, const int field_idx);

	/**
	 * \brief Write to table.
	 *
	 * \return Reference to internal table
	 */
	DecoratedStringTable& in_table();

	/**
	 * \brief Read from table.
	 *
	 * \return Reference to internal table
	 */
	const DecoratedStringTable& from_table() const;

private:

	// RecordInterface

	void do_set_field(const int record_idx, const int field_idx,
			const std::string& str) final;

	const std::string& do_field(const int i, const ATTR& field_type)
		const final;

	void do_set_label_by_type(const ATTR& field_type, const std::string& label)
		final;

	std::string do_label_by_type(const ATTR& field_type) const final;

	virtual void do_set_label_by_index(const int field_idx,
			const std::string& label)
	= 0;

	virtual std::string do_label_by_index(const int field_idx) const
	= 0;

	int do_field_idx(const ATTR& field_type, const int i) const final;

	bool do_has_field(const ATTR& field_type) const final;

	// DecorationInterface

	void do_mark(const int record_idx, const int field_idx) final;

	void do_unmark(const int record_idx, const int field_idx) final;

	// TableComposer

	virtual int do_get_row(const int i, const int j) const
	= 0;

	virtual int do_get_col(const int i, const int j) const
	= 0;

	/**
	 * \brief List of fields in the order they appear in the record.
	 */
	std::vector<ATTR> fields_;
};


/**
 * \brief Build a table whose records are rows and the fields are columns.
 */
class RowTableComposer final : public TableComposer
{
	// RecordInterface

	size_type do_total_records() const final;
	size_type do_fields_per_record() const final;
	virtual void do_set_label_by_index(const int field_idx,
			const std::string& label) final;
	virtual std::string do_label_by_index(const int field_idx) const final;

	// DecorationInterface

	void do_register_to_record(const int record_idx,
			std::unique_ptr<CellDecorator> d) final;

	const CellDecorator* do_on_record(const int record_idx) const final;

	void do_register_to_field(const int field_idx,
			std::unique_ptr<CellDecorator> d) final;

	const CellDecorator* do_on_field(const int field_idx) const final;

	// TableComposer

	int do_get_row(const int i, const int j) const final;
	int do_get_col(const int i, const int j) const final;

public:

	RowTableComposer(const std::size_t records, const std::vector<ATTR>& order);
};


/**
 * \brief Build a table whose records are columns and the fields are rows.
 */
class ColTableComposer final : public TableComposer
{
	// RecordInterface

	size_type do_total_records() const final;
	size_type do_fields_per_record() const final;
	virtual void do_set_label_by_index(const int field_idx,
			const std::string& label) final;
	virtual std::string do_label_by_index(const int field_idx) const final;

	// DecorationInterface

	void do_register_to_record(const int record_idx,
			std::unique_ptr<CellDecorator> d) final;

	const CellDecorator* do_on_record(const int record_idx) const final;

	void do_register_to_field(const int field_idx,
			std::unique_ptr<CellDecorator> d) final;

	const CellDecorator* do_on_field(const int field_idx) const final;

	// TableComposer

	int do_get_row(const int i, const int j) const final;
	int do_get_col(const int i, const int j) const final;

public:

	ColTableComposer(const std::size_t records, const std::vector<ATTR>& order);
};


/**
 * \brief Create a TableComposer.
 */
class TableComposerBuilder
{
public:

	/**
	 * \brief Constructor.
	 */
	TableComposerBuilder();

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~TableComposerBuilder() noexcept = default;

	void set_label(ATTR, const std::string& label);

	std::string label(ATTR);

	/**
	 * \brief Create a TableComposer.
	 *
	 * If labels are activated, the default labels are printed.
	 *
	 * \param[in] records     Total number of records
	 * \param[in] field_types List of fields for each record
	 * \param[in] with_labels If TRUE, use default labels
	 *
	 * \see DefaultLabel
	 */
	std::unique_ptr<TableComposer> build(
		const std::size_t records,
		const std::vector<ATTR>& field_types, const bool with_labels) const;

protected:

	/**
	 * \brief Worker: assign each field its respective DefaultLabel.
	 *
	 * \param[in] c           TableComposer to use
	 * \param[in] field_types List of field types
	 */
	void assign_default_labels(TableComposer& c,
			const std::vector<ATTR>& field_types) const;

private:

	/**
	 * \brief Implements create_composer().
	 */
	virtual std::unique_ptr<TableComposer> do_build(
		const std::size_t records,
		const std::vector<ATTR>& field_types, const bool with_labels) const
	= 0;

	/**
	 * \brief Default field labels.
	 */
	std::map<ATTR, std::string> labels_;
};


/**
 * \brief Create a RowTableComposer.
 */
class RowTableComposerBuilder final : public TableComposerBuilder
{
	std::unique_ptr<TableComposer> do_build(
		const std::size_t records,
		const std::vector<ATTR>& field_types, const bool with_labels) const
		final;
};


/**
 * \brief Create a ColTableComposer.
 */
class ColTableComposerBuilder final : public TableComposerBuilder
{
	std::unique_ptr<TableComposer> do_build(
		const std::size_t records,
		const std::vector<ATTR>& field_types, const bool with_labels) const
		final;
};


/**
 * \brief A set of flags.
 */
template <typename T, typename S>
class Flags final
{
public:

	/**
	 * \brief Type for flags.
	 *
	 * Is an unsigned numeric type.
	 */
	using type = S;

	/**
	 * \brief TRUE iff value for parameter \c t is TRUE, otherwise FALSE.
	 *
	 * \param[in] t  Input value to check flag value for
	 *
	 * \return TRUE iff \c t has flag value TRUE, otherwise FALSE.
	 */
	bool operator() (const T t) const
	{
		return flags_ & (1 << std::underlying_type_t<T>(t));
	}

	/**
	 * \brief Set a flag for input value \c t.
	 *
	 * \param[in] t     Input to set value for
	 * \param[in] value Value to be set for \c t
	 */
	void set(const T t, const bool value)
	{
		flags_ |= (value << std::underlying_type_t<T>(t));
	}

private:

	/**
	 * \brief Internal flags.
	 */
	type flags_;
};


// Required for ResultFormatter


using arcstk::ARId;
using arcstk::Checksum;
using arcstk::Checksums;
using arcstk::ChecksumSource;
using arcstk::ChecksumSourceOf;
using arcstk::DBAR;
using arcstk::VerificationResult;
using arcstk::TOC;


/**
 * \brief Access list of reference values by block and index.
 */
class FromRefvalues final : public ChecksumSourceOf<std::vector<uint32_t>>
{
	ARId do_id(const ChecksumSource::size_type block_idx) const final;
	Checksum do_checksum(const ChecksumSource::size_type block_idx,
			const ChecksumSource::size_type idx) const final;
	const uint32_t& do_arcs_value(const ChecksumSource::size_type block_idx,
			const ChecksumSource::size_type track_idx) const final;
	const uint32_t& do_confidence(const ChecksumSource::size_type block_idx,
			const ChecksumSource::size_type track_idx) const final;
	const uint32_t& do_frame450_arcs_value(
			const ChecksumSource::size_type block_idx,
			const ChecksumSource::size_type track_idx) const final;
	std::size_t do_size(const ChecksumSource::size_type block_idx) const final;
	std::size_t do_size() const final;

public:

	using ChecksumSourceOf::ChecksumSourceOf;
	using ChecksumSourceOf::operator=;
};


/**
 * \brief Dummy source for providing only empty checksums.
 */
class EmptyChecksumSource final : public ChecksumSource
{
	static const auto zero = uint32_t { 0 };

	ARId do_id(const ChecksumSource::size_type block_idx) const final;
	Checksum do_checksum(const ChecksumSource::size_type block_idx,
			const ChecksumSource::size_type idx) const final;
	const uint32_t& do_arcs_value(const ChecksumSource::size_type block_idx,
			const ChecksumSource::size_type track_idx) const final;
	const uint32_t& do_confidence(const ChecksumSource::size_type block_idx,
			const ChecksumSource::size_type track_idx) const final;
	const uint32_t& do_frame450_arcs_value(
			const ChecksumSource::size_type block_idx,
			const ChecksumSource::size_type track_idx) const final;
	std::size_t do_size(const ChecksumSource::size_type block_idx) const final;
	std::size_t do_size() const final;

public:

	EmptyChecksumSource();
};


/**
 * \brief Functor for inserting a specified field.
 *
 * Concrete subclasses specify which field to create. The field is created for a
 * record specified by its index and built by the TableComposer passed.
 */
class FieldCreator
{
	virtual void do_create(TableComposer* c, const int record_idx)
		const
	= 0;

protected:

	/**
	 * \brief Track represented by the specified record index.
	 *
	 * \param[in] record_idx The record index to get the track for
	 */
	int track(const int record_idx) const;

public:

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~FieldCreator() noexcept = default;

	/**
	 * \brief Use specified TableComposer to create the field in the specified
	 * record.
	 *
	 * \param[in,out] c          The TableComposer to use
	 * \param[in]     record_idx The current record index
	 */
	void create(TableComposer* c, const int record_idx) const;
};


/**
 * \brief Creates records of a table.
 *
 * Accepts functors for adding fields to records and recors to the
 * TableComposer. Hence it is possible to "queue" the production of fields by
 * calling \c add_fields() and then produce the entire table by calling
 * \c create_records().
 */
class RecordCreator final
{
	/**
	 * \brief Internal list of creators for every field.
	 */
	std::vector<std::unique_ptr<FieldCreator>> fields_;

	/**
	 * \brief Internal table composer to add the fields.
	 */
	TableComposer* composer_;

	/**
	 * \brief Create a single record containing every field previously added.
	 *
	 * \param[in] record_idx The index of the record to create
	 */
	void create_record(const int record_idx) const;

public:

	/**
	 * \brief Create records using the specified TableComposer.
	 *
	 * \param[in] c TableComposer to be used
	 */
	RecordCreator(TableComposer* c);

	/**
	 * \brief Add creator for one or more fields.
	 *
	 * \param[in] f FieldCreator to add fields
	 */
	void add_fields(std::unique_ptr<FieldCreator> f);

	/**
	 * \brief Create all records.
	 */
	void create_records() const;
};


/**
 * \brief Abstract base class for result formatting.
 *
 * \todo Use a bitmask for flags of p_tracks, p_offsets, p_lengths, p_filenames
 */
class ResultFormatter : public WithInternalFlags
{
	template <enum ATTR> friend class AddField;

public:

	/**
	 * \brief Set the TableComposerBuilder to use.
	 *
	 * \param[in] c The TableComposerBuilder to use
	 */
	void set_builder(std::unique_ptr<TableComposerBuilder> c);

	/**
	 * \brief The TableComposerBuilder to use.
	 *
	 * \return The TableComposerBuilder to use
	 */
	const TableComposerBuilder* builder() const;

	/**
	 * \brief Set the layout to use for formatting the output table.
	 *
	 * \param[in] layout The StringTableLayout to set
	 */
	void set_table_layout(std::unique_ptr<StringTableLayout> layout);

	/**
	 * \brief Layout to use for formatting the output table.
	 *
	 * \return The StringTableLayout to use
	 */
	StringTableLayout copy_table_layout() const;

	/**
	 * \brief Set the layout to use for formatting the ARId.
	 *
	 * \param[in] arid_layout The ARIdLayout to set
	 */
	void set_arid_layout(std::unique_ptr<ARIdLayout> arid_layout);

	/**
	 * \brief Layout to use for formatting the ARId.
	 *
	 * \return The internal ARIdLayout
	 */
	const ARIdLayout* arid_layout() const;

	/**
	 * \brief Set the layout for printing the checksums
	 *
	 * \param[in] layout Layout for printing the checksums
	 */
	void set_checksum_layout(std::unique_ptr<ChecksumLayout> layout);

	/**
	 * \brief Return the layout for printing the checksums
	 *
	 * \return Layout for printing the checksums
	 */
	const ChecksumLayout* checksum_layout() const;

	/**
	 * \brief Return TRUE iff instance is configured to format the label.
	 *
	 * Intended to control the printing of column titles and row labels.
	 *
	 * \return Flag for printing the label
	 */
	bool formats_label() const;

	/**
	 * \brief Activate or deactivate the printing of labels.
	 *
	 * Intended to control the printing of column titles and row labels.
	 *
	 * \param[in] label Flag to set for printing the labels
	 */
	void format_label(const bool &label);

	/**
	 * \brief TRUE iff data attribute \c a is to be formatted by this instance.
	 *
	 * \return TRUE iff \c a is formatted by this instance, otherwise FALSE.
	 */
	bool formats_data(const ATTR a) const;

	/**
	 * \brief Set data type to be formatted in the output.
	 *
	 * Iff data type \c d is set to false, it will not be contained in the
	 * printed output.
	 *
	 * \param[in] a     Data attributes to be formatted or not
	 * \param[in] value Flag value to activate or deactivate formatting of \c d
	 */
	void format_data(const ATTR a, const bool value);

protected:

	/**
	 * \brief Type for print flags.
	 *
	 * Is an unsigned numeric type.
	 */
	using print_flag_t = Flags<ATTR, uint8_t>;

	/**
	 * \brief Worker: TRUE iff field \c f is requested for output.
	 *
	 * \param[in] a Field to check
	 *
	 * \return TRUE iff \c f is requested for output, otherwise FALSE
	 */
	bool is_requested(const ATTR f) const;

	/**
	 * \brief Worker: produce print flags for all printable attributes.
	 *
	 * \param[in] toc       TOC available
	 * \param[in] filenames Filenames available
	 *
	 * \return Print flags for all printable attributes
	 */
	print_flag_t create_print_flags(const TOC* toc,
			const std::vector<std::string>& filenames) const;

	/**
	 * \brief Respect flags to create or skip the optional fields.
	 *
	 * This creates TRACK, OFFSET, LENGTH and FILENAME fields in accordance to
	 * the print flags passed.
	 *
	 * \param[in] print_flags Print flags to respect
	 *
	 * \return List of field types
	 */
	std::vector<ATTR> create_optional_fields(const print_flag_t print_flags)
		const;

	/**
	 * \brief Populate the list of common FieldCreators.
	 *
	 * These are the TRACK, OFFSET, LENGTH and FILENAME fields.
	 *
	 * \param[in] field_creators List of field creators
	 * \param[in] field_types    List of fields to format for print
	 * \param[in] toc            TOC for calculated checksums
	 * \param[in] checksums      Calculated checksums
	 * \param[in] filenames      List of input filenames
	 */
	void populate_common_creators(
			std::vector<std::unique_ptr<FieldCreator>>& field_creators,
			const std::vector<ATTR>& field_types, const TOC& toc,
			const Checksums& checksums,
			const std::vector<std::string>& filenames) const;

	/**
	 * \brief Create the internal TableComposer to compose the result data.
	 *
	 * Uses the internal TableComposer instance.
	 *
	 * \param[in] total_records Number of records to print
	 * \param[in] field_types   List of fields to format for print
	 * \param[in] with_labels   Decide whether to print field labels
	 *
	 * \return TableComposer with specified field list, size and labels
	 */
	std::unique_ptr<TableComposer> create_composer(
		const std::size_t total_records,
		const std::vector<ATTR>& field_types, const bool with_labels) const;

	/**
	 * \brief Hook in format_table() for initializing composer.
	 *
	 * The hook is called after create_composer() is called.
	 *
	 * You can do things like adding decorators or do some checking/validation,
	 * changing default settings or other things.
	 *
	 * Default implementation is empty.
	 *
	 * \param[in] c TableComposer to be initialized
	 */
	void init_composer(TableComposer& c) const;

	/**
	 * \brief Actually format the result table.
	 *
	 * \param[in] field_types    List of fields to format for print
	 * \param[in] total_records  Number of records to print
	 * \param[in] with_labels    Decide whether to print field labels
	 * \param[in] field_creators List of field creators
	 *
	 * \return Result table
	 */
	std::unique_ptr<Result> format_table(
		const std::vector<ATTR>& field_types,
		const std::size_t total_records,
		const bool with_labels,
		std::vector<std::unique_ptr<FieldCreator>>& field_creators) const;

	/**
	 * \brief Build an ARId enriched with print information.
	 *
	 * \param[in] toc        TOC by which \c arid was created
	 * \param[in] arid       Actual ARId
	 * \param[in] alt_prefix Alternative URL prefix
	 *
	 * \return ARId container
	 */
	RichARId build_id(const TOC* toc, const ARId& arid,
		const std::string& alt_prefix) const;

	/**
	 * \brief Print my checksums.
	 *
	 * Used by build_table().
	 *
	 * Note that \c record also determines access to \c checksums.
	 *
	 * \param[in] checksum   Local checksum to be formatted
	 * \param[in] record     Index of the record in \c c to edit
	 * \param[in] field      Index of the field in \c c to edit
	 * \param[in] c          TableComposer to use
	 */
	void mine_checksum(const Checksum& checksum,
		const int record, const int field, TableComposer* c) const;

	/**
	 * \brief Print their checksums.
	 *
	 * Used by build_table().
	 *
	 * Note that \c record also determines access to \c checksums.
	 *
	 * \param[in] checksums  Reference checksum to be formatted
	 * \param[in] does_match Print as matching or as not matching
	 * \param[in] record     Index of the record in \c b to edit
	 * \param[in] field      Index of the field in \c b to edit
	 * \param[in] c          TableComposer to use
	 */
	void their_checksum(const Checksum& checksums, const bool does_match,
		const int record, const int field, TableComposer* c) const;

	/**
	 * \brief Worker: print a checksum to the result object.
	 *
	 * If checksum_layout() is available for formatting the checksums, it
	 * is used, otherwise the fitting implementation of operator '<<' is
	 * picked (which could be libarcstk's).
	 *
	 * \param[in] checksum  Result checksum
	 *
	 * \return Formatted checksum
	 */
	std::string checksum(const Checksum& checksum) const;

	/**
	 * \brief Validate the result objects common to every result.
	 *
	 * Throws if validation fails.
	 *
	 * \param[in] checksums  Checksums as resulted
	 * \param[in] toc        TOC as resulted
	 * \param[in] arid       ARId as resulted
	 * \param[in] filenames  Filenames as resulted
	 *
	 * \throws invalid_argument If validation fails
	 */
	void validate(const Checksums& checksums, const TOC* toc,
		const ARId& arid, const std::vector<std::string>& filenames) const;

private:

	virtual void do_init_composer(TableComposer& c) const;

	/**
	 * \brief Implements mine_checksum().
	 */
	virtual void do_mine_checksum(const Checksum& checksum,
		const int record, const int field, TableComposer* c) const;

	/**
	 * \brief Called by their_checksum() on a match.
	 */
	virtual void do_their_match(const Checksum& checksum, const int record,
			const int field, TableComposer* c) const;

	/**
	 * \brief Called by their_checksum() on a mismatch.
	 */
	virtual void do_their_mismatch(const Checksum& checksum, const int record,
			const int field, TableComposer* c) const;

	/**
	 * \brief Internal TableComposerBuilder.
	 */
	std::unique_ptr<TableComposerBuilder> builder_creator_;

	/**
	 * \brief Format for the result StringTable.
	 */
	std::unique_ptr<StringTableLayout> table_layout_;

	/**
	 * \brief Format for the ARId.
	 */
	std::unique_ptr<ARIdLayout> arid_layout_;

	/**
	 * \brief Format for the Checksums.
	 */
	std::unique_ptr<ChecksumLayout> checksum_layout_;
};


/**
 * \brief Functor for adding fields for the specified attribute in a table.
 *
 * Specializations of AddField are FieldCreators that specify the field they
 * create by the attribute.
 */
template <enum ATTR>
class AddField
{
public:

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~AddField() noexcept = default;
};



template <>
class AddField<ATTR::TRACK> final : public FieldCreator
{
	void do_create(TableComposer* c, const int record_idx) const final;
};


template <>
class AddField<ATTR::OFFSET> final : public FieldCreator
{
	const TOC* toc_;

	void do_create(TableComposer* c, const int record_idx) const final;

public:

	AddField(const TOC* toc);
};


template <>
class AddField<ATTR::LENGTH> final : public FieldCreator
{
	const Checksums* checksums_;

	void do_create(TableComposer* c, const int record_idx) const final;

public:

	AddField(const Checksums* checksums);
};


template <>
class AddField<ATTR::FILENAME> final : public FieldCreator
{
	const std::vector<std::string>* filenames_;

	void do_create(TableComposer* c, const int record_idx) const final;

public:

	AddField(const std::vector<std::string>* filenames);
};


template <>
class AddField<ATTR::CHECKSUM_ARCS1> final : public FieldCreator
{
	const Checksums* checksums_;
	const ResultFormatter* formatter_;

	void do_create(TableComposer* c, const int record_idx) const final;

public:

	AddField(const Checksums* checksums, const ResultFormatter* formatter);
};


template <>
class AddField<ATTR::CHECKSUM_ARCS2> final : public FieldCreator
{
	const Checksums* checksums_;
	const ResultFormatter* formatter_;

	void do_create(TableComposer* c, const int record_idx) const final;

public:

	AddField(const Checksums* checksums, const ResultFormatter* formatter);
};


/**
 * \brief Creates Theirs-columns with optional Confidence-columns
 */
template <>
class AddField<ATTR::THEIRS> final : public FieldCreator
{
	const VerificationResult* vresult_;
	const int block_;
	const ChecksumSource* checksums_;
	const std::vector<arcstk::checksum::type>* types_to_print_;
	const ResultFormatter* formatter_;
	const int total_theirs_per_block_;
	const bool print_confidence_;

	void do_create(TableComposer* c, const int record_idx) const final;

public:

	AddField(const std::vector<arcstk::checksum::type>* types,
			const VerificationResult* vresult,
			const int block,
			const ChecksumSource* checksums,
			const ResultFormatter* formatter,
			const int total_theirs_per_block,
			const bool print_confidence);
};


} // namespace arcsapp

#endif

