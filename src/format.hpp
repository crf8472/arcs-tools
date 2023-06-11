#ifndef __ARCSTOOLS_FORMAT_HPP__
#define __ARCSTOOLS_FORMAT_HPP__

/**
 * \file
 *
 * \brief Formatter for result objects.
 */

#include <cstddef>      // for size_t
#include <cstdint>      // for uint32_t
#include <map>          // for map
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
#include "table.hpp"        // for PrintableTable, StringTable, StringTableLayout
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
 * \brief Produce default label for a specified attribute.
 */
template<ATTR A>
std::string DefaultLabel();


/**
 * \brief Decorator for table cells.
 *
 * A decorator can be registered for rows or columns. It gets an index \c i
 * which can be either a row or column index and decorates the corresponding
 * string passed, iff \c i is set to be decorated.
 */
class CellDecorator
{
	/**
	 * \brief Internal decoration flags.
	 */
	std::vector<bool> flags_;

	virtual std::string do_decorate(std::string&& s) const
	= 0;

	virtual std::unique_ptr<CellDecorator> do_clone() const
	= 0;

protected:

	/**
	 * \brief Copy constructor for subclasses.
	 *
	 * \param[in] rhs Instance to copy
	 */
	CellDecorator(const CellDecorator& rhs);

public:

	/**
	 * \brief Constructor for a decorator with \c n entries.
	 */
	CellDecorator(const std::size_t n);

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~CellDecorator() noexcept = default;

	/**
	 * \brief Set index \c i to be decorated.
	 *
	 * \param[in] i Index to be decorated
	 */
	void set(const int i);

	/**
	 * \brief Set index \c i to not be decorated.
	 *
	 * \param[in] i Index to not be decorated
	 */
	void unset(const int i);

	/**
	 * \brief Return TRUE iff index \c i is set, otherwise FALSE.
	 *
	 * \param[in] i Index to check
	 *
	 * \return TRUE iff index \c i is set, otherwise FALSE
	 */
	bool is_set(const int i) const;

	/**
	 * \brief Decorate \c s iff index \c i is set.
	 *
	 * \param[in] i Index to be decorated
	 * \param[in] s String to decorate
	 *
	 * \return Decorated string
	 */
	std::string decorate(const int i, std::string&& s) const;

	/**
	 * \brief Clone this instance.
	 *
	 * \return Deep copy of this instance.
	 */
	std::unique_ptr<CellDecorator> clone() const;
};


using table::PrintableTable;
using table::StringTable;
using table::StringTableLayout;


class DecoratedStringTable;
bool operator==(const DecoratedStringTable& lhs,
		const DecoratedStringTable& rhs);


/**
 * \brief PrintableTable with decorators attached.
 */
class DecoratedStringTable final : public PrintableTable
{
	/**
	 * \brief Inner table.
	 */
	std::unique_ptr<StringTable> table_;

	/**
	 * \brief Registry for decorators registered for rows or columns.
	 */
	std::unique_ptr<std::map<int, std::unique_ptr<CellDecorator>>> registry_;


	friend bool operator==(const DecoratedStringTable& lhs,
		const DecoratedStringTable& rhs);

public:

	/**
	 * \brief Constructor.
	 *
	 * \param[in] title Title of the table
	 * \param[in] rows  Number of rows
	 * \param[in] cols  Number of columns
	 */
	DecoratedStringTable(const std::string &title, const int rows,
			const int cols);

	/**
	 * \brief Constructor.
	 *
	 * Title of the table will be an empty string.
	 *
	 * \param[in] rows  Number of rows
	 * \param[in] cols  Number of columns
	 */
	DecoratedStringTable(const int rows, const int cols);

	/**
	 * \brief Default destructor.
	 */
	~DecoratedStringTable() noexcept final = default;

	/**
	 * \brief Write to cell by row and column.
	 *
	 * A std::runtime_error is thrown if the cell does not exist in the
	 * table.
	 *
	 * \param[in] row  Table row to access
	 * \param[in] col  Table column to access
	 *
	 * \return Content of the cell
	 */
	std::string& operator() (int row, int col);

