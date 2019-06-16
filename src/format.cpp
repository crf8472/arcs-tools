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


// StringLayout


StringLayout::~StringLayout() noexcept = default;


std::string StringLayout::format(const std::string &text, const int width) const
{
	return this->do_format(text, width);
}


std::string StringLayout::format(const uint32_t &number, const int width) const
{
	return this->do_format(number, width);
}


// DefaultLayout


std::string DefaultLayout::do_format(const std::string &text, const int width)
	const
{
	std::stringstream ss;

	ss << std::setw(width) << std::left << text;
	return ss.str();
}


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


std::string HexLayout::do_format(const std::string &text, const int width) const
{
	std::stringstream ss;

	ss << std::setw(width) << std::left << text;
	return ss.str();
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


void StringTable::set_column_name(const int col, const std::string &name)
{
	this->do_set_column_name(col, name);
}


void StringTable::set_column_width(const int col, const int &width)
{
	this->do_set_column_width(col, width);
}


std::string StringTable::column_name(const int col) const
{
	return this->do_column_name(col);
}


int StringTable::column_width(const int col) const
{
	return this->do_column_width(col);
}


std::size_t StringTable::columns() const
{
	return this->do_columns();
}


void StringTable::set_column_delimiter(const std::string &delim)
{
	this->do_set_column_delimiter(delim);
}


std::string StringTable::column_delimiter() const
{
	return this->do_column_delimiter();
}


void StringTable::register_layout(const int col, const StringLayout *layout)
{
	this->do_register_layout(col, layout);
}


const StringLayout* StringTable::layout(const int col) const
{
	return this->do_layout(col);
}


void StringTable::set_alignment(const int col, const bool align)
{
	this->do_set_alignment(col, align);
}


bool StringTable::alignment(const int col) const
{
	return this->do_alignment(col);
}


std::size_t StringTable::rows() const
{
	return this->do_rows();
}


void StringTable::update_cell(const int row, const int col,
		const std::string &text)
{
	this->do_update_cell(row, col, text);
}


void StringTable::update_cell(const int row, const int col, const int &number)
{
	this->do_update_cell(row, col, number);
}


std::string StringTable::cell(const int row, const int col) const
{
	return this->do_cell(row, col);
}


std::unique_ptr<Lines> StringTable::print() const
{
	return this->do_print();
}


/**
 * \brief Private implementation of a StringTableBase
 */
class StringTableBase::Impl final
{

public:

	/**
	 * \brief Constructor
	 *
	 * \param[in] rows    Number of rows (including header, if any)
	 * \param[in] columns Number of columns (including label column, if any)
	 */
	Impl(const int rows, const int columns);

	/**
	 * \brief Default destructor
	 */
	~Impl() noexcept;

	/**
	 * The table content
	 */
	std::vector<std::vector<std::string>> columns_;

	/**
	 * The names
	 */
	std::vector<std::string> names_;

	/**
	 * The column widths
	 */
	std::vector<int> widths_;

	/**
	 * The column alignments
	 */
	std::vector<bool> alignments_;

	/**
	 * The default layout
	 */
	std::unique_ptr<DefaultLayout> default_layout_;

	/**
	 * The column layouts
	 */
	std::vector<const StringLayout*> layouts_;

	/**
	 * \brief Column delimiter
	 */
	std::string column_delim_;
};


StringTableBase::Impl::Impl(const int rows, const int columns)
	: columns_(columns, std::vector<std::string>(rows, ""))
	, names_(columns, "")
	, widths_(columns, 10)
	, alignments_(columns, true)
	, default_layout_{ std::make_unique<DefaultLayout>() }
	, layouts_(columns, default_layout_.get() )
	, column_delim_{ " " }
{
	// empty
}


StringTableBase::Impl::~Impl() noexcept = default;


// StringTableBase


StringTableBase::StringTableBase(const int rows, const int columns)
	: impl_(std::make_unique<StringTableBase::Impl>(rows, columns))
{
	// empty
}


StringTableBase::~StringTableBase() noexcept = default;


void StringTableBase::do_set_column_name(const int col, const std::string &name)
{
	impl_->names_[col] = name;
}


void StringTableBase::do_set_column_width(const int col, const int &width)
{
	impl_->widths_[col] = width;
}


std::string StringTableBase::do_column_name(const int col) const
{
	return impl_->names_[col];
}


int StringTableBase::do_column_width(const int col) const
{
	return impl_->widths_[col];
}


std::size_t StringTableBase::do_columns() const
{
	return impl_->columns_.size();
}


void StringTableBase::do_set_column_delimiter(const std::string &delim)
{
	impl_->column_delim_ = delim;
}


std::string StringTableBase::do_column_delimiter() const
{
	return impl_->column_delim_;
}


void StringTableBase::do_register_layout(const int col,
		const StringLayout *layout)
{
	impl_->layouts_[col] = layout;
}


const StringLayout* StringTableBase::do_layout(const int col) const
{
	return impl_->layouts_[col];
}


void StringTableBase::do_set_alignment(const int col, const bool align)
{
	impl_->alignments_[col] = align;
}


bool StringTableBase::do_alignment(const int col) const
{
	return impl_->alignments_[col];
}


std::size_t StringTableBase::do_rows() const
{
	return impl_->columns_[0].size();
}


void StringTableBase::do_update_cell(const int row, const int col,
		const std::string &text)
{
	impl_->columns_[col][row] = this->layout(col)->format(text,
			this->column_width(col));
}


void StringTableBase::do_update_cell(const int row, const int col,
		const int &number)
{
	impl_->columns_[col][row] = this->layout(col)->format(number,
			this->column_width(col));
}


std::string StringTableBase::do_cell(const int row, const int col) const
{
	return impl_->columns_[col][row];
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

	for (std::size_t col = 0; col < impl_->columns_.size(); ++col)
	{
		buf << std::setw(this->column_width(col)) << std::left
			<< this->column_name(col)
			<< this->impl_->column_delim_;
	}
	lines->append(buf.str());

	buf.str("");
	buf.clear();

	for (std::size_t row = 0; row < this->rows(); ++row)
	{
		for (std::size_t col = 0; col < impl_->columns_.size(); ++col)
		{
			buf << std::setw(this->column_width(col))
				<< (this->alignment(col) > 0 ? std::left : std::right)
				<< this->cell(row, col)
				<< this->impl_->column_delim_;
		}

		lines->append(buf.str());

		buf.str("");
		buf.clear();
	}

	return lines;
}

