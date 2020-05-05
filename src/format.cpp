/**
 * \file
 *
 * \brief Formatting interfaces and classes
 */

#ifndef __ARCSTOOLS_FORMAT_HPP__
#include "format.hpp"
#endif

#include <deque>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <vector>


// Lines


Lines::~Lines() noexcept = default;


std::size_t Lines::size() const
{
	return this->do_size();
}


std::string Lines::get(const uint32_t &i) const
{
	return this->do_get(i);
}


void Lines::prepend(const std::string &line)
{
	this->do_prepend(line);
}


void Lines::append(const std::string &line)
{
	this->do_append(line);
}


void Lines::append(const Lines &lines)
{
	for (std::size_t i = 0; i < lines.size(); ++i)
	{
		this->append(lines.get(i));
	}
}


bool Lines::empty() const
{
	return this->do_empty();
}


// DefaultLines


DefaultLines::DefaultLines()
	: lines_()
{
	// empty
}


DefaultLines::DefaultLines(const DefaultLines &rhs) = default;


DefaultLines::const_iterator DefaultLines::begin() const
{
	return lines_.begin();
}


DefaultLines::const_iterator DefaultLines::end() const
{
	return lines_.end();
}


DefaultLines::const_iterator DefaultLines::cbegin() const
{
	return lines_.cbegin();
}


DefaultLines::const_iterator DefaultLines::cend() const
{
	return lines_.cend();
}


std::size_t DefaultLines::do_size() const
{
	return lines_.size();
}


std::string DefaultLines::do_get(const uint32_t &i) const
{
	return lines_.at(i);
}


void DefaultLines::do_prepend(const std::string &line)
{
	lines_.insert(lines_.begin(), line);
}


void DefaultLines::do_append(const std::string &line)
{
	lines_.push_back(line);
}


bool DefaultLines::do_empty() const
{
	return lines_.empty();
}


// OutputFormat


OutputFormat::~OutputFormat() noexcept = default;


std::unique_ptr<Lines> OutputFormat::lines()
{
	return do_lines();
}


// NumberLayout


NumberLayout::~NumberLayout() noexcept = default;


std::string NumberLayout::format(const uint32_t &number, const int width) const
{
	return this->do_format(number, width);
}


// DefaultLayout


std::string DefaultLayout::do_format(const uint32_t &number, const int width)
	const
{
	std::stringstream ss;

	ss << std::dec << std::setw(width) << std::right << number;
	return ss.str();
}


// Hex8Layout


HexLayout::HexLayout()
	: flags_{ 0 }
{
	// empty
}


void HexLayout::set_show_base(const bool base)
{
	if (base)
	{
		flags_ |= 1;
	} else
	{
		flags_ &= ~1;
	}
}


bool HexLayout::shows_base() const
{
	return flags_ & 1;
}


void HexLayout::set_uppercase(const bool uppercase)
{
	if (uppercase)
	{
		flags_ |= (1 << 1);
	} else
	{
		flags_ &= ~(1 << 1);
	}
}


bool HexLayout::is_uppercase() const
{
	return flags_ & (1 << 1);
}


std::string HexLayout::do_format(const uint32_t &number, const int width) const
{
	std::stringstream ss;

	if (this->shows_base())
	{
		ss << std::showbase;
	}

	if (this->is_uppercase())
	{
		ss << std::uppercase;
	}

	// ss_flags  =  ss.flags();
	// ss_flags &= ~ss.basefield;
	// ss_flags &= ~ss.adjustfield;
	// ss.flags(ss_flags);

	ss << std::hex << std::setw(width) << std::setfill('0') << number;
	return ss.str();
}


// StringTable


StringTable::~StringTable() noexcept = default;


std::size_t StringTable::rows() const
{
	return this->do_rows();
}


std::size_t StringTable::columns() const
{
	return this->do_columns();
}


void StringTable::set_width(const int col, const int &width)
{
	this->do_set_width(col, width);
}


int StringTable::width(const int col) const
{
	return this->do_width(col);
}


void StringTable::set_alignment(const int col, const bool align)
{
	this->do_set_alignment(col, align);
}


bool StringTable::alignment(const int col) const
{
	return this->do_alignment(col);
}


void StringTable::set_title(const int col, const std::string &title)
{
	this->do_set_title(col, title);
}


std::string StringTable::title(const int col) const
{
	return this->do_title(col);
}


void StringTable::set_column_delimiter(const std::string &delim)
{
	this->do_set_column_delimiter(delim);
}


std::string StringTable::column_delimiter() const
{
	return this->do_column_delimiter();
}


