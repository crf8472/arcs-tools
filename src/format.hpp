#ifndef __ARCSTOOLS_FORMAT_HPP__
#define __ARCSTOOLS_FORMAT_HPP__

/**
 * \file
 *
 * \brief Interface to output formats for printing.
 *
 * Provides OutputFormat, an abstract base class that encapsulates all actual
 * details for formatting the output. After different format() methods may have
 * been called, the actual output lines can be requested as an instance of
 * Lines, a class that represents a sequence of text lines to be printed to an
 * output stream from first to last.
 */

#include <cstdint>
#include <deque>
#include <memory>
#include <string>
#include <vector>

/**
 * \brief A sequence of lines formatted for output.
 */
class Lines
{

public:

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~Lines() noexcept;

	/**
	 * \brief Number of lines.
	 *
	 * \return The number of lines
	 */
	std::size_t size() const;

	/**
	 * \brief Get a line by index.
	 *
	 * Trying to get a line with an index bigger than size() - 1 results in an
	 * exception.
	 *
	 * \param[in] i The index to get the line for
	 *
	 * \return The line with index i
	 */
	std::string get(const uint32_t &i) const;

	/**
	 * \brief Add a line as first line.
	 *
	 * \param[in] line The line to set as new first line
	 */
	void prepend(const std::string &line);

	/**
	 * \brief Add a line as last line.
	 *
	 * \param[in] line The line to set as new last line
	 */
	void append(const std::string &line);

	/**
	 * \brief Append other lines as new lines below the existing lines.
	 *
	 * \param[in] lines Lines to append
	 */
	void append(const Lines &lines);

	/**
	 * \brief TRUE iff there are no actual lines in the sequence.
	 *
	 * Equivalent to the check whether size() is 0.
	 *
	 * \return TRUE iff there are no actual lines in this sequence
	 */
	bool empty() const;


private:

	/**
	 * \brief Implements size() const.
	 *
	 * \return The number of lines
	 */
	virtual std::size_t do_size() const
	= 0;

	/**
	 * \brief Implements get(const uint32_t &i) const.
	 *
	 * \param[in] i The index to get the line for
	 *
	 * \return The line with index i
	 */
	virtual std::string do_get(const uint32_t &i) const
	= 0;

	/**
	 * \brief Implements prepend(const std::string &line)
	 *
	 * \param[in] line The line to set as new first line
	 */
	virtual void do_prepend(const std::string &line)
	= 0;

	/**
	 * \brief Implements append(const std::string &line)
	 *
	 * \param[in] line The line to set as new last line
	 */
	virtual void do_append(const std::string &line)
	= 0;

	/**
	 * \brief Implements empty() const.
	 *
	 * \return TRUE iff there are no actual lines in this sequence
	 */
	virtual bool do_empty() const
	= 0;
};


/**
 * \brief Default implementation of Lines.
 */
class DefaultLines final : virtual public Lines
{

public: /* types */

	/**
	 * \brief Const_iterator for Lines
	 */
	using const_iterator = std::deque<std::string>::const_iterator;


public: /* functions */

	/**
	 * \brief Default constructor.
	 */
	DefaultLines();

	/**
	 * \brief Copy constructor
	 *
	 * \param[in] rhs The instance to copy
	 */
	DefaultLines(const DefaultLines &rhs);

	/**
	 * \brief Const iterator pointing to the first line.
	 *
	 * \return Iterator pointing to first line
	 */
	const_iterator begin() const;

	/**
	 * \brief Const iterator pointing behind the last line.
	 *
	 * \return Iterator pointing behind the last line
	 */
	const_iterator end() const;

	/**
	 * \brief Const iterator pointing to the first line.
	 *
	 * \return Const iterator pointing to first line
	 */
	const_iterator cbegin() const;

	/**
	 * \brief Const iterator pointing behind the last line.
	 *
	 * \return Const iterator pointing behind the last line
	 */
	const_iterator cend() const;

	std::size_t do_size() const final;