	/**
	 * \brief Register a decorator for column \c j.
	 *
	 * \param[in] j Column to register a decorator for
	 * \param[in] d CellDecorator to be registered
	 */
	void register_to_col(const int j, std::unique_ptr<CellDecorator> d);

	/**
	 * \brief Return decorator for column \c j or nullptr.
	 *
	 * \param[in] j Column to get the decorator for
	 *
	 * \return CellDecorator for column \c j
	 */
	const CellDecorator* col_decorator(const int j) const;

	/**
	 * \brief Register a decorator for row \c i.
	 *
	 * \param[in] i Row to register a decorator for
	 * \param[in] d CellDecorator to be registered
	 */
	void register_to_row(const int i, std::unique_ptr<CellDecorator> d);

	/**
	 * \brief Return decorator for row \c i or nullptr.
	 *
	 * \param[in] i Row to get the decorator for
	 *
	 * \return CellDecorator for row \c i
	 */
	const CellDecorator* row_decorator(const int i) const;

	/**
	 * \brief Set cell \c i, \c j to be decorated.
	 *
	 * \param[in] i Row
	 * \param[in] j Column
	 */
	void decorate(const int i, const int j);

	/**
	 * \brief Set cell \c i, \c j to not be decorated.
	 *
	 * \param[in] i Row
	 * \param[in] j Column
	 */
	void undecorate(const int i, const int j);

	/**
	 * \brief Set the layout to be use for printing.
	 *
	 * \param[in] l Layout for this table
	 */
	void set_layout(std::unique_ptr<StringTableLayout> l);

	/**
	 * \brief Set a label for the specified column.
	 *
	 * \param[in] col   Table column to set a label to
	 * \param[in] label Label to set
	 */
	void set_col_label(int col, const std::string& label);

	/**
	 * \brief Set a label for the specified row.
	 *
	 * \param[in] row   Table row to set a label to
	 * \param[in] label Label to set
	 */
	void set_row_label(int row, const std::string& label);

	/**
	 * \brief Set the alignment for the respective column.
	 *
	 * \param[in] col   The column to modify
	 * \param[in] align The alignment to apply to the column
	 */
	void set_align(int col, table::Align align);

	/**
	 * \brief Return the inner (undecorated) table.
	 *
	 * This is a way to skip the costs for decoration in the output.
	 *
	 * This is useful if either the decoration is decided to be skipped entirely
	 * or no decoration was requested in the first place.
	 *
	 * \return Inner table
	 */
	std::unique_ptr<PrintableTable> remove_inner_table();

protected:

	/**
	 * \brief Read from inner table.
	 *
	 * \return Inner table
	 */
	const StringTable* table() const;

	/**
	 * \brief Return decorator for the specified index or nullptr.
	 *
	 * \param[in] idx Inner row or column index
	 *
	 * \return Decorator for this row or column index or nullptr
	 */
	const CellDecorator* decorator(const int idx) const;

	/**
	 * \brief Set flag for a cell to TRUE or FALSE.
	 *
	 * \param[in] i Row
	 * \param[in] j Column
	 * \param[in] f Boolean value for flag
	 */
	void set_flag(const int i, const int j, const bool f);

	/**
	 * \brief Convert row to inner row index.
	 *
	 * \param[in] row Row
	 * \return Inner row index
	 */
	int row_idx(const int i) const;

	/**
	 * \brief Convert column to inner column index.
	 *
	 * \param[in] col Column
	 * \return Inner col index
	 */
	int col_idx(const int i) const;

private:

	// PrintableTable

	std::string do_title() const final;

	const std::string& do_ref(int row, int col) const final;

	std::string do_cell(int row, int col) const final;

	int do_rows() const final;

	std::string do_row_label(int row) const final;

	std::size_t do_max_height(int row) const final;

	int do_cols() const final;

	std::string do_col_label(int col) const final;

	std::size_t do_max_width(int col) const final;

	table::Align do_align(int col) const final;

	std::size_t do_optimal_width(const int col) const final;

	bool do_empty() const final;

	const StringTableLayout* do_layout() const final;
};


