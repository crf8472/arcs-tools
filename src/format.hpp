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
	T object_;

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
		return object_;
	}

protected:

	inline RecordInterface(T&& object)
		: object_ { std::move(object) }
	{
		// empty
	}

	/**
	 * \brief Read or manipulate the object holding the records.
	 */
	inline T& object()
	{
		return object_;
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
 * ResultComposer will know whether the field_types are rows or columns in the
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
 * \brief Description.
 */
class CellDecorator
{
	std::vector<bool> flags_;

	virtual std::string do_decorate(std::string&& s) const
	= 0;

public:

	CellDecorator(const std::size_t n);

	void set(const int i);
	void unset(const int i);

	bool is_set(const int i) const;

	std::string decorate(const int i, std::string&& s) const;
};


using table::PrintableTable;
using table::StringTable;
using table::StringTableLayout;


/**
 * \brief Table decorator.
 */
class StringTableDecorator : public PrintableTable
{
	StringTable table_;

	std::vector<std::unique_ptr<CellDecorator>> decorators_;

	std::map<int, const CellDecorator*> registry_;

public:

	StringTableDecorator(StringTable&& t);

	void set(const StringTable* t);

	const CellDecorator* add(std::unique_ptr<CellDecorator> d);

	void register_to_row(const int i, const CellDecorator* d);

	const CellDecorator* row_decorator(const int i) const;

	void register_to_col(const int j, const CellDecorator* d);

	const CellDecorator* col_decorator(const int j) const;

protected:

	const StringTable* table() const;

	const CellDecorator* find(const int n) const;

private:

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
class TableDecoratorManager
{
	virtual void do_register_to_record(const int record_idx,
			std::unique_ptr<CellDecorator> d)
	= 0;

	virtual void do_register_to_field(const ATTR field_type, const int f,
			std::unique_ptr<CellDecorator> d)
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
	 * \brief Register a decorator for a certain field.
	 *
	 * \param[in] field_type Type of field
	 * \param[in] f          f-th occurence of field_type
	 * \param[in] d          Decorator
	 */
	void register_to_field(const ATTR field_type, const int f,
			std::unique_ptr<CellDecorator> d);
};


// Rename to TableComposer
/**
 * \brief Interface for constructing a result table.
 *
 * A ResultComposer builds a StringTable with records and field_types. Each
 * record contains a value for each of its defined fields.
 */
class ResultComposer :   public RecordInterface<StringTable, ATTR>
					   , public TableDecoratorManager
					   , public ResultProvider
{
public:

	/**
	 * \brief Set the layout for the result table.
	 *
	 * \param[in] layout The layout to use for the result table
	 */
	void set_layout(std::unique_ptr<StringTableLayout> layout);

	/**
	 * \brief Return the StringTable instance.
	 *
	 * \return StringTable
	 */
	StringTable table() const;

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

protected:

	/**
	 * \brief Common constructor.
	 *
	 * \param[in] fields   Ordering of field_types
	 * \param[in] table    Table with dimensions for records and field_types
	 */
	ResultComposer(const std::vector<ATTR>& fields, StringTable&& table);

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
	StringTable& in_table();

	/**
	 * \brief Read from table.
	 *
	 * \return Reference to internal table
	 */
	const StringTable& from_table() const;

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

	// ResultProvider

	std::unique_ptr<Result> do_result() const final;

	// ResultComposer

	virtual int do_get_row(const int i, const int j) const
	= 0;

	virtual int do_get_col(const int i, const int j) const
	= 0;

	virtual void set_field_label(const int field_idx, const std::string& label)
	= 0;

	virtual std::string field_label(const int field_idx) const
	= 0;

	std::vector<ATTR> fields_;
	std::map<ATTR, std::string> labels_;
};


/**
 * \brief Build a table whose records are rows and the fields are columns.
 */
class RowResultComposer final : public ResultComposer
{
	// RecordInterface

	size_type do_total_records() const final;
	size_type do_fields_per_record() const final;

	// TableDecoratorManager

	void do_register_to_record(const int record_idx,
			std::unique_ptr<CellDecorator> d) final;

	void do_register_to_field(const ATTR field_type, const int f,
			std::unique_ptr<CellDecorator> d) final;

	// ResultComposer

	void set_field_label(const int field_idx, const std::string& label) final;
	std::string field_label(const int field_idx) const final;
	int do_get_row(const int i, const int j) const final;
	int do_get_col(const int i, const int j) const final;

public:

	RowResultComposer(const std::size_t records,
		const std::vector<ATTR>& order, const bool with_labels);
};


/**
 * \brief Build a table whose records are columns and the fields are rows.
 */
class ColResultComposer final : public ResultComposer
{
	// RecordInterface

	size_type do_total_records() const final;
	size_type do_fields_per_record() const final;

	// TableDecoratorManager

	void do_register_to_record(const int record_idx,
			std::unique_ptr<CellDecorator> d) final;

	void do_register_to_field(const ATTR field_type, const int f,
			std::unique_ptr<CellDecorator> d) final;

	// ResultComposer

	void set_field_label(const int field_idx, const std::string& label) final;
	std::string field_label(const int field_idx) const final;
	int do_get_row(const int i, const int j) const final;
	int do_get_col(const int i, const int j) const final;

public:

	ColResultComposer(const std::size_t records,
		const std::vector<ATTR>& order, const bool with_labels);
};


/**
 * \brief Create a ResultComposer.
 */
class ResultComposerBuilder
{
public:

	std::unique_ptr<ResultComposer> create_composer(
		const std::size_t records,
		const std::vector<ATTR>& field_types, const bool with_labels) const;

private:

	virtual std::unique_ptr<ResultComposer> do_create_composer(
		const std::size_t records,
		const std::vector<ATTR>& field_types, const bool with_labels) const
	= 0;
};


/**
 * \brief Create a RowResultComposer.
 */
class RowResultComposerBuilder final : public ResultComposerBuilder
{
	std::unique_ptr<ResultComposer> do_create_composer(
		const std::size_t records,
		const std::vector<ATTR>& field_types, const bool with_labels) const
		final;
};


/**
 * \brief Create a ColResultComposer.
 */
class ColResultComposerBuilder final : public ResultComposerBuilder
{
	std::unique_ptr<ResultComposer> do_create_composer(
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
	 * \brief Set the ResultComposerBuilder to use.
	 *
	 * \param[in] c The ResultComposerBuilder to use
	 */
	void set_builder_creator(std::unique_ptr<ResultComposerBuilder> c);

	/**
	 * \brief The ResultComposerBuilder to use.
	 *
	 * \return The ResultComposerBuilder to use
	 */
	const ResultComposerBuilder* builder_creator() const;

	/**
	 * \brief Create the internal ResultComposer to compose the result data.
	 *
	 * Uses the internal ResultComposer instance.
	 */
	std::unique_ptr<ResultComposer> create_composer(
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
	 * result object, create_composer() to instantiate the ResultComposer
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
	 * \param[in] b           ResultComposer to use
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
	 * \param[in] c          ResultComposer to use
	 */
	void mine_checksum(const Checksum& checksum,
		const int record, const int field, ResultComposer* c) const;

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
	 * \param[in] c          ResultComposer to use
	 */
	void their_checksum(const Checksum& checksums, const bool does_match,
		const int record, const int field, ResultComposer* c) const;

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
	 * \brief Configure ResultComposer.
	 *
	 * This is called by build_result() before the composer creates the result
	 * object. Its configuration can be adjusted by implementing this function.
	 *
	 * \param[in,out] composer The ResultComposer to be configured
	 */
	virtual void pre_table(ResultComposer& composer) const;

	/**
	 * \brief Configure StringTable before build_table() returns it.
	 *
	 * Subclasses may intercept the table before it is returned to the caller.
	 */
	virtual std::unique_ptr<PrintableTable> post_table(StringTable&& table)
		const;

	/**
	 * \brief Implements mine_checksum().
	 */
	virtual void do_mine_checksum(const Checksum& checksum,
		const int record, const int field, ResultComposer* c) const;

	/**
	 * \brief Called by their_checksum() on a match.
	 */
	virtual void do_their_match(const Checksum& checksum, const int record,
			const int field, ResultComposer* c) const;

	/**
	 * \brief Called by their_checksum() on a mismatch.
	 */
	virtual void do_their_mismatch(const Checksum& checksum, const int record,
			const int field, ResultComposer* c) const;

	/**
	 * \brief Internal ResultComposerBuilder.
	 */
	std::unique_ptr<ResultComposerBuilder> builder_creator_;

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

