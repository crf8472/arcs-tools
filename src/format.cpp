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


// WithInternalFlags


WithInternalFlags::WithInternalFlags(const uint32_t flags)
	: flags_(flags)
{
	// empty
}


void WithInternalFlags::set_flag(const int idx, const bool value)
{
	if (value)
	{
		flags_ |= (1 << idx);  // <= true
	} else
	{
		flags_ &= ~(0 << idx); // <= false
	}
}


bool WithInternalFlags::flag(const int idx) const
{
	return flags_ & (1 << idx);
}


// ARIdLayout


ARIdLayout::ARIdLayout()
	: WithInternalFlags(0xFFFFFFFF) // all flags true
{
	// empty
}


ARIdLayout::ARIdLayout(const bool &url, const bool &filename,
		const bool &track_count, const bool &disc_id_1, const bool &disc_id_2,
		const bool &cddb_id)
	: WithInternalFlags(
			0
			| url
			| (filename    << 1)
			| (track_count << 2)
			| (disc_id_1   << 3)
			| (disc_id_2   << 4)
			| (cddb_id     << 5)
		)
{
	// empty
}


ARIdLayout::~ARIdLayout() noexcept = default;


bool ARIdLayout::url() const
{
	return flag(0);
}


void ARIdLayout::set_url(const bool url)
{
	this->set_flag(0, url);
}


bool ARIdLayout::filename() const
{
	return flag(1);
}


void ARIdLayout::set_filename(const bool filename)
{
	this->set_flag(1, filename);
}


bool ARIdLayout::track_count() const
{
	return flag(2);
}


void ARIdLayout::set_trackcount(const bool trackcount)
{
	this->set_flag(2, trackcount);
}


bool ARIdLayout::disc_id_1() const
{
	return flag(3);
}


void ARIdLayout::set_disc_id_1(const bool disc_id_1)
{
	this->set_flag(3, disc_id_1);
}


bool ARIdLayout::disc_id_2() const
{
	return flag(4);
}


void ARIdLayout::set_disc_id_2(const bool disc_id_2)
{
	this->set_flag(4, disc_id_2);
}


bool ARIdLayout::cddb_id() const
{
	return flag(5);
}


void ARIdLayout::set_cddb_id(const bool cddb_id)
{
	this->set_flag(5, cddb_id);
}


std::string ARIdLayout::format(const ARId &id, const std::string &alt_prefix)
	const
{
	return this->do_format(id, alt_prefix);
}


// WithARId


WithARId::WithARId()
	: arid_layout_(nullptr)
{
	// empty
}


WithARId::WithARId(std::unique_ptr<ARIdLayout> arid_layout)
	: arid_layout_(std::move(arid_layout))
{
	// empty
}


WithARId::~WithARId() noexcept = default;


void WithARId::set_arid_layout(std::unique_ptr<ARIdLayout> format)
{
	if (arid_layout_)
	{
		arid_layout_.reset();
	}

	arid_layout_ = std::move(format);
}


ARIdLayout* WithARId::arid_layout()
{
	return arid_layout_ ? arid_layout_.get() : nullptr;
}


// WithMetadataFlagMethods


WithMetadataFlagMethods::WithMetadataFlagMethods(
		const bool show_track,
		const bool show_offset,
		const bool show_length,
		const bool show_filename)
	: WithInternalFlags(
			0
			| show_track
			| (show_offset << 1)
			| (show_length << 2)
			| (show_filename << 3))
{
	// empty
}


WithMetadataFlagMethods::~WithMetadataFlagMethods() noexcept = default;


bool WithMetadataFlagMethods::track() const
{
	return this->flag(0);
}


void WithMetadataFlagMethods::set_track(const bool &track)
{
	this->set_flag(0, track);
}


bool WithMetadataFlagMethods::offset() const
{
	return this->flag(1);
}


void WithMetadataFlagMethods::set_offset(const bool &offset)
{
	this->set_flag(1, offset);
}


bool WithMetadataFlagMethods::length() const
{
	return this->flag(2);
}


void WithMetadataFlagMethods::set_length(const bool &length)
{
	this->set_flag(2, length);
}