void StringTable::update_cell(const int row, const int col,
		const std::string &text)
{
	this->bounds_check(row, col);
	this->do_update_cell(row, col, text);
}


std::string StringTable::cell(const int row, const int col) const
{
	this->bounds_check(row, col);
	return this->do_cell(row, col);
}


std::string StringTable::operator() (const int row, const int col) const
{
	return this->do_cell(row, col);
}


void StringTable::resize(const int rows, const int cols)
{
	this->do_resize(rows, cols);
}


std::unique_ptr<Lines> StringTable::print() const
{
	return this->do_print();
}


void StringTable::bounds_check(const int row, const int col) const
{
	this->do_bounds_check(row, col);
}


std::ostream& operator << (std::ostream &out, const StringTable &table)
{
	std::size_t col = 0;

	// Column titles
	for (; col < table.columns(); ++col)
	{
		out << std::setw(table.width(col)) << std::left
			<< table.title(col)
			<< table.column_delimiter();
	}
	out << std::endl;

	// Row contents
	for (std::size_t row = 0; row < table.rows(); ++row)
	{
		for (col = 0; col < table.columns(); ++col)
		{
			out << std::setw(table.width(col))
				<< (table.alignment(col) > 0 ? std::left : std::right)
				<< table.cell(row, col)
				<< table.column_delimiter();
		}
		out << std::endl;
	}

	return out;
}


/**
 * \brief Private implementation of a StringTableBase
 */
class StringTableBase::Impl final
{

public:

	Impl(const int rows, const int columns);

	~Impl() noexcept;

	int rows() const;
	int columns() const;

	void set_width(const int col, const int width);
	int width(const int col);

	void set_alignment(const int col, const bool align);
	bool alignment(const int col);

	void set_title(const int col, const std::string &title);
	std::string title(const int col) const;

	void set_col_delim(const std::string &delim);
	std::string col_delim() const;

	std::string get(const int row, const int col) const;

	std::string cell(const int row, const int col) const;
	void update_cell(const int row, const int col, const std::string &text);

	/**
	 * \brief Perform a bounds check
	 */
	void bounds_check(const int row, const int col) const;

	/**
	 * \brief Resize to new dimensions
	 */
	void resize(const int rows, const int cols);

private:

	/**
	 * \brief Return index of the cell in row \c row dn column \c col.
	 *
	 * \param[in] row Row index
	 * \param[in] col Column index
	 *
	 * \return Index of the table cell
	 */
	int index(const int row, const int col) const;

	bool legal_row(const int row) const;

	bool legal_col(const int col) const;

	bool legal_width(const int width) const;

private:

	/**
	 * \brief The table content.
	 */
	std::vector<std::string> cells_;

	/**
	 * \brief The column widths.
	 */
	std::vector<int> widths_;

	/**
	 * \brief The column alignments.
	 */
	std::vector<bool> alignments_;

	/**
	 * \brief The column titles.
	 */
	std::vector<std::string> column_titles_;

	/**
	 * \brief Number of rows.
	 */
	int rows_;

	/**
	 * \brief Number of columns.
	 */
	int cols_;

	/**
	 * \brief Column delimiter.
	 */
	std::string column_delim_;
};


StringTableBase::Impl::Impl(const int rows, const int cols)
	: cells_        ()
	, widths_       ()
	, alignments_   ()
	, column_titles_()
	, rows_         { rows }
	, cols_         { cols }
	, column_delim_ { " " }
{
	resize(rows, cols);
}


StringTableBase::Impl::~Impl() noexcept = default;


int StringTableBase::Impl::rows() const
{
	return rows_;
}


int StringTableBase::Impl::columns() const
{
	return cols_;
}


std::string StringTableBase::Impl::cell(const int row, const int col) const
{
	return get(row, col);
}


void StringTableBase::Impl::update_cell(const int row, const int col,
		const std::string &text)
{
	if (text.length() > static_cast<std::size_t>(widths_[col]))
	{
		// TODO
	}

	cells_[index(row, col)] = text;
}


std::string StringTableBase::Impl::get(const int row, const int col) const
{
	return cells_[index(row, col)];
}


void StringTableBase::Impl::set_width(const int col, const int width)
{
	if (legal_col(col) and legal_width(width))
	{
		widths_.insert(widths_.begin() + col, width);
	}
}


int StringTableBase::Impl::width(const int col)
{
	return widths_[col];
}


void StringTableBase::Impl::set_alignment(const int col, const bool align)
{
	alignments_[col] = align;
}


