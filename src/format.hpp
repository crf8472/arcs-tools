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
#ifndef __LIBARCSTK_MATCH_HPP__
#include <arcstk/match.hpp>       // for Match
#endif
#ifndef __LIBARCSTK_PARSE_HPP__
#include <arcstk/parse.hpp>       // for ARResponse
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
	inline void set_field(const int i, const F& field_type,
			const std::string& value)
	{
		this->do_set_field(i, this->field_idx(field_type), value);
	}

	/**
	 * \brief Set the value for the specified field in record \c i, field \c j.
	 */
	inline void set_field(const int i, const int j,
			const std::string& value)
	{
		this->do_set_field(i, j, value);
	}

	/**
	 * \brief The value of the specified field.
	 */
	inline std::string field(const int i, const F& field_type) const
	{
		return this->do_field(i, field_type);
	}

	/**
	 * \brief Set the label for the specified field type.
	 */
	inline void set_label(const F& field_type, const std::string& label)
	{
		this->do_set_label(field_type, label);
	}

	/**
	 * \brief Label for the specified field type.
	 */
	inline std::string label(const F& field_type) const
	{
		return this->do_label(field_type);
	}

	/**
	 * \brief First index of specified field type.
	 *
	 * \param[in] field_type  Type of the field
	 */
	inline int field_idx(const F& field_type) const
	{
		return this->do_field_idx(field_type, 1);
	}

	/**
	 * \brief I-th index of specified field type.
	 *
	 * \param[in] field_type  Type of the field
	 * \param[in] i           Occurrence of the field_type
	 */
	inline int field_idx(const F& field_type, const int i) const
	{
		return this->do_field_idx(field_type, i);
	}

	/**
	 * \brief TRUE iff specified field type is part of the result.
	 */
	inline bool has_field(const F& field_type) const
	{
		return this->do_has_field(field_type);
	}

	/**
	 * \brief Total number of records.
	 */
	inline size_type total_records() const
	{
		return this->do_total_records();
	}

	/**
	 * \brief Total number of fields per record.
	 */
	inline size_type fields_per_record() const
	{
		return this->do_fields_per_record();
	}

	/**
	 * \brief Return the object holding the records.
	 *
	 * \return Object with records
	 */
	inline const T& object() const
	{
		return *object_;
	}

	/**
	 * \brief Return and remove the object holding the records.
	 *
	 * \return Object with records
	 */
	inline std::unique_ptr<T> remove_object()
	{
		auto p { std::move(object_) };
		return p;
	}

protected:

	inline RecordInterface(std::unique_ptr<T> object)
		: object_ { std::move(object) }
	{
		// empty
	}

	/**
	 * \brief Read or manipulate the object holding the records.
	 */
	inline T& to_object()
	{
		return *object_;
	}

private:

	virtual void do_set_field(const int record_idx, const int field_idx,
			const std::string& str)
	= 0;

	virtual const std::string& do_field(const int i, const F& field_type) const
	= 0;

	virtual void do_set_label(const F& field_type, const std::string& label)
	= 0;

	virtual std::string do_label(const F& field_type) const
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
	THEIRS
};


/**
 * \brief Maximal occurring value for an ATTR.
 *
 * Must be less than sizeof(print_flags_t).
 */
constexpr int MAX_ATTR = 6;


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
	 * \brief Get the fields of the table to construct.
	 *
	 * \return Field types in the order they appear in the table
	 */
	const std::vector<ATTR>& fields() const;

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

	/**
	 * \brief Assign each field its respective label.
	 *
	 * The labels are composed from the default names of the attribute labels.
	 */
	void assign_labels(const std::vector<ATTR>& field_types);