	std::string do_get(const uint32_t &i) const final;

	void do_prepend(const std::string &line) final;

	void do_append(const std::string &line) final;

	bool do_empty() const final;


private:

	/**
	 * \brief Internal representation of the lines.
	 */
	std::deque<std::string> lines_;
};


/**
 * \brief Abstract base class for output formats.
 *
 * A format provides lines for output. The interface for the actual formatting
 * is not specified but as a convention, the subclasses use a method format()
 * that recieves the object to be formatted as an argument.
 */
class OutputFormat
{

public:

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~OutputFormat() noexcept;

	/**
	 * \brief Return lines formatted for output.
	 *
	 * \return The output according to the formatting rules of this instance
	 */
	std::unique_ptr<Lines> lines();


private:

	/**
	 * \brief Implements lines().
	 *
	 * \return The output according to the formatting rules of this instance
	 */
	virtual std::unique_ptr<Lines> do_lines()
	= 0;
};


/**
 * \brief Interface for formatting strings
 */
class StringLayout
{

public:

	/**
	 * \brief Virtual default destructor
	 */
	virtual ~StringLayout() noexcept;

	/**
	 * \brief Layout a string
	 *
	 * \param[in] text  Text to format
	 * \param[in] width Width for format
	 */
	std::string format(const std::string &text, const int width) const;

	/**
	 * \brief Layout an unsigned 32 bit integer
	 *
	 * \param[in] number  Number to format
	 * \param[in] width   Width to format
	 */
	std::string format(const uint32_t &number, const int width) const;


private:

	/**
	 * \brief Implements StringLayout::format(const std::string &text, const int width) const
	 *
	 * \param[in] text  Text to format
	 * \param[in] width Width for format
	 */
	virtual std::string do_format(const std::string &text, const int width)
		const
	= 0;

	/**
	 * \brief Implements StringLayout::(const uint32_t &number, const int width) const
	 *
	 * \param[in] number  Number to format
	 * \param[in] width   Width to format
	 */
	virtual std::string do_format(const uint32_t &number, const int width) const
	= 0;
};


/**
 * \brief Default layout: unaltered right numbers, unaltered left text
 */
class DefaultLayout : public StringLayout
{

private:

	std::string do_format(const std::string &text, const int width) const
		override;

	std::string do_format(const uint32_t &number, const int width) const
		override;
};


/**
 * \brief Hexadecimal numbers, unaltered text
 */
class HexLayout : public StringLayout
{

public:

	/**
	 * \brief Constructor
	 */
	HexLayout();

	/**
	 * \brief Make the base '0x' visible
	 *
	 * \param[in] base Flag for showing the base
	 */
	void set_show_base(const bool base);

	/**
	 * \brief Return TRUE if the base is shown, otherwise FALSE
	 *
	 * \return TRUE if the base is shown, otherwise FALSE
	 */
	bool shows_base() const;

	/**
	 * \brief Make the hex digits A-F uppercase
	 *
	 * \param[in] base Flag for making hex digits A-F uppercase
	 */
	void set_uppercase(const bool base);

	/**
	 * \brief Return TRUE if A-F are uppercase, otherwise FALSE
	 *
	 * \return TRUE if A-F are uppercase, otherwise FALSE
	 */
	bool is_uppercase() const;


private:

	std::string do_format(const std::string &text, const int width) const
		override;

	std::string do_format(const uint32_t &number, const int width) const
		override;

	/**
	 * Internal layout flags
	 */
	uint8_t flags_;
};


/**
 * \brief A table with formatted columns
 */
class StringTable
{

public:

	/**
	 * \brief Virtual destructor.
	 */
	virtual ~StringTable() noexcept;

	/**
	 * \brief Set the column name
	 *
	 * \param[in] col  The column id
	 * \param[in] name The column name
	 */
	void set_column_name(const int col, const std::string &name);

	/**
	 * \brief Set the column width
	 *
	 * \param[in] col  The column id
	 * \param[in] width Widht of the column
	 */
	void set_column_width(const int col, const int &width);

