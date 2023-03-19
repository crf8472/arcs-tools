#ifndef __ARCSTOOLS_TABLE_HPP__
#define __ARCSTOOLS_TABLE_HPP__

/**
 * \file
 *
 * \brief Printable Tables.
 */

#include <cstddef>       // for size_t
#include <iomanip>       // for setw, setfill
#include <istream>       // for ostream
#include <memory>        // for unique_ptr
#include <string>        // for string
#include <vector>        // for vector


#ifndef __LIBARCSTK_IDENTIFIER_HPP__
#include <arcstk/identifier.hpp>
#endif
//#ifndef __LIBARCSTK_CALCULATE_HPP__
//#include <arcstk/calculate.hpp>
//#endif
//#ifndef __LIBARCSTK_MATCH_HPP__
//#include <arcstk/match.hpp>
//#endif
//#ifndef __LIBARCSTK_PARSE_HPP__
//#include <arcstk/parse.hpp>
//#endif

namespace arcsapp
{


/**
 * \brief Minimal fitting width (= longest entry) of a string column
 */
template <class Container, typename = arcstk::IsFilenameContainer<Container>>
inline int optimal_width(Container&& list)
{
	std::size_t width = 0;

	for (const auto& entry : list)   // TODO Do this with STL, e.g. std::max!
	{
		if (entry.length() > width)
		{
			width = entry.length();
		}
	}

	return width;
}


/**
 * \brief Base class for a table of strings layout.
 *
 * Implement function \c do_init() in a subclass to get a concrete table.
 * The default implementation of do_init() is empty in StringTable.
 *
 * \see StringTable
 */
class StringTableStructure
{
public:

	/**
	 * \brief Constructor
	 *
	 * \param[in] rows Number of rows (including header, if any)
	 * \param[in] cols Number of columns (including label column, if any)
	 */
	StringTableStructure(const int rows, const int cols);

	/**
	 * \brief Default constructor constructs a table with dimensions 0,0
	 */
	StringTableStructure() : StringTableStructure(0, 0) { /* empty */ }

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~StringTableStructure() noexcept;

	/**
	 * \brief Initialize the instance with the specified number of
	 * rows and columns.
	 *
	 * \param[in] rows Number of rows
	 * \param[in] cols Number of columns
	 */
	void init(const int rows, const int cols);

	/**
	 * \brief Returns the number of rows (without header)
	 *
	 * \return The number of rows
	 */
	std::size_t rows() const;

	/**
	 * \brief Returns the number of columns
	 *
	 * \return The number of columns
	 */
	std::size_t columns() const;

	/**
	 * \brief Set the title of this table.
	 *
	 * \param[in] title Title of this table
	 */
	void set_table_title(const std::string &title);

	/**
	 * \brief Get the title of this table.
	 *
	 * \return Title of this table
	 */
	const std::string& table_title() const;

	/**
	 * \brief Set the column width
	 *
	 * \param[in] col   Column index
	 * \param[in] width Width of the column
	 */
	void set_width(const int col, const int width);

	/**
	 * \brief Get width of a column
	 *
	 * \param[in] col The column to get the width of
	 *
	 * \return Width of the column
	 *
	 * \throws std::out_of_range If col > columns() or col < 0.
	 */
	int width(const int col) const;

	/**
	 * \brief Set the column width to dynamic
	 *
	 * \param[in] col   Column index
	 */
	void set_dynamic_width(const int col);

	/**
	 * \brief Indicate whether width of a column is dynamic
	 *
	 * \param[in] col The column to get the width of
	 *
	 * \return Width of the column
	 *
	 * \throws std::out_of_range If col > columns() or col < 0.
	 */
	bool has_dynamic_width(const int col) const;

	/**
	 * \brief Set alignment for the specified column
	 *
	 * \param[in] col   Column index
	 * \param[in] align Alignment
	 */
	void set_alignment(const int col, const bool align);

	/**
	 * \brief Alignment of the specified column
	 *
	 * \param[in] col   Column index
	 *
	 * \return Alignment of this column
	 */
	bool alignment(const int col) const;

	/**
	 * \brief Set the column type
	 *
	 * \param[in] col   Column index
	 * \param[in] type  The column type
	 */
	void set_type(const int col, const int type);

	/**
	 * \brief Get type of a column
	 *
	 * \param[in] col The column to get the type of
	 *
	 * \return Type of the column
	 *
	 * \throws std::out_of_range If col > columns() or col < 0.
	 */
	int type(const int col) const;

	/**
	 * \brief Set the column name
	 *
	 * \param[in] col   Column index
	 * \param[in] title The column title
	 */
	void set_title(const int col, const std::string &title);

	/**
	 * \brief Get name of a column
	 *
	 * \param[in] col The column to get the name of
	 *
	 * \return Name of the column
	 *
	 * \throws std::out_of_range If col > columns() or col < 0.
	 */
	std::string title(const int col) const;

	/**
	 * \brief Set the row label
	 *
	 * \param[in] row   Row index
	 * \param[in] label The row label
	 */
	void set_row_label(const int row, const std::string &label);