private:

	// RecordInterface

	void do_set_field(const int record_idx, const int field_idx,
			const std::string& str) final;

	const std::string& do_field(const int i, const ATTR& field_type)
		const final;

	void do_set_label(const ATTR& field_type, const std::string& label) final;

	std::string do_label(const ATTR& field_type) const final;

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

	virtual void set_field_label(const int field_idx, const std::string& label)
	= 0;

	virtual std::string field_label(const int field_idx) const
	= 0;

	/**
	 * \brief List of fields in the order they appear in the record.
	 */
	std::vector<ATTR> fields_;

	/**
	 * \brief Default field labels.
	 */
	std::map<ATTR, std::string> labels_;
};


/**
 * \brief Build a table whose records are rows and the fields are columns.
 */
class RowTableComposer final : public TableComposer
{
	// RecordInterface

	size_type do_total_records() const final;
	size_type do_fields_per_record() const final;

	// DecorationInterface

	void do_register_to_record(const int record_idx,
			std::unique_ptr<CellDecorator> d) final;

	const CellDecorator* do_on_record(const int record_idx) const final;

	void do_register_to_field(const int field_idx,
			std::unique_ptr<CellDecorator> d) final;

	const CellDecorator* do_on_field(const int field_idx) const final;

	// TableComposer

	void set_field_label(const int field_idx, const std::string& label) final;
	std::string field_label(const int field_idx) const final;
	int do_get_row(const int i, const int j) const final;
	int do_get_col(const int i, const int j) const final;

public:

	RowTableComposer(const std::size_t records,
		const std::vector<ATTR>& order, const bool with_labels);
};


/**
 * \brief Build a table whose records are columns and the fields are rows.
 */
class ColTableComposer final : public TableComposer
{
	// RecordInterface

	size_type do_total_records() const final;
	size_type do_fields_per_record() const final;

	// DecorationInterface

	void do_register_to_record(const int record_idx,
			std::unique_ptr<CellDecorator> d) final;

	const CellDecorator* do_on_record(const int record_idx) const final;

	void do_register_to_field(const int field_idx,
			std::unique_ptr<CellDecorator> d) final;

	const CellDecorator* do_on_field(const int field_idx) const final;

	// TableComposer

	void set_field_label(const int field_idx, const std::string& label) final;
	std::string field_label(const int field_idx) const final;
	int do_get_row(const int i, const int j) const final;
	int do_get_col(const int i, const int j) const final;

public:

	ColTableComposer(const std::size_t records,
		const std::vector<ATTR>& order, const bool with_labels);
};


/**
 * \brief Create a TableComposer.
 */
class TableComposerBuilder
{
public:

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

private:

	/**
	 * \brief Implements create_composer().
	 */
	virtual std::unique_ptr<TableComposer> do_build(
		const std::size_t records,
		const std::vector<ATTR>& field_types, const bool with_labels) const
	= 0;
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
	inline bool operator() (const T t) const
	{
		return flags_ & (1 << std::underlying_type_t<T>(t));
	}

	/**
	 * \brief Set a flag for input value \c t.
	 *
	 * \param[in] t     Input to set value for
	 * \param[in] value Value to be set for \c t
	 */
	inline void set(const T t, const bool value)
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
using arcstk::ARResponse;
using arcstk::Checksum;
using arcstk::Checksums;
using arcstk::Match;
using arcstk::TOC;


/**
 * \brief Functor for inserting a specified field.
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
	int track(const int record_idx) const
	{
		return record_idx + 1;
	}

public:

	/**
	 * \brief Use specified TableComposer to create the field in the specified
	 * record.
	 *
	 * \param[in] c          The TableComposer to use
	 * \param[in] record_idx The current record index
	 */
	void create(TableComposer* c, const int record_idx) const
	{
		this->do_create(c, record_idx);
	}
};


/**
 * \brief Interface for checksum sources.
 */
class ChecksumSource
{
	virtual Checksum do_read(const int block_idx, const int idx) const
	= 0;

public:

	/**
	 * \brief Read checksum \c idx in section with the specified \c block_idx.
	 */
	Checksum read(const int block_idx, const int idx) const
	{
		return this->do_read(block_idx, idx);
	}
};


/**
 * \brief Provide unified access method to checksum containers.
 */