	/**
	 * \brief Get name of a column
	 *
	 * \param[in] col The column to get the name of
	 *
	 * \return Name of the column
	 *
	 * \throws std::exception If col > columns or col < 0.
	 */
	std::string column_name(const int col) const;

	/**
	 * \brief Get width of a column
	 *
	 * \param[in] col The column to get the width of
	 *
	 * \return Width of the column
	 *
	 * \throws std::exception If col > columns or col < 0.
	 */
	int column_width(const int col) const;

	/**
	 * \brief Returns the number of columns
	 *
	 * \return The number of columns
	 */
	std::size_t columns() const;

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
	 * \brief Register a cell format for the specified column.
	 *
	 * This does not affect the header (row 0) but only rows with actual values.
	 *
	 * \param[in] col    The column ID
	 * \param[in] layout The layout
	 */
	void register_layout(const int col, const StringLayout *layout);

	/**
	 * \brief Return the layout of the specified column
	 *
	 * \returns The layout of the specified column
	 */
	const StringLayout* layout(const int col) const;

	/**
	 * \brief Set alignment for the specified column
	 *
	 * \param[in] col   The column ID
	 * \param[in] align Alignment
	 */
	void set_alignment(const int col, const bool align);

	/**
	 * \brief Alignment of the specified column
	 *
	 * \param[in] col   The column ID
	 *
	 * \return Alignment of this column
	 */
	bool alignment(const int col) const;

	/**
	 * \brief Returns the number of rows (without header)
	 *
	 * \return The number of rows
	 */
	std::size_t rows() const;

	/**
	 * \brief Updates an existing cell.
	 *
	 * \param[in] col  Column number
	 * \param[in] row  Row number
	 * \param[in] text New cell text
	 *
	 * \throws std::exception If col > columns or col < 0.
	 */
	void update_cell(const int row, const int col, const std::string &text);

	/**
	 * \brief Updates an existing cell.
	 *
	 * \param[in] col    Column number
	 * \param[in] row    Row number
	 * \param[in] number New number in cell
	 *
	 * \throws std::exception If col > columns or col < 0.
	 */
	void update_cell(const int row, const int col, const int &number);

	/**
	 * \brief Returns the content of the specified cell.
	 *
	 * \param[in] col  Column number
	 * \param[in] row  Row number
	 *
	 * \throws std::exception If col > columns or col < 0.
	 */
	std::string cell(const int row, const int col) const;

	/**
	 * \brief Print the table
	 *
	 * \return The printed table
	 */
	std::unique_ptr<Lines> print() const;


private:

	/**
	 * \brief Implements StringTable::
	 *
	 * \param[in] col  The column id
	 * \param[in] name The column name
	 */
	virtual void do_set_column_name(const int col, const std::string &name)
	= 0;

	/**
	 * \brief Implements StringTable::
	 *
	 * \param[in] col  The column id
	 * \param[in] width Widht of the column
	 */
	virtual void do_set_column_width(const int col, const int &width)
	= 0;

	/**
	 * \brief Implements StringTable::
	 *
	 * \param[in] col The column to get the name of
	 *
	 * \return Name of the column
	 *
	 * \throws std::exception If col > columns or col < 0.
	 */
	virtual std::string do_column_name(const int col) const
	= 0;

	/**
	 * \brief Implements StringTable::
	 *
	 * \param[in] col The column to get the width of
	 *
	 * \return Width of the column
	 *
	 * \throws std::exception If col > columns or col < 0.
	 */
	virtual int do_column_width(const int col) const
	= 0;

	/**
	 * \brief Implements StringTable::
	 *
	 * \return The number of columns
	 */
	virtual std::size_t do_columns() const
	= 0;

	/**
	 * \brief Implements StringTable::
	 *
	 * \param[in] delim The column delimiter symbol
	 */
	virtual void do_set_column_delimiter(const std::string &delim)
	= 0;