bool WithMetadataFlagMethods::filename() const
{
	return this->flag(3);
}


void WithMetadataFlagMethods::set_filename(const bool &filename)
{
	this->set_flag(3, filename);
}


// NumberLayout


NumberLayout::~NumberLayout() noexcept = default;


std::string NumberLayout::format(const uint32_t &number, const int width) const
{
	return this->do_format(number, width);
}


// HexLayout


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


// TableLayout


TableLayout::~TableLayout() noexcept = default;


std::size_t TableLayout::rows() const
{
	return this->do_rows();
}


std::size_t TableLayout::columns() const
{
	return this->do_columns();
}


void TableLayout::set_width(const int col, const int width)
{
	this->do_set_width(col, width);
}


int TableLayout::width(const int col) const
{
	return this->do_width(col);
}


void TableLayout::set_alignment(const int col, const bool align)
{
	this->do_set_alignment(col, align);
}


bool TableLayout::alignment(const int col) const
{
	return this->do_alignment(col);
}


void TableLayout::set_title(const int col, const std::string &title)
{
	this->do_set_title(col, title);
}


std::string TableLayout::title(const int col) const
{
	return this->do_title(col);
}


void TableLayout::set_type(const int col, const int type)
{
	this->do_set_type(col, type);
}


int TableLayout::type(const int col) const
{
	return this->do_type(col);
}


void TableLayout::set_column_delimiter(const std::string &delim)
{
	this->do_set_column_delimiter(delim);
}


std::string TableLayout::column_delimiter() const
{
	return this->do_column_delimiter();
}


void TableLayout::resize(const int rows, const int cols)
{
	this->do_resize(rows, cols);
}


void TableLayout::bounds_check(const int row, const int col) const
{
	this->do_bounds_check(row, col);
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

	void set_type(const int col, const int type);
	int type(const int col) const;

	void set_col_delim(const std::string &delim);
	std::string col_delim() const;

	/**
	 * \brief Perform a bounds check
	 */
	void bounds_check(const int row, const int col) const;

	/**
	 * \brief Resize to new dimensions
	 */
	void resize(const int rows, const int cols);

	std::string get(const int row, const int col) const;
	std::string cell(const int row, const int col) const;
	void update_cell(const int row, const int col, const std::string &text);

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
	std::vector<std::string> titles_;

	/**
	 * \brief The column types.
	 */
	std::vector<int> types_;

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
	, titles_       ()
	, types_        ()
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
	return this->get(row, col);
}


void StringTableBase::Impl::update_cell(const int row, const int col,
		const std::string &text)
{
	cells_[index(row, col)] = text;
}


std::string StringTableBase::Impl::get(const int row, const int col) const
{
	auto text = cells_[index(row, col)];

	if (auto width = static_cast<std::size_t>(widths_[col]);
			text.length() > width)
	{
		return text.substr(0, width - 1) + "~";
	}

	return text;
}


void StringTableBase::Impl::set_width(const int col, const int width)
{
	widths_[col] = width;
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
	titles_[col] = title;
}


std::string StringTableBase::Impl::title(const int col) const
{
	return titles_[col];
}


void StringTableBase::Impl::set_type(const int col, const int type)
{
	types_[col] = type;
}


int StringTableBase::Impl::type(const int col) const
{
	return types_[col];
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

	cells_.resize(rows_ * columns());

	widths_.resize(columns());
	alignments_.resize(columns());
	titles_.resize(columns());
	types_.resize(columns());
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


void StringTableBase::update_cell(const int row, const int col,
		const std::string &text)
{
	this->bounds_check(row, col);
	this->do_update_cell(row, col, text);
}


std::string StringTableBase::cell(const int row, const int col) const
{
	this->bounds_check(row, col);
	return this->do_cell(row, col);
}


std::string StringTableBase::operator() (const int row, const int col) const
{
	return this->do_cell(row, col);
}


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


void StringTableBase::do_set_type(const int col, const int type)
{
	impl_->set_type(col, type);
}


int StringTableBase::do_type(const int col) const
{
	return impl_->type(col);
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


std::ostream& operator << (std::ostream &out, const StringTableBase &table)
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