template <typename T>
class GetChecksum : public ChecksumSource
{
	const T* checksum_source_;

protected:

	/**
	 * \brief The checksum source.
	 *
	 * \return The checksum source.
	 */
	const T* source() const
	{
		return checksum_source_;
	}

public:

	/**
	 * \brief Constructor.
	 *
	 * \param[in] t The primary checksum source
	 */
	GetChecksum(const T* t)
		: checksum_source_ { t }
	{
		// empty
	}
};


/**
 * \brief Access an ARResponse by block and index.
 */
class FromResponse final : public GetChecksum<ARResponse>
{
	using GetChecksum::GetChecksum;
	Checksum do_read(const int block_idx, const int idx) const final;
};


/**
 * \brief Access references values by block and index.
 */
class FromRefvalues final : public GetChecksum<std::vector<Checksum>>
{
	using GetChecksum::GetChecksum;
	Checksum do_read(const int block_idx, const int idx) const final;
};


/**
 * \brief Dummy source for providing only empty checksums.
 */
class EmptyChecksums final : public ChecksumSource
{
	Checksum do_read(const int block_idx, const int idx) const final;
};


/**
 * \brief Functor for filling fields in the table.
 */
template <enum ATTR>
class AddField
{
	// empty
};


template <>
class AddField<ATTR::CHECKSUM_ARCS1>;

template <>
class AddField<ATTR::CHECKSUM_ARCS2>;

template <>
class AddField<ATTR::THEIRS>;


/**
 * \brief Abstract base class for result formatting.
 *
 * \todo Use a bitmask for flags of p_tracks, p_offsets, p_lengths, p_filenames
 */
class ResultFormatter : public WithInternalFlags
{
	friend AddField<ATTR::CHECKSUM_ARCS1>;
	friend AddField<ATTR::CHECKSUM_ARCS2>;
	friend AddField<ATTR::THEIRS>;

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
	 * \param[in] table_layout The StringTableLayout to set
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
	 * \brief Set the list of requested checksum types.
	 *
	 * \param[in] types List of requested checksum types.
	 */
	void set_types_to_print(std::vector<arcstk::checksum::type> types);

	/**
	 * \brief List of requested checksum types.
	 *
	 * \return List of requested checksum types.
	 */
	std::vector<arcstk::checksum::type> types_to_print() const;

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
	 * \brief TRUE iff attribute \c a is requested for output.
	 */
	bool is_requested(const ATTR a) const;

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
	 * \brief Build the result representation.
	 *
	 * Calls create_attributes() to determine and create the field_types for the
	 * result object, create_composer() to instantiate the TableComposer
	 * instance, and build_table() to populate the result table with the
	 * relevant data.
	 *
	 * \param[in] match          Match between calculation and reference
	 * \param[in] block          Best block in \c diff
	 * \param[in] checksums      Calculated checksums
	 * \param[in] response       Reference checksums
	 * \param[in] refvalues      Reference checksums
	 * \param[in] arid           ARId of input
	 * \param[in] toc            TOC for calculated checksums
	 * \param[in] alt_prefix     Alternative AccurateRip URL prefix
	 * \param[in] filenames      List of input filenames
	 * \param[in] types_to_print List of Checksum types requested for print
	 */
	std::unique_ptr<Result> build_result(
		const Match* match,
		const int block,
		const Checksums& checksums,
		const ARId& arid,
		const TOC* toc,
		const ARResponse& response,
		const std::vector<Checksum>& refvalues,
		const std::vector<std::string>& filenames,
		const std::string& alt_prefix,
		const std::vector<arcstk::checksum::type>& types_to_print) const;

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

	/**
	 * \brief Create the result \c field_types.
	 *
	 * \param[in] print_flags  Flags to instruct print of data
	 * \param[in] types        List of checksum types to print
	 *
	 * \return Sequence of result field_types to form an record
	 */
	std::vector<ATTR> create_attributes(const print_flag_t print_flags,
		const std::vector<arcstk::checksum::type>& types,
		const int total_theirs) const;