/**
 * \brief Record-based interface for decorating a PrintableTable.
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
	 * \param[in] d          Decorator
	 */
	void register_to_record(const int record_idx,
			std::unique_ptr<CellDecorator> d);

	/**
	 * \brief Get decorator for record.
	 */
	const CellDecorator* on_record(const int record_idx) const;

	/**
	 * \brief Register a decorator for a certain field.
	 *
	 * \param[in] field_idx Field index
	 * \param[in] d         Decorator
	 */
	void register_to_field(const int field_idx,
			std::unique_ptr<CellDecorator> d);

	/**
	 * \brief Get decorator for field.
	 */
	const CellDecorator* on_field(const int field_idx) const;

	/**
	 * \brief Mark cell as decorated.
	 */
	void mark(const int record_idx, const int field_idx);

	/**
	 * \brief Mark cell as undecorated.
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

	std::unique_ptr<TableComposer> create_composer(
		const std::size_t records,
		const std::vector<ATTR>& field_types, const bool with_labels) const;

private:

	virtual std::unique_ptr<TableComposer> do_create_composer(
		const std::size_t records,
		const std::vector<ATTR>& field_types, const bool with_labels) const
	= 0;
};


/**
 * \brief Create a RowTableComposer.
 */
class RowTableComposerBuilder final : public TableComposerBuilder
{
	std::unique_ptr<TableComposer> do_create_composer(
		const std::size_t records,
		const std::vector<ATTR>& field_types, const bool with_labels) const
		final;
};


/**
 * \brief Create a ColTableComposer.
 */
class ColTableComposerBuilder final : public TableComposerBuilder
{
	std::unique_ptr<TableComposer> do_create_composer(
		const std::size_t records,
		const std::vector<ATTR>& field_types, const bool with_labels) const
		final;
};


// Required for ResultFormatter

using arcstk::ARId;
using arcstk::ARResponse;
using arcstk::Checksum;
using arcstk::Checksums;
using arcstk::Match;
using arcstk::TOC;


/**
 * \brief Abstract base class for result formatting.
 */
class ResultFormatter : public WithInternalFlags
{
public:

	/**
	 * \brief Set the TableComposerBuilder to use.
	 *
	 * \param[in] c The TableComposerBuilder to use
	 */
	void set_builder_creator(std::unique_ptr<TableComposerBuilder> c);

	/**
	 * \brief The TableComposerBuilder to use.
	 *
	 * \return The TableComposerBuilder to use
	 */
	const TableComposerBuilder* builder_creator() const;

	/**
	 * \brief Create the internal TableComposer to compose the result data.
	 *
	 * Uses the internal TableComposer instance.
	 */
	std::unique_ptr<TableComposer> create_composer(
		const std::size_t records,
		const std::vector<ATTR>& field_types, const bool with_labels) const;

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
	bool label() const;

	/**
	 * \brief Activate or deactivate the printing of labels.
	 *
	 * Intended to control the printing of column titles and row labels.
	 *
	 * \param[in] label Flag to set for printing the labels
	 */
	void set_label(const bool &label);

	/**
	 * \brief Returns TRUE iff instance is configured to format the track
	 * number.
	 *
	 * \return Flag for printing the track number
	 */
	bool track() const;

	/**
	 * \brief Activate or deactivate the printing of the track number.
	 *
	 * \param[in] track Flag to set for printing the track number
	 */
	void set_track(const bool &track);

	/**
	 * \brief Returns TRUE iff instance is configured to format the offset.
	 *
	 * \return Flag for printing the offset
	 */
	bool offset() const;

	/**
	 * \brief Activate or deactivate the printing of the offsets.
	 *
	 * \param[in] offset Flag to set for printing the offset
	 */
	void set_offset(const bool &offset);

	/**
	 * \brief Returns TRUE iff instance is configured to format the length.
	 *
	 * \return Flag for printing the length
	 */
	bool length() const;

	/**
	 * \brief Activate or deactivate the printing of the lengths.
	 *
	 * \param[in] length Flag to set for printing the length
	 */
	void set_length(const bool &length);

	/**
	 * \brief Returns TRUE iff instance is configured to format the filename.
	 *
	 * \return Flag for printing the filename
	 */
	bool filename() const;

	/**
	 * \brief Activate or deactivate the printing of the filenames.
	 *
	 * \param[in] filename Flag to set for printing the filename
	 */
	void set_filename(const bool &filename);

protected:

	/**
	 * \brief Build the result representation.
	 *
	 * Calls create_attributes() to determine and create the field_types for the
	 * result object, create_composer() to instantiate the TableComposer
	 * instance, and build_table() to populate the result table with the
	 * relevant data.
	 *
	 * \param[in] checksums      Calculated checksums
	 * \param[in] response       Reference checksums
	 * \param[in] refsums        Reference checksums
	 * \param[in] diff           Match between calculation and reference
	 * \param[in] block          Best block in \c diff
	 * \param[in] toc            TOC for calculated checksums
	 * \param[in] arid           ARId of input
	 * \param[in] alt_prefix     Alternative AccurateRip URL prefix
	 * \param[in] filenames      List of input filenames
	 * \param[in] types_to_print List of Checksum types requested for print
	 */
	std::unique_ptr<Result> build_result(
		const Checksums& checksums, const ARResponse* response,
		const std::vector<Checksum>* refsums, const Match* diff, int block,
		const TOC* toc, const ARId& arid,
		const std::string& alt_prefix,
		const std::vector<std::string>& filenames,
		const std::vector<arcstk::checksum::type>& types_to_print) const;

	/**
	 * \brief Validate the result objects common to every result.
	 *
	 * Throws if validation fails.
	 *
	 * \param[in] checksums  Checksums as resulted
	 * \param[in] filenames  Filenames as resulted
	 * \param[in] toc        TOC as resulted
	 * \param[in] arid       ARId as resulted
	 */
	void validate(const Checksums& checksums, const TOC* toc,
		const ARId& arid, const std::vector<std::string>& filenames) const;

	/**
	 * \brief Create the result field_types.
	 *
	 * \param[in] tracks       Iff TRUE, print tracks
	 * \param[in] offsets      Iff TRUE, print offsets
	 * \param[in] lengths      Iff TRUE, print lengths
	 * \param[in] filenames    Iff TRUE, print filenames
	 * \param[in] types        List of checksum types to print
	 *
	 * \return Sequence of result field_types to form an record
	 */
	std::vector<ATTR> create_attributes(const bool tracks,
		const bool offsets, const bool lengths, const bool filenames,
		const std::vector<arcstk::checksum::type>& types,
		const int total_theirs) const;

	/**
	 * \brief Build an ARId.
	 */
	RichARId build_id(const TOC* toc, const ARId& arid,
		const std::string& alt_prefix) const;

	/**
	 * \brief Build the result table.
	 *
	 * \param[in] checksums   Checksums as resulted
	 * \param[in] response    Reference checksums from AccurateRip response
	 * \param[in] refsums     Reference checksums as specified
	 * \param[in] match       Match object (maybe null)
	 * \param[in] block       Index to choose from \c match
	 * \param[in] filenames   Filenames as resulted
	 * \param[in] toc         TOC as resulted
	 * \param[in] arid        ARId as resulted
	 * \param[in] b           TableComposer to use
	 * \param[in] p_tracks    Iff TRUE, print tracks
	 * \param[in] p_offsets   Iff TRUE, print offsets
	 * \param[in] p_lengths   Iff TRUE, print lengths
	 * \param[in] p_filenames Iff TRUE, print filenames
	 * \param[in] types       List of checksum types to print
	 *
	 * \return Table with result data
	 */
	std::unique_ptr<PrintableTable> build_table(const Checksums& checksums,
		const ARResponse* response, const std::vector<Checksum>* refsums,
		const Match* match, const int block,
		const TOC* toc, const ARId& arid,
		const std::vector<std::string>& filenames,
		const std::vector<arcstk::checksum::type>& types,
		const bool p_tracks, const bool p_offsets, const bool p_lengths,
		const bool p_filenames) const;

	/**
	 * \brief Print my checksums.
	 *
	 * Used by build_table().
	 *
	 * Note that \c record also determines access to \c checksums.
	 *
	 * \param[in] checksums  Result checksums
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
	 * \param[in] checksum   Reference checksum
	 * \param[in] does_match Print as matching or not matching
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
		const bool tracks, const bool offsets, const bool lengths,
		const bool filenames,
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


} // namespace arcsapp

#endif