	/**
	 * \brief Implements StringTable::
	 *
	 * \return The column delimiter
	 */
	virtual std::string do_column_delimiter() const
	= 0;

	/**
	 * \brief Implements StringTable::
	 *
	 * \param[in] col    The column ID
	 * \param[in] format The format
	 */
	virtual void do_register_layout(const int col, const StringLayout *format)
	= 0;

	/**
	 * \brief Implements StringTable::
	 *
	 * \returns The format of the specified column
	 */
	virtual const StringLayout* do_layout(const int col) const
	= 0;

	/**
	 * \brief Implements StringTable::
	 *
	 * \param[in] col   The column ID
	 * \param[in] align Alignment
	 */
	virtual void do_set_alignment(const int col, const bool align)
	= 0;

	/**
	 * \brief Implements StringTable::
	 *
	 * \param[in] col   The column ID
	 *
	 * \return Alignment of this column
	 */
	virtual bool do_alignment(const int col) const
	= 0;

	/**
	 * \brief Implements StringTable::
	 *
	 * \return The number of rows
	 */
	virtual std::size_t do_rows() const
	= 0;

	/**
	 * \brief Implements StringTable::
	 *
	 * \param[in] row  Row number
	 * \param[in] col  Column number
	 * \param[in] text New cell text
	 *
	 * \throws std::exception If col > columns or col < 0.
	 */
	virtual void do_update_cell(const int row, const int col,
			const std::string &text)
	= 0;

	/**
	 * \brief Implements StringTable::
	 *
	 * \param[in] row    Row number
	 * \param[in] col    Column number
	 * \param[in] number New number in cell
	 *
	 * \throws std::exception If col > columns or col < 0.
	 */
	virtual void do_update_cell(const int row, const int col,
			const int &number)
	= 0;

	/**
	 * \brief Implements StringTable::
	 *
	 * \param[in] row  Row number
	 * \param[in] col  Column number
	 *
	 * \throws std::exception If col > columns or col < 0.
	 *
	 * \return Content of cell (row, col)
	 */
	virtual std::string do_cell(const int row, const int col) const
	= 0;

	/**
	 * \brief Implements StringTable::
	 *
	 * \return The printed table
	 */
	virtual std::unique_ptr<Lines> do_print() const
	= 0;
};


/**
 * \brief Abstract base class for StringTables
 */
class StringTableBase : public StringTable
{

public:

	/**
	 * \brief Constructor
	 *
	 * \param[in] rows    Number of rows (including header, if any)
	 * \param[in] columns Number of columns (including label column, if any)
	 */
	StringTableBase(const int rows, const int columns);

	/**
	 * \brief Non-virtual default destructor
	 */
	~StringTableBase() noexcept;


protected:

	/**
	 * \brief Validate that table has at least specified size
	 *
	 * \param[in] rows
	 * \param[in] columns
	 *
	 * \throws Exception reporting that table is too small
	 */
	void validate_table_dimensions(const unsigned int rows,
			const unsigned int columns) const;


private:

	void do_set_column_name(const int col, const std::string &name) override;

	void do_set_column_width(const int col, const int &width) override;

	std::string do_column_name(const int col) const override;

	int do_column_width(const int col) const override;

	std::size_t do_columns() const override;

	void do_set_column_delimiter(const std::string &delim) override;

	std::string do_column_delimiter() const override;

	void do_register_layout(const int col, const StringLayout *layout) override;

	const StringLayout* do_layout(const int col) const override;

	void do_set_alignment(const int col, const bool align) override;

	bool do_alignment(const int col) const override;

	std::size_t do_rows() const override;

	void do_update_cell(const int row, const int col,
			const std::string &text) override;

	void do_update_cell(const int row, const int col, const int &number)
		override;

	std::string do_cell(const int row, const int col) const override;

	std::unique_ptr<Lines> do_print() const override;


	// forward declaration
	class Impl;

	/**
	 * \brief Private implementation
	 */
	std::unique_ptr<Impl> impl_;
};

#endif