	/**
	 * \brief Create the internal TableComposer to compose the result data.
	 *
	 * Uses the internal TableComposer instance.
	 */
	std::unique_ptr<TableComposer> create_composer(
		const std::size_t records,
		const std::vector<ATTR>& field_types, const bool with_labels) const;

	/**
	 * \brief Hook for initializing composer.
	 *
	 * You can do things like adding decorators or do some checking/validation.
	 * Default implementation is empty.
	 *
	 * \param[in] c TableComposer to be initialized
	 */
	virtual void init_composer(TableComposer* c) const;

	/**
	 * \brief Build an ARId.
	 */
	RichARId build_id(const TOC* toc, const ARId& arid,
		const std::string& alt_prefix) const;

	/**
	 * \brief Build the result table.
	 *
	 * \param[in] match       Match object (maybe null)
	 * \param[in] block       Index to choose from \c match
	 * \param[in] checksums   Checksums as resulted
	 * \param[in] arid        ARId as resulted
	 * \param[in] toc         TOC as resulted
	 * \param[in] response    Reference checksums from AccurateRip response
	 * \param[in] refvalues   Reference checksums as specified
	 * \param[in] filenames   Filenames as resulted
	 * \param[in] types       List of checksum types to print
	 * \param[in] print_flags Flags to instruct print of data
	 *
	 * \return Table with result data
	 */
	std::unique_ptr<PrintableTable> build_table(
		const Match* match,
		const int block,
		const Checksums& checksums,
		const ARId& arid,
		const TOC* toc,
		const ARResponse& response,
		const std::vector<Checksum>& refvalues,
		const std::vector<std::string>& filenames,
		const std::vector<arcstk::checksum::type>& types,
		const print_flag_t print_flags) const;

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
	 * \param[in] checksum   Reference checksum to be formatted
	 * \param[in] does_match Print as matching or as not matching
	 * \param[in] record     Index of the record in \c b to edit
	 * \param[in] field      Index of the field in \c b to edit
	 * \param[in] c          TableComposer to use
	 */
	void their_checksum(const Checksum& checksums, const bool does_match,
		const int record, const int field, TableComposer* c) const;

	/**
	 * \brief Worker for printing a checksum to the result object.
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

private:

	virtual std::vector<ATTR> do_create_attributes(
		const print_flag_t print_flags,
		const std::vector<arcstk::checksum::type>& types_to_print,
		const int total_theirs) const
	= 0;

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

	/**
	 * \brief List of requested checksum types.
	 */
	std::vector<arcstk::checksum::type> types_;
};


// RecordCreator


/**
 * \brief Creates records.
 *
 * Accepts functors for adding fields to the TableComposer. Hence it is possible
 * to "queue" the production of fields by calling \c add_fields() and then
 * produce the entire table when calling \c create_records().
 */
class RecordCreator final
{
	std::vector<std::unique_ptr<FieldCreator>> fields_;
	TableComposer* composer_;

	/**
	 * \brief Create a record containing every field previously added.
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
	 * \brief Add field.
	 */
	void add_fields(std::unique_ptr<FieldCreator> f);

	/**
	 * \brief Create specified number of records.
	 */
	void create_records() const;
};


// Specializations for AddField


template <>
class AddField<ATTR::TRACK> final : public FieldCreator
{
	void do_create(TableComposer* c, const int record_idx) const
	{
		using std::to_string;
		c->set_field(record_idx, ATTR::TRACK,
				to_string(this->track(record_idx)));
	}
};


template <>
class AddField<ATTR::OFFSET> final : public FieldCreator
{
	const TOC* toc_;

	void do_create(TableComposer* c, const int record_idx) const
	{
		using std::to_string;
		c->set_field(record_idx, ATTR::OFFSET,
				to_string(toc_->offset(track(record_idx))));
	}

public:

	AddField(const TOC* toc) : toc_ { toc } { /* empty */ }
};