	/**
	 * \brief Get label of row
	 *
	 * \param[in] row The row to get the label of
	 *
	 * \return Label of the row
	 *
	 * \throws std::out_of_range If row > rows()
	 */
	std::string row_label(const int row) const;

	/**
	 * \brief Set the column delimiter symbol
	 *
	 * \param[in] delim The column delimiter symbol
	 */
	void set_column_delimiter(const std::string &delim);

	/**
	 * \brief Returns the column delimiter symbol
	 *
	 * \return The column delimiter
	 */
	std::string column_delimiter() const;

	/**
	 * \brief Reset the dimensions of the table
	 *
	 * \param[in] rows Number of rows
	 * \param[in] cols Number of columns
	 */
	void resize(const int rows, const int cols);

	/**
	 * \brief Perform a bounds check and throw std::out_of_range on illegal
	 * values.
	 *
	 * \param[in] row  Row index
	 * \param[in] col  Column index
	 *
	 * \throws std::out_of_range If col > columns() or col < 0.
	 * \throws std::out_of_range If row > rows() or row < 0.
	 */
	void bounds_check(const int row, const int col) const;

protected:

	/**
	 * \brief Prints the title of the table.
	 *
	 * \param[in] out The stream to print to
	 */
	void print_title(std::ostream &out) const;

	/**
	 * \brief Worker to print column titles.
	 *
	 * Default implementation prints title(column) for each column
	 * with alignment to the right side.
	 *
	 * \param[in] out The stream to print to
	 */
	void print_column_titles(std::ostream &out) const;

	/**
	 * \brief Worker to print a row label
	 *
	 * Default implementation prints row_label(row) with alignment to the right
	 * side.
	 *
	 * \param[in] out The stream to print to
	 * \param[in] row Current row
	 */
	void print_label(std::ostream &out, const int row) const;

	/**
	 * \brief Worker to print a cell.
	 *
	 * Default implementation prints \c text using \c width(col) and
	 * \c alignment(col).
	 *
	 * \param[in] out        The stream to print to
	 * \param[in] col        Current column
	 * \param[in] text       Text to print
	 * \param[in] with_delim TRUE prints a column delimiter, FALSE skips it
	 */
	void print_cell(std::ostream &out, const int col, const std::string &text,
		const bool with_delim) const;

	/**
	 * \brief Return the optimal width for the label column.
	 *
	 * The optimal width is based only on the labels that would be printed.
	 *
	 * \return Optimal width for label column
	 */
	int optimal_label_width() const;

	/**
	 * \brief Resize the dimensions of the layout
	 *
	 * \param[in] rows New number of rows
	 * \param[in] cols New number of columns
	 */
	void update_dimensions(const int rows, const int cols) const;

	/**
	 * \brief TRUE if \c row is within the dimensions
	 *
	 * \param[in] row Row index to check
	 *
	 * \return TRUE if \c row is within the dimensions, otherwise FALSE
	 */
	bool legal_row(const int row) const;

	/**
	 * \brief TRUE if \c col is within the dimensions
	 *
	 * \param[in] col Column index to check
	 *
	 * \return TRUE if \c col is within the dimensions, otherwise FALSE
	 */
	bool legal_col(const int col) const;

	/**
	 * \brief Throws if \c legal_row(row) is FALSE
	 *
	 * \param[in] row Row index to check
	 *
	 * \throws std::out_of_range If not \c legal_row(row)
	 */
	void bounds_check_row(const int row) const;

	/**
	 * \brief Throws if <tt>legal_col(col)</tt> is FALSE
	 *
	 * \param[in] col Column index to check
	 *
	 * \throws std::out_of_range If not <tt>legal_col(col)</tt>
	 */
	void bounds_check_col(const int col) const;


	StringTableStructure(const StringTableStructure &rhs);

	StringTableStructure(StringTableStructure &&rhs) noexcept;

private:

	virtual void do_init(const int rows, const int cols)
	= 0;

	virtual std::size_t do_rows() const;
	virtual std::size_t do_columns() const;

	virtual void do_set_table_title(const std::string &title);
	virtual const std::string& do_table_title() const;

	virtual void do_set_width(const int col, const int width);
	virtual int do_width(const int col) const;

	virtual void do_set_dynamic_width(const int col);
	virtual bool do_has_dynamic_width(const int col) const;

	virtual void do_set_alignment(const int col, const bool align);
	virtual bool do_alignment(const int col) const;

	virtual void do_set_type(const int col, const int type);
	virtual int do_type(const int col) const;

	virtual void do_set_title(const int col, const std::string &name);
	virtual std::string do_title(const int col) const;

	virtual void do_set_row_label(const int row, const std::string &label);
	virtual std::string do_row_label(const int row) const;

	virtual void do_set_column_delimiter(const std::string &delim);
	virtual std::string do_column_delimiter() const;

	virtual void do_resize(const int rows, const int cols);
	virtual void do_bounds_check(const int row, const int col) const;

	// forward declaration
	class Impl;

	/**
	 * \brief Private implementation
	 */
	std::unique_ptr<Impl> impl_;
};


class StringTableBase;

std::ostream& operator << (std::ostream &o, const StringTableBase &table);

/**
 * \brief Base class for tables of strings.
 *
 * This class is not intended for polymorphic use.
 */
class StringTableBase : public StringTableStructure
{
public:

	friend std::ostream& operator<< (std::ostream &o,
			const StringTableBase &table);

	/**
	 * \brief Constructor
	 *
	 * \param[in] rows    Number of rows (including header, if any)
	 * \param[in] columns Number of columns (including label column, if any)
	 * \param[in] has_appending_rows If 'FALSE' appending rows is forbidden
	 */
	StringTableBase(const int rows, const int columns,
			const bool has_appending_rows);

	/**
	 * \brief Constructor
	 *
	 * Activates dynamic optimal table width and appending of rows.
	 *
	 * \param[in] rows    Number of rows (including header, if any)
	 * \param[in] columns Number of columns (including label column, if any)
	 */
	StringTableBase(const int rows, const int columns)
		: StringTableBase(rows, columns, true) { /* empty */ };

	/**
	 * \brief Default constructor
	 *
	 * Constructs a table with dimensions 0,0, activates appending rows and
	 * dynamic column widths.
	 */
	StringTableBase() : StringTableBase(0, 0) { /* empty */ }

	/**
	 * \brief Default destructor
	 */
	virtual ~StringTableBase() noexcept;

	/**
	 * \brief TRUE if the table has row appending activated.
	 *
	 * \return TRUE if row appending is activated, otherwise FALSE.
	 */
	bool has_appending_rows() const;

	/**
	 * \brief Updates an existing cell with specified text.
	 *
	 * Performs bounds checking and throws on illegal values.
	 *
	 * \param[in] row  Row index to access
	 * \param[in] col  Column index to access
	 * \param[in] text New text for specified cell
	 *
	 * \throws std::out_of_range If col > columns() or col < 0.
	 */
	void update_cell(const int row, const int col, const std::string &text);

	/**
	 * \brief Returns the content of the specified cell.
	 *
	 * The result is equivalent to \c (row, col) if \c row and \c col
	 * have legal values, but \c cell() performs bounds checking and
	 * throws on illegal values.
	 *
	 * \param[in] row Row index to access
	 * \param[in] col Column index to access
	 *
	 * \return Content of the specified cell
	 *
	 * \throws std::out_of_range If col > columns() or col < 0.
	 */
	std::string cell(const int row, const int col) const;

	/**
	 * \brief Access operator.
	 *
	 * The result is equivalent to \c cell(row, col) if \c row and \c col
	 * have legal values, but there is no bounds check performed.
	 *
	 * \param[in] row Row index to access
	 * \param[in] col Column index to access
	 *
	 * \return Content of the specified cell
	 */
	std::string operator() (const int row, const int col) const;

	/**
	 * \brief Row index of the first row that is not filled
	 *
	 * \return Current 0-based row index
	 */
	int current_row() const;

	/**
	 * \brief Return maximum number of rows
	 *
	 * \return Maximum number of rows
	 */
	int max_rows() const;

	/**
	 * \brief Return TRUE if the table contains no content.
	 *
	 * \return TRUE if the table has no content, otherwise FALSE
	 */
	bool empty() const;

protected:

	StringTableBase(const StringTableBase &rhs);

	StringTableBase(StringTableBase &&rhs) noexcept;

	StringTableBase& operator = (const StringTableBase &rhs);

	StringTableBase& operator = (StringTableBase &&rhs) noexcept;

	// TODO swap

	// TODO ==

private:

	void do_resize(const int rows, const int cols) override;
	// from TableStructure

	/**
	 * \brief Implements StringTableBase::update_cell(const int, const int)
	 *
	 * \param[in] row  Row index
	 * \param[in] col  Column index
	 * \param[in] text New cell text
	 *
	 * \throws std::out_of_range If col > columns() or col < 0.
	 */
	virtual void do_update_cell(const int row, const int col,
			const std::string &text);

	/**
	 * \brief Implements StringTableBase::cell(const int, const int)
	 *
	 * \param[in] row  Row index
	 * \param[in] col  Column index
	 *
	 * \throws std::out_of_range If col > columns() or col < 0.
	 *
	 * \return Content of cell(row, col)
	 */
	virtual std::string do_cell(const int row, const int col) const;

	/**
	 * \brief Holds the configuration flags.
	 */
	std::vector<bool> flags_;

	// forward declaration
	class Impl;

	/**
	 * \brief Private implementation
	 */
	std::unique_ptr<Impl> impl_;
};


/**
 * \brief A table of strings.
 *
 * The columns are by default configured to have always optimal width.
 * The table is by default allowed to have rows appended.
 */
class StringTable : public StringTableBase
{
public:

	//StringTable(const StringTable &rhs) = default;
	//StringTable(StringTable &&rhs) noexcept = default;
	//StringTable& operator = (const StringTable &rhs) = default;
	//StringTable& operator = (StringTable &&rhs) noexcept = default;

	using StringTableBase::StringTableBase;
	using StringTableBase::operator=;

private:

	void do_init(const int rows, const int cols) override;
	// from StringTableStructure
};

} // namespace arcsapp

#endif

