#ifndef __ARCSTOOLS_TABLE_HPP__
#define __ARCSTOOLS_TABLE_HPP__

#include <algorithm>   // for max
#include <cstddef>     // for size_t
#include <ios>         // for boolalpha
#include <map>         // for map
#include <memory>      // for unique_ptr
#include <ostream>     // for ostream
#include <sstream>     // for ostringstream
#include <string>      // for string
#include <type_traits> // for enable_if, is_unsigned
#include <utility>     // for forward
#include <vector>      // for vector

// to_sep_list requirements:
//#include <algorithm>   // for transform
#include <functional>  // for function
#include <iterator>    // for ostream_iterator
//#include <sstream>     // for ostringstream

/**
 * \file
 *
 * \brief Printable Tables.
 */

namespace arcsapp
{
inline namespace v_1_0_0
{
namespace details
{

/**
 * \brief Convert an object to a std::string.
 *
 * Type \c T must have operator <<.
 *
 * \tparam T Input type to convert, must have overloaded operator <<.
 *
 * \param[in] value The value to convert
 *
 * \return A string representation of \c value
 */
template <typename T> // T must have <<
inline std::string to_string(T&& value)
{
	std::ostringstream s;
	s << std::boolalpha << std::forward<T>(value);
	return s.str();
}

/**
 * \brief Remove leading and trailing whitespace from a string.
 *
 * \param[in] str String to trim
 *
 * \return Input string with leading and trailing whitespace removed.
 */
std::string trim(std::string str);

/**
 * \brief Split a string in parts of equal length.
 *
 * The last part is allowed to have a shorter length.
 *
 * \param[in] str     String to split
 * \param[in] max_len Length of each part except last.
 *
 * \return Parts of the string with equal length except last.
 */
std::vector<std::string> split(std::string str, const std::size_t max_len);

/**
 * \brief Split a string in parts divided by <tt>delim</tt>.
 *
 * The split strings will no more contain any occurrence of <tt>delim</tt>.
 *
 * If the input string does not contain the delimiter, the returned aggregation
 * of substring will contain an instance of the original input string as its
 * only element.
 *
 * \param[in] str     String to split
 * \param[in] delim   Delimiter for splitting
 *
 * \return Substrings.
 */
std::vector<std::string> split(std::string str, const std::string& delim);

/**
 * \brief Transform container content to a delimiter separated list.
 *
 * \param[in] c     Container to transform
 * \param[in] delim Delimiter to place between every two container elements
 * \param[in] f     Function to transform container element to string
 */
template <typename Container> // TODO SFINAE stuff: empty(), size(), b+e, rbegin
inline std::string to_sep_list(const Container c, const std::string delim,
		const std::function<std::string(const typename Container::value_type &)>
		&f)
{
	if (c.empty())
	{
		return std::string{};
	}

	std::ostringstream list_stream;

	if (c.size() > 1)
	{
		std::transform(c.begin(), --c.rbegin().base(),
			std::ostream_iterator<std::string>(list_stream, delim.c_str()), f);

		list_stream << f(*c.rbegin());
	} else
	{
		list_stream << f(*c.begin());
	}

	return list_stream.str();
}

} // namespace details


/**
 * \brief Data structures, interfaces and formatters for tables.
 */
namespace table
{

/**
 * \brief Alignment types for StringTable.
 */
enum Align : int
{
	LEFT,
	RIGHT,
	BLOCK
};


class StringTableLayout;


/**
 * \brief Interface for a printable table.
 */
class PrintableTable
{
public:

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~PrintableTable() = default;

	/**
	 * \brief Table title.
	 *
	 * \return Table title
	 */
	std::string title() const;

	/**
	 * \brief Read cell by row and column.
	 *
	 * Provide a const reference of the cell value.
	 *
	 * \param[in] row  Table row to access
	 * \param[in] col  Table column to access
	 *
	 * \return Content of the cell
	 */
	const std::string& ref(int row, int col) const;

	/**
	 * \brief Read cell by row and column.
	 *
	 * Equivalent to operator();
	 *
	 * \param[in] row  Table row to access
	 * \param[in] col  Table column to access
	 *
	 * \return Content of the cell
	 */
	std::string cell(int row, int col) const;

	/**
	 * \brief Number of actual rows in the table.
	 *
	 * \return Number of rows
	 */
	std::size_t rows() const;

	/**
	 * \brief Label of specified row.
	 *
	 * \param[in] row Table row to get label from
	 *
	 * \return Label of the specified row
	 */
	std::string row_label(int row) const;

	/**
	 * \brief Maximal height for this row.
	 *
	 * \param[in] row    Index of row
	 *
	 * \return Maximal height in characters
	 */
	std::size_t max_height(int row) const;

	/**
	 * \brief Number of actual columns in the table.
	 *
	 * \return Number of columns
	 */
	std::size_t cols() const;

	/**
	 * \brief Label of specified column.
	 *
	 * \param[in] col Table column to get label from
	 *
	 * \return Label of the specified column
	 */
	std::string col_label(int col) const;

	/**
	 * \brief Maximal width for this column.
	 *
	 * \param[in] col    Index of column
	 *
	 * \return Maximal width in characters
	 */
	std::size_t max_width(int col) const;

	/**
	 * \brief Alignment of the specified column.
	 *
	 * \return Alignment applied to the specified column.
	 */
	Align align(int col) const;

	/**
	 * \brief Optimal width of a column ignoring its column label.
	 *
	 * The column label if any may have a greater width. The optimal
	 * width of a column respecting its label can be determined by
	 * <code>std::max(optimal_width(c), col_label(c).width())</code>.
	 *
	 * \return The optimal width of the specified column
	 */
	std::size_t optimal_width(const int col) const;

	/**
	 * \brief Returns TRUE iff the table does not contain any cells.
	 *
	 * Every table is empty after the constructor is finished.
	 *
	 * \return TRUE iff the table does not contain cells, otherwise FALSE
	 */
	bool empty() const;

	/**
	 * \brief The layout to be used for printing.
	 *
	 * \return Layout for this table
	 */
	const StringTableLayout* layout() const;

private:

	virtual std::string do_title() const
	= 0;

	virtual const std::string& do_ref(int row, int col) const
	= 0;

	virtual std::string do_cell(int row, int col) const
	= 0;

	virtual std::size_t do_rows() const
	= 0;

	virtual std::string do_row_label(int row) const
	= 0;

	virtual std::size_t do_max_height(int row) const
	= 0;

	virtual std::size_t do_cols() const
	= 0;

	virtual std::string do_col_label(int col) const
	= 0;

	virtual std::size_t do_max_width(int col) const
	= 0;

	virtual Align do_align(int col) const
	= 0;

	virtual std::size_t do_optimal_width(const int col) const
	= 0;

	virtual bool do_empty() const
	= 0;

	virtual const StringTableLayout* do_layout() const
	= 0;
};


class StringTable;
bool operator== (const StringTable& lhs, const StringTable& rhs);


/**
 * \brief A table of strings.
 *
 * The table is initially constructed with a specified number of rows and
 * columns. It can optionally have title. Default behaviour is that each column
 * has alignment LEFT and optimal width.
 */
class StringTable final : public PrintableTable
{
	friend bool operator== (const StringTable& lhs, const StringTable& rhs);

public:

	using PrintableTable::cell; // const-version from base class

	/**
	 * \brief Constructor.
	 *
	 * \param[in] rows Number of rows
	 * \param[in] cols Number of columns
	 */
	StringTable(const std::size_t rows, const std::size_t cols);

	/**
	 * \brief Constructor.
	 *
	 * \param[in] title Table title
	 * \param[in] rows  Number of rows
	 * \param[in] cols  Number of columns
	 */
	StringTable(const std::string& title, const std::size_t rows,
			const std::size_t cols);

	StringTable(const StringTable& rhs);

	StringTable(StringTable&& rhs) noexcept;

	// General assignment
	StringTable& operator=(StringTable rhs) noexcept;

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~StringTable() noexcept;

	/**
	 * \brief Set the table title.
	 *
	 * \param[in] title Table title
	 */
	void set_title(const std::string& title);

	/**
	 * \brief Read cell by row and column.
	 *
	 * A std::runtime_error is thrown if the cell does not exist in the
	 * table.
	 *
	 * \param[in] row  Table row to access
	 * \param[in] col  Table column to access
	 *
	 * \return Content of the cell
	 */
	std::string operator() (int row, int col) const;

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
	 * \brief Write to cell by row and column.
	 *
	 * If the column is legal but the row does not exist, rows are
	 * appended as necessary to access the cell.
	 *
	 * \param[in] row  Table row to access
	 * \param[in] col  Table column to access
	 *
	 * \return Content of the cell
	 */
	std::string& cell(int row, int col);

	/**
	 * \brief Write a value to cell by row and column.
	 *
	 * Type of <tt>value</tt> must support stream operator <<.
	 * Write content using cell().
	 *
	 * \param[in] row   Table row to access
	 * \param[in] col   Table column to access
	 * \param[in] value Value to set
	 */
	template <typename T>
	void set_cell(int row, int col, T&& value)
	{
		using details::to_string;
		cell(row,col) = to_string(std::forward<T>(value));
	}

	/**
	 * \brief Set a label for the specified row.
	 *
	 * \param[in] row   Table row to set a label to
	 * \param[in] label Label to set
	 */
	void set_row_label(int row, const std::string& label);

	/**
	 * \brief Set maximal height for this row.
	 *
	 * \param[in] row    Index of row
	 * \param[in] height Maximal height in characters
	 */
	void set_max_height(int row, std::size_t height);

	/**
	 * \brief Default maximal height for this row.
	 *
	 * \return Maximal height in characters
	 */
	std::size_t default_max_height() const;

	/**
	 * \brief Set a label for the specified column.
	 *
	 * \param[in] col   Table column to set a label to
	 * \param[in] label Label to set
	 */
	void set_col_label(int col, const std::string& label);

	/**
	 * \brief Set maximal width for this column.
	 *
	 * \param[in] col    Index of column
	 * \param[in] width  Maximal width in characters
	 */
	void set_max_width(int col, std::size_t width);

	/**
	 * \brief Default maximal width for this column.
	 *
	 * \return Default maximal width in characters
	 */
	std::size_t default_max_width() const;

	/**
	 * \brief Set the alignment for the respective column.
	 *
	 * \param[in] col   The column to modify
	 * \param[in] align The alignment to apply to the column
	 */
	void set_align(int col, Align align);

	/**
	 * \brief Description.
	 *
	 * \param[in] row  Table row to access
	 * \param[in] col  Table column to access
	 *
	 * \return TRUE iff the specified cell exists in the table, otherwise FALSE
	 */
	bool exists(const int row, const int col) const;

	/**
	 * \brief TRUE iff the specified row exists in the table.
	 *
	 * \return TRUE iff row \c row exists, otherwise FALSE
	 */
	bool row_exists(const int row) const;

	/**
	 * \brief TRUE iff the specified column exists in the table.
	 *
	 * \return TRUE iff column \c col exists, otherwise FALSE
	 */
	bool col_exists(const int col) const;

	/**
	 * \brief Insert one or more rows after an existing row.
	 *
	 * \param[in] rows Number of rows to insert
	 * \param[in] row  Row after which the rows are inserted
	 */
	void insert_rows_after(const int rows, const int row);

	/**
	 * \brief Append rows to the table.
	 *
	 * \param[in] rows Number of rows to be appended to the table
	 */
	void append_rows(const int rows);

	/**
	 * \brief Set the layout to be use for printing.
	 *
	 * \param[in] l Layout for this table
	 */
	void set_layout(std::unique_ptr<StringTableLayout> l);

	/**
	 * \brief Swap this instance with another.
	 *
	 * \param[in] rhs Instance to swap
	 */
	void swap(StringTable& rhs) noexcept;


	friend void swap(StringTable& lhs, StringTable& rhs) noexcept
	{
		lhs.swap(rhs);
	}

private:

	std::string do_title() const final;

	const std::string& do_ref(int row, int col) const final;

	std::string do_cell(int row, int col) const final;

	std::size_t do_rows() const final;

	std::string do_row_label(int row) const final;

	std::size_t do_max_height(int row) const final;

	std::size_t do_cols() const final;

	std::string do_col_label(int col) const final;

	std::size_t do_max_width(int col) const final;

	Align do_align(int col) const final;

	std::size_t do_optimal_width(const int col) const final;

	bool do_empty() const final;

	const StringTableLayout* do_layout() const final;

protected:

	/**
	 * \brief Type for storing the table contents.
	 */
	using store_type = std::vector<std::string>;

	/**
	 * \brief Type for the internal indices.
	 */
	using index_type = store_type::size_type;

	/**
	 * \brief Returns the internal index for a specified cell.
	 *
	 * The specified cell may or may not actually exist.
	 *
	 * \param[in] row  Table row to access
	 * \param[in] col  Table column to access
	 *
	 * \return The internal index of the specified cell
	 */
	index_type index(const int row, const int col) const;

	/**
	 * \brief Returns the internal index for a specified cell.
	 *
	 * A std::runtime_error is thrown iff the actual cell does not exist.
	 *
	 * \param[in] row  Table row to access
	 * \param[in] col  Table column to access
	 *
	 * \return The internal index of the specified cell
	 */
	index_type safe_index(const int row, const int col) const;

	/**
	 * \brief TRUE iff (unsigned) cell index actually exists.
	 *
	 * \return TRUE iff the specified index points to an existing cell,
	 * otherwise FALSE.
	 */
	template<typename T>
	typename std::enable_if<std::is_unsigned<T>::value, bool>::type
		exists(T i) const
	{
		return i < cells_.size();
	}

	/**
	 * \brief TRUE iff (signed) cell index actually exists.
	 *
	 * \return TRUE iff the specified index points to an existing cell,
	 * otherwise FALSE.
	 */
	template<typename T>
	typename std::enable_if<!std::is_unsigned<T>::value, bool>::type
		exists(T i) const
	{
		return i >= 0 && i < cells_.size();
	}

private:

	/**
	 * \brief Title of the table.
	 */
	std::string title_;

	/**
	 * \brief Number of actual rows.
	 */
	std::size_t rows_;

	/**
	 * \brief Default maximum height of a cell.
	 */
	std::size_t default_max_height_;

	/**
	 * \brief Number of actual columns.
	 */
	std::size_t cols_;

	/**
	 * \brief Default maximum width of a cell
	 */
	std::size_t default_max_width_;

	/**
	 * \brief Internal storage for row labels.
	 */
	std::vector<std::string> row_labels_;

	/**
	 * \brief Actual maximum heights of rows.
	 */
	std::vector<std::size_t> row_max_heights_;

	/**
	 * \brief Internal storage for column labels.
	 */
	std::vector<std::string> col_labels_;

	/**
	 * \brief Actual maximum widths of columns.
	 */
	std::vector<std::size_t> col_max_widths_;

	/**
	 * \brief Internal storage for column alignments.
	 */
	std::vector<Align> aligns_;

	/**
	 * \brief Internal storage for table cells.
	 */
	store_type cells_;

	/**
	 * \brief Layout of the table.
	 */
	std::unique_ptr<StringTableLayout> layout_;
};


/**
 * \brief Interface for splitting a string.
 */
class StringSplitter
{
public:

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~StringSplitter() noexcept = default;

	/**
	 * \brief Split a string in <tt>n</tt> parts with <tt>n-1</tt> parts
	 * having exact length of <tt>max_len</tt> chars.
	 *
	 * \param[in] str     String to split
	 * \param[in] max_len Length of each part (possibly except one part)
	 *
	 * \return Parts of the string
	 */
	std::vector<std::string> split(const std::string& str,
			const std::size_t max_len) const;

	/**
	 * \brief Deep copy of this instance.
	 *
	 * \return Deep copy of this instance
	 */
	std::unique_ptr<StringSplitter> clone() const;

private:

	virtual std::vector<std::string> do_split(const std::string& str,
			const std::size_t max_len) const
	= 0;

	virtual std::unique_ptr<StringSplitter> do_clone() const
	= 0;
};


/**
 * \brief Splits a string in n parts with n-1 parts of equal length.
 */
class DefaultSplitter final : public StringSplitter
{
	virtual std::vector<std::string> do_split(const std::string& str,
			const std::size_t max_len) const final;

	virtual std::unique_ptr<StringSplitter> do_clone() const final;
};


/**
 * \brief Layout for a StringTable.
 *
 * Layout contains all the visual information about a StringTable that can be
 * expressed without any knowledge of the concrete instance. Therefore, things
 * like alignment of columns is part of the table not of its layout.
 */
class StringTableLayout final
{
public:

	/**
	 * \brief Constructor.
	 */
	StringTableLayout();

	/**
	 * \brief Constructor.
	 *
	 * \param[in] s The splitter to use
	 */
	StringTableLayout(std::unique_ptr<StringSplitter> s);

	StringTableLayout(const StringTableLayout& rhs);
	StringTableLayout& operator=(const StringTableLayout& rhs);

	void swap(StringTableLayout& rhs) noexcept;

	// flags

	void set_title(const bool f);
	void set_row_labels(const bool f);
	void set_col_labels(const bool f);

	void set_top_delims(const bool f);
	void set_row_header_delims(const bool f);
	void set_row_inner_delims(const bool f);
	void set_bottom_delims(const bool f);

	void set_left_outer_delims(const bool f);
	void set_col_labels_delims(const bool f);
	void set_col_inner_delims(const bool f);
	void set_right_outer_delims(const bool f);

	bool title() const;
	bool row_labels() const;
	bool col_labels() const;

	bool top_delims() const;
	bool row_header_delims() const;
	bool row_inner_delims() const;
	bool bottom_delims() const;

	bool left_outer_delims() const;
	bool col_labels_delims() const;
	bool col_inner_delims() const;
	bool right_outer_delims() const;

	// delimiters

	void set_top_delim(const std::string& d);
	void set_row_header_delim(const std::string& d);
	void set_row_inner_delim(const std::string& d);
	void set_bottom_delim(const std::string& d);

	void set_left_outer_delim(const std::string& d);
	void set_col_labels_delim(const std::string& d);
	void set_col_inner_delim(const std::string& d);
	void set_right_outer_delim(const std::string& d);

	std::string top_delim() const;
	std::string row_header_delim() const;
	std::string row_inner_delim() const;
	std::string bottom_delim() const;

	std::string left_outer_delim() const;
	std::string col_labels_delim() const;
	std::string col_inner_delim() const;
	std::string right_outer_delim() const;

	// capabilities

	/**
	 * \brief Split a string in subsequent parts of guaranteed length.
	 *
	 * \param[in] str     The string to split
	 * \param[in] max_len The maximal length of each part
	 *
	 * \return Parts of the original string
	 */
	std::vector<std::string> split(const std::string& str,
			const std::size_t max_len) const;

	/**
	 * \brief Set splitter to use for formatting cells.
	 *
	 * \param[in] s Splitter to use for splitting lines in cells
	 */
	void set_splitter(std::unique_ptr<StringSplitter> s);

	/**
	 * \brief Return splitter for formatting cells.
	 *
	 * \return Splitter to use for splitting lines in cells
	 */
	const StringSplitter* splitter() const;

private:

	/**
	 * \brief Internal type to store actual flags.
	 */
	using flag_store_type = std::vector<bool>;

	/**
	 * \brief Flags for printing aspects of the table.
	 */
	enum class Flag : flag_store_type::size_type
	{
		TITLE,
		ROW_LABELS,
		COL_LABELS,
		ROW_TOP_DELIMS,
		ROW_HEADER_DELIMS,
		ROW_INNER_DELIMS,
		ROW_BOTTOM_DELIMS,
		COL_LEFT_OUTER_DELIMS,
		COL_LABELS_DELIMS,
		COL_INNER_DELIMS,
		COL_RIGHT_OUTER_DELIMS
	};

	/**
	 * \brief Internal flag store.
	 */
	flag_store_type flags_;

	/**
	 * \brief Get value for flag \c f.
	 *
	 * \return Value for flag \c f.
	 */
	bool flag_get(const Flag f) const;

	/**
	 * \brief Set value for flag \c f.
	 *
	 * \param[in] f     Flag to set value for
	 * \param[in] value Value for flag \c f.
	 */
	void flag_set(const Flag f, const bool value);

	/**
	 * \brief Internal type to store delimiters.
	 */
	using delim_store_type = std::vector<std::string>;

	/**
	 * \brief Delimiter type.
	 */
	enum class Delimiter : delim_store_type::size_type
	{
		ROW_TOP_DELIM,
		ROW_HEADER_DELIM,
		ROW_INNER_DELIM,
		ROW_BOTTOM_DELIM,
		COL_LEFT_OUTER_DELIM,
		COL_LABELS_DELIM,
		COL_INNER_DELIM,
		COL_RIGHT_OUTER_DELIM
	};

	/**
	 * \brief Internal delimiter store.
	 */
	delim_store_type delims_;

	/**
	 * \brief Get delimiter \c d.
	 *
	 * \return Delimiter \c d.
	 */
	std::string delim_get(const Delimiter d) const;

	/**
	 * \brief Set delimiter string for delimiter \c d.
	 *
	 * \param[in] d Delimiter type to set string for
	 * \param[in] s Delimiter \c d.
	 */
	void delim_set(const Delimiter d, const std::string& s);

	/**
	 * \brief Internal splitter for formatting multiline cells.
	 */
	std::unique_ptr<StringSplitter> splitter_;
};


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

	virtual std::string do_decorate_set(std::string&& s) const
	= 0;

	virtual std::string do_decorate_unset(std::string&& s) const
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
	 * \brief Constructor for a decorator with \c n flags.
	 *
	 * \param[in] n Total number of flags
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

} // namespace table


namespace details
{

using arcsapp::table::CellDecorator;

class DecoratorRegistry;
bool operator == (const DecoratorRegistry& lhs, const DecoratorRegistry& rhs)
	noexcept;

/**
 * \brief Internal storage interface for CellDecorators.
 */
class DecoratorRegistry final
{
public:

	friend bool operator == (const DecoratorRegistry& lhs,
			const DecoratorRegistry& rhs) noexcept;

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
	 * \brief Mark cell \c i, \c j as decorated.
	 *
	 * \param[in] i Row
	 * \param[in] j Column
	 */
	void mark_decorated(const int i, const int j);

	/**
	 * \brief Mark cell \c i, \c j as not decorated.
	 *
	 * \param[in] i Row
	 * \param[in] j Column
	 */
	void unmark_decorated(const int i, const int j);

	/**
	 * \brief TRUE iff \c i, \c j is marked as decorated.
	 *
	 * \param[in] i Row
	 * \param[in] j Column
	 *
	 * \return TRUE iff \c i, \c j is marked as decorated
	 */
	bool is_decorated(const int i, const int j);

private:

	/**
	 * \brief Add decorator for the specified index to the registry.
	 *
	 * \param[in] d   Decorator to add to the registry
	 * \param[in] idx Inner row or column index
	 */
	void add_decorator(std::unique_ptr<CellDecorator> d, const int idx);

	/**
	 * \brief Return decorator for the specified index or nullptr.
	 *
	 * \param[in] idx Inner row or column index
	 *
	 * \return Decorator for this row or column index or nullptr
	 */
	CellDecorator* get_decorator(const int idx) const;

	/**
	 * \brief Convert row to inner row index.
	 *
	 * \param[in] i Row
	 *
	 * \return Inner row index
	 */
	int row_idx(const int i) const;

	/**
	 * \brief Convert column to inner column index.
	 *
	 * \param[in] j Column
	 *
	 * \return Inner col index
	 */
	int col_idx(const int j) const;

	/**
	 * \brief Set flag for a cell to TRUE or FALSE.
	 *
	 * \param[in] i Row
	 * \param[in] j Column
	 * \param[in] f Boolean value for flag
	 */
	void set_flag(const int i, const int j, const bool f);

	/**
	 * \brief Worker to set a flag to a single decorator.
	 *
	 * \param[in] d CellDecorator to set a flag on
	 * \param[in] n Row or Column to be flagged
	 * \param[in] f Boolean value for flag
	 */
	void set_flag_worker(CellDecorator* d, const int n, const bool f);

	/**
	 * \brief Worker to check whether decorator is set on entry \c n.
	 *
	 * \param[in] d CellDecorator to get a flag from
	 * \param[in] n Row or Column to check for flag
	 *
	 * \return TRUE, if \c n is set in \c d, otherwise FALSE
	 */
	bool is_decorated_worker(const CellDecorator* d, const int n);

	/**
	 * \brief Association of CellDecorators and row/column indices.
	 */
	std::map<int, std::unique_ptr<CellDecorator>> internal_registry_;
};

} // namespace details


namespace table
{

class DecoratedStringTable;
bool operator==(const DecoratedStringTable& lhs,
		const DecoratedStringTable& rhs);


/**
 * \brief PrintableTable with decorators attached.
 *
 * Decorators can be registered to columns as well as to rows.
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
	std::unique_ptr<details::DecoratorRegistry> registry_;


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
	DecoratedStringTable(const std::string& title, const std::size_t rows,
			const std::size_t cols);

	/**
	 * \brief Constructor.
	 *
	 * Title of the table will be an empty string.
	 *
	 * \param[in] rows  Number of rows
	 * \param[in] cols  Number of columns
	 */
	DecoratedStringTable(const std::size_t rows, const std::size_t cols);

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
	 * \brief Mark cell \c i, \c j as decorated.
	 *
	 * This activates decoration for the specified cell.
	 *
	 * Calling this function on decorated cells will have no effect.
	 *
	 * \param[in] i Row
	 * \param[in] j Column
	 */
	void mark_decorated(const int i, const int j);

	/**
	 * \brief Mark cell \c i, \c j as not be decorated.
	 *
	 * This activates decoration for the specified cell.
	 *
	 * Calling this function on undecorated cells will have no effect.
	 *
	 * \param[in] i Row
	 * \param[in] j Column
	 */
	void unmark_decorated(const int i, const int j);

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

private:

	// PrintableTable

	std::string do_title() const final;

	const std::string& do_ref(int row, int col) const final;

	std::string do_cell(int row, int col) const final;

	std::size_t do_rows() const final;

	std::string do_row_label(int row) const final;

	std::size_t do_max_height(int row) const final;

	std::size_t do_cols() const final;

	std::string do_col_label(int col) const final;

	std::size_t do_max_width(int col) const final;

	table::Align do_align(int col) const final;

	std::size_t do_optimal_width(const int col) const final;

	bool do_empty() const final;

	const StringTableLayout* do_layout() const final;
};


/**
 * \brief Prints a StringTable.
 */
class TablePrinter final
{
public:

	/**
	 * \brief Default constructor.
	 */
	TablePrinter();

	/**
	 * \brief Default destructor.
	 */
	~TablePrinter() noexcept;

	/**
	 * \brief Prints a StringTable with the specified layout to an
	 * output stream.
	 *
	 * \param[in] o   The output stream to print to
	 * \param[in] t   The StringTable to print
	 */
	void print(std::ostream& o, const PrintableTable& t) const;

private:

	class Impl;

	std::unique_ptr<Impl> impl_;
};


/**
 * \brief Output stream operator for PrintableTable.
 *
 * \param[in] o     Output stream
 * \param[in] table Table to print
 *
 * \return Output stream
 */
std::ostream& operator << (std::ostream& o, const PrintableTable& table);


/**
 * \brief Output stream operator for PrintableTable.
 *
 * \param[in] o     Output stream
 * \param[in] table Table to print
 *
 * \return Output stream
 */
std::ostream& operator << (std::ostream& o,
		const std::unique_ptr<PrintableTable>& table);


} // namespace table
} // namespace v_1_0_0
} // namespace arcsapp

#endif