template <>
class AddField<ATTR::LENGTH> final : public FieldCreator
{
	const Checksums* checksums_;

	void do_create(TableComposer* c, const int record_idx) const
	{
		using std::to_string;
		c->set_field(record_idx, ATTR::LENGTH,
				to_string((*checksums_)[record_idx].length()));
	}

public:

	AddField(const Checksums* checksums) : checksums_ { checksums }
	{ /* empty */ }
};


template <>
class AddField<ATTR::FILENAME> final : public FieldCreator
{
	const std::vector<std::string>* filenames_;

	void do_create(TableComposer* c, const int record_idx) const
	{
		if (filenames_->size() > 1)
		{
			c->set_field(record_idx, ATTR::FILENAME, filenames_->at(record_idx));
		} else
		{
			c->set_field(record_idx, ATTR::FILENAME, *(filenames_->begin()));
		}
	}

public:

	AddField(const std::vector<std::string>* filenames)
		: filenames_ { filenames }
	{ /* empty */ }
};


template <>
class AddField<ATTR::CHECKSUM_ARCS1> final : public FieldCreator
{
	const Checksums* checksums_;
	const ResultFormatter* formatter_;

	void do_create(TableComposer* c, const int record_idx) const
	{
		formatter_->mine_checksum(
				checksums_->at(record_idx).get(arcstk::checksum::type::ARCS1),
				record_idx, c->field_idx(ATTR::CHECKSUM_ARCS1), c);
	}

public:

	AddField(const Checksums* checksums, const ResultFormatter* formatter)
		: checksums_ { checksums }
		, formatter_ { formatter }
	{ /* empty */ }
};


template <>
class AddField<ATTR::CHECKSUM_ARCS2> final : public FieldCreator
{
	const Checksums* checksums_;
	const ResultFormatter* formatter_;

	void do_create(TableComposer* c, const int record_idx) const
	{
		formatter_->mine_checksum(
				checksums_->at(record_idx).get(arcstk::checksum::type::ARCS2),
				record_idx, c->field_idx(ATTR::CHECKSUM_ARCS2), c);
	}

public:

	AddField(const Checksums* checksums, const ResultFormatter* formatter)
		: checksums_ { checksums }
		, formatter_ { formatter }
	{ /* empty */ }
};


template <>
class AddField<ATTR::THEIRS> final : public FieldCreator
{
	const Match* match_;
	const ChecksumSource*  checksums_;
	const std::vector<arcstk::checksum::type>* types_to_print_;
	const ResultFormatter* formatter_;
	const int total_theirs_per_block_;

	void do_create(TableComposer* c, const int record_idx) const
	{
		auto block_idx  = int { 0 }; // Iterate over blocks of checksums
		auto curr_type  { types_to_print_->at(0) }; // Current checksum type
		auto does_match = bool { false }; // Is current checksum a match?

		// Total number of THEIRS fields in the entire record type
		const auto total_theirs =
			total_theirs_per_block_ * types_to_print_->size();

		// Create all theirs fields
		for (auto b = int { 0 }; b < total_theirs; ++b)
		{
			block_idx = b % total_theirs_per_block_;

			curr_type =
				types_to_print_->at(std::ceil(b / total_theirs_per_block_));

			does_match = match_->track(block_idx, record_idx,
							curr_type == arcstk::checksum::type::ARCS2);

			formatter_->their_checksum(
					checksums_->read(block_idx, record_idx), does_match,
					record_idx, c->field_idx(ATTR::THEIRS, b + 1), c);
		}
	}

public:

	AddField(const Match* match,
			const ChecksumSource* checksums,
			const std::vector<arcstk::checksum::type>* types,
			const ResultFormatter* formatter,
			const int total_theirs_per_block)
		: match_ { match }
		, checksums_ { checksums }
		, types_to_print_ { types }
		, formatter_ { formatter }
		, total_theirs_per_block_ { total_theirs_per_block }
	{ /* empty */ }
};


} // namespace arcsapp

#endif