bool StringTableBase::Impl::alignment(const int col)
{
	return alignments_[col];
}


void StringTableBase::Impl::set_title(const int col,
		const std::string &title)
{
	column_titles_[col] = title;
}


std::string StringTableBase::Impl::title(const int col) const
{
	return column_titles_[col];
}


void StringTableBase::Impl::set_col_delim(const std::string &delim)
{
	column_delim_ = delim;
}


std::string StringTableBase::Impl::col_delim() const
{
	return column_delim_;
}


void StringTableBase::Impl::resize(const int rows, const int cols)
{
	rows_ = rows;
	cols_ = cols;

	cells_.resize(rows_ * cols_);

	widths_.resize(cols_);
	alignments_.resize(cols_);
	column_titles_.resize(cols_);
}


int StringTableBase::Impl::index(const int row, const int col) const
{
	return row * columns() + col;
}


void StringTableBase::Impl::bounds_check(const int row, const int col) const
{
	if (not legal_row(row))
	{
		throw std::out_of_range("Row index " + std::to_string(row) +
				" illegal");
	}

	if (not legal_col(col))
	{
		throw std::out_of_range("Column index " + std::to_string(col) +
				"illegal");
	}
}


bool StringTableBase::Impl::legal_row(const int row) const
{
	return row >= 0 && row < rows();
}


bool StringTableBase::Impl::legal_col(const int col) const
{
	return col >= 0 && col < columns();
}


bool StringTableBase::Impl::legal_width(const int width) const
{
	return width > 0 && width < 32; // defines maximum width
}


// StringTableBase


StringTableBase::StringTableBase(const int rows, const int columns)
	: impl_(std::make_unique<StringTableBase::Impl>(rows, columns))
{
	// empty
}


StringTableBase::~StringTableBase() noexcept = default;


std::size_t StringTableBase::do_rows() const
{
	return impl_->rows();
}


std::size_t StringTableBase::do_columns() const
{
	return impl_->columns();
}


void StringTableBase::do_resize(const int rows, const int cols)
{
	impl_->resize(rows, cols);
}


void StringTableBase::do_set_width(const int col, const int width)
{
	impl_->set_width(col, width);
}


int StringTableBase::do_width(const int col) const
{
	return impl_->width(col);
}


void StringTableBase::do_set_alignment(const int col, const bool align)
{
	impl_->set_alignment(col, align);
}


bool StringTableBase::do_alignment(const int col) const
{
	return impl_->alignment(col);
}


void StringTableBase::do_set_title(const int col, const std::string &title)
{
	impl_->set_title(col, title);
}


std::string StringTableBase::do_title(const int col) const
{
	return impl_->title(col);
}


void StringTableBase::do_set_column_delimiter(const std::string &delim)
{
	impl_->set_col_delim(delim);
}


std::string StringTableBase::do_column_delimiter() const
{
	return impl_->col_delim();
}


void StringTableBase::do_update_cell(const int row, const int col,
		const std::string &text)
{
	this->bounds_check(row, col);
	impl_->update_cell(row, col, text);
	//impl_->columns_[col][row] = this->layout(col)->format(text,
	//		this->column_width(col));
}


std::string StringTableBase::do_cell(const int row, const int col) const
{
	this->bounds_check(row, col);
	return impl_->cell(row, col);
}


void StringTableBase::do_bounds_check(const int row, const int col) const
{
	impl_->bounds_check(row, col);
}


void StringTableBase::validate_table_dimensions(const unsigned int rows,
		const unsigned int columns) const
{
	if (rows > this->rows())
	{
		throw std::logic_error("Too much rows for this table");
	}

	if (columns > this->columns())
	{
		throw std::logic_error("Too much columns for this table");
	}
}


std::unique_ptr<Lines> StringTableBase::do_print() const
{
	std::unique_ptr<DefaultLines> lines { std::make_unique<DefaultLines>() };

	std::stringstream buf;

	for (std::size_t col = 0; col < this->columns(); ++col)
	{
		buf << std::setw(this->width(col)) << std::left
			<< this->title(col)
			<< this->column_delimiter();
	}
	lines->append(buf.str());

	buf.str("");
	buf.clear();

	for (std::size_t row = 0; row < this->rows(); ++row)
	{
		for (std::size_t col = 0; col < this->columns(); ++col)
		{
			buf << std::setw(this->width(col))
				<< (this->alignment(col) > 0 ? std::left : std::right)
				<< this->cell(row, col)
				<< this->column_delimiter();
		}

		lines->append(buf.str());

		buf.str("");
		buf.clear();
	}

	return lines;
}

