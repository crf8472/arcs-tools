/**
 * \file
 *
 * \brief Implementation of a printable table holdings strings.
 */

#include <algorithm>  // for max, min, find_if, for_each
#include <cstddef>    // for size_t
#include <cctype>     // for isspace
#include <functional> // for bind, function, placeholders
#include <iomanip>    // for setw, setfill
#include <ios>        // for left, right
#include <iterator>   // for begin, end
#include <map>        // for map
#include <memory>     // for unique_ptr, make_unique
#include <ostream>    // for ostream
#include <stdexcept>  // for runtime_error
#include <sstream>    // for ostringstream
#include <string>     // for string
#include <type_traits>// for underlying_type_t
#include <utility>    // for forward, make_pair, move, swap


#ifndef __ARCSTOOLS_TABLE_HPP__
#include "table.hpp"
#endif

namespace arcsapp
{

namespace details
{


std::string trim(std::string str)
{
	static const auto whitespace = [](int c) { return !std::isspace(c); };

	// left
	str.erase(str.begin(), std::find_if(str.begin(), str.end(), whitespace));

	// right
	str.erase(std::find_if(str.rbegin(), str.rend(), whitespace).base(),
			str.end());

	return str;
}


std::vector<std::string> split(std::string str, const std::size_t max_len)
{
	if (str.empty())
	{
		return {};
	}

	if (str.length() < max_len)
	{
		return { str };
	}

	std::vector<std::string> parts;

	for (std::size_t i = 0; i < str.length(); i += max_len)
	{
		parts.push_back(str.substr(i, max_len));
	}

	return parts;
}


std::vector<std::string> split(std::string str, const std::string& delim)
{
	if (str.empty())
	{
		return {};
	}

	std::vector<std::string> parts;

	std::size_t pos = 0;
	while ((pos = str.find(delim)) != std::string::npos)
	{
		parts.push_back(str.substr(0, pos));
		str.erase(0, pos + delim.length());
	}

	return parts.empty() ? std::vector<std::string>{ str } : parts;
}


/**
 * \brief Insert or resize.
 */
template <typename T>
void insert_or_resize(std::vector<T>& v, const int rows, const int row,
		T&& value)
{
	if (v.empty())
	{
		v.resize(rows);
	} else
	{
		v.insert(std::begin(v) + row, std::forward<T>(value));
	}
}

} // details

namespace table
{


StringTable::StringTable(const std::string& title, int rows, int cols)
	: title_              { title }
	, rows_               { rows }
	, default_max_height_ {  5 } // Max height for a row is 5 lines
	, cols_               { cols }
	, default_max_width_  {  8 } // Max width for a col is 8 characters
	// std::vector constructor calls done with parentheses (size, default value)
	, row_labels_         ( this->rows() /* empty string */ )
	, row_max_heights_    ( this->rows(), this->default_max_height() )
	, col_labels_         ( this->cols() /* empty string */ )
	, col_max_widths_     ( this->cols(), this->default_max_width() )
	, aligns_             ( this->cols(), Align::LEFT )
	, cells_              ( this->rows() * this->cols() )
	, layout_             { std::make_unique<StringTableLayout>() }
{
	// empty
}


StringTable::StringTable(int rows, int cols)
	: StringTable("" /* empty title */, rows, cols)
{
	// empty
}


StringTable::StringTable(const StringTable& rhs)
	: title_              { rhs.title_ }
	, rows_               { rhs.rows_ }
	, default_max_height_ { rhs.default_max_height_ }
	, cols_               { rhs.cols_ }
	, default_max_width_  { rhs.default_max_width_ }
	, row_labels_         { rhs.row_labels_ }
	, row_max_heights_    { rhs.row_max_heights_ }
	, col_labels_         { rhs.col_labels_ }
	, col_max_widths_     { rhs.col_max_widths_ }
	, aligns_             { rhs.aligns_ }
	, cells_              { rhs.cells_ }
	, layout_             { std::make_unique<StringTableLayout>(*rhs.layout_) }
{
	// empty
}


StringTable& StringTable::operator=(StringTable rhs) noexcept
{
	// Note that this operator also takes rvalue references.
	// See discussion at
	// https://stackoverflow.com/questions/19841626/move-assignment-incompatible-with-standard-copy-and-swap
	using std::swap;
	swap(*this, rhs);
	return *this;
}


StringTable::StringTable(StringTable&& rhs) noexcept
	: title_              { std::move(rhs.title_) }
	, rows_               { std::move(rhs.rows_) }
	, default_max_height_ { std::move(rhs.default_max_height_) }
	, cols_               { std::move(rhs.cols_) }
	, default_max_width_  { std::move(rhs.default_max_width_) }
	, row_labels_         { std::move(rhs.row_labels_) }
	, row_max_heights_    { std::move(rhs.row_max_heights_) }
	, col_labels_         { std::move(rhs.col_labels_) }
	, col_max_widths_     { std::move(rhs.col_max_widths_) }
	, aligns_             { std::move(rhs.aligns_) }
	, cells_              { std::move(rhs.cells_) }
	, layout_             { std::move(rhs.layout_) }
{
	// empty
}


StringTable::~StringTable() noexcept = default;


void StringTable::set_title(const std::string &title)
{
	title_ = title;
}


std::string StringTable::title() const
{
	return title_;
}


std::string StringTable::operator()(int row, int col) const
{
	return cells_[safe_index(row, col)];
}


std::string& StringTable::operator()(int row, int col)
{
	return cells_[safe_index(row, col)];
}


std::string StringTable::cell(int row, int col) const
{
	return this->operator()(row, col);
}


std::string& StringTable::cell(int row, int col)
{
	const auto rows { row - this->rows() + 1 };
	if (rows > 0)
	{
		this->append_rows(rows);
	}
	return this->operator()(row, col);
}


const std::string& StringTable::ref(int row, int col) const
{
	return cells_[safe_index(row, col)];
}


int StringTable::rows() const
{
	return rows_;
}


void StringTable::set_row_label(int row, const std::string& label)
{
	row_labels_[row] = label;
}


std::string StringTable::row_label(int row) const
{
	return row_labels_[row];
}


void StringTable::set_max_height(int row, std::size_t height)
{
	row_max_heights_[row] = height;
}


std::size_t StringTable::max_height(int row) const
{
	return row_max_heights_[row];
}


std::size_t StringTable::default_max_height() const
{
	return default_max_height_;
}


int StringTable::cols() const
{
	return cols_;
}


void StringTable::set_col_label(int col, const std::string& label)
{
	col_labels_[col] = label;
}


std::string StringTable::col_label(int col) const
{
	return col_labels_[col];
}


void StringTable::set_max_width(int col, std::size_t width)
{
	col_max_widths_[col] = width;
}


std::size_t StringTable::max_width(int col) const
{
	return col_max_widths_[col];
}


std::size_t StringTable::default_max_width() const
{
	return default_max_width_;
}


void StringTable::set_align(int col, Align align)
{
	aligns_[col] = align;
}


Align StringTable::align(int col) const
{
	return aligns_[col];
}


std::size_t StringTable::optimal_width(const int col) const
{
	auto width = std::size_t { 0 };
	auto optimal_width { width };

	for(auto row = int { 0 }; row < rows(); ++row)
	{
		width = cell(row, col).length();
		if (width > optimal_width) { optimal_width = width; }
	}

	return optimal_width;
}


bool StringTable::empty() const
{
	return cells_.empty();
}


bool StringTable::exists(const int row, const int col) const
{
	const auto i { index(row, col) };
	return exists(i);
}


bool StringTable::row_exists(const int row) const
{
	return exists(row, 0);
}


bool StringTable::col_exists(const int col) const
{
	return exists(0, col);
}


void StringTable::insert_rows_after(const int rows, const int row)
{
	if (!this->cols())
	{
		throw std::runtime_error(
				"Cannot insert row in table with zero columns");
	}

	using std::begin;
	using std::end;

	if (cells_.empty())
	{
		cells_.resize(rows * this->cols());
	} else
	{
		auto after_pos { begin(cells_) };
		after_pos += index(row, this->cols());

		store_type new_rows(rows * this->cols());
		cells_.insert(after_pos, begin(new_rows), end(new_rows));
	}
	rows_ += rows;

	details::insert_or_resize(row_labels_,      rows, row, std::string());
	details::insert_or_resize(row_max_heights_, rows, row,
			default_max_height());
}


void StringTable::append_rows(const int rows)
{
	if (rows > 0)
	{
		insert_rows_after(rows, std::max(0, this->rows() - 1));
	}
}


void StringTable::set_layout(std::unique_ptr<StringTableLayout> l)
{
	layout_ = std::move(l);
}


const StringTableLayout* StringTable::layout() const
{
	return layout_.get();
}


void StringTable::swap(StringTable& rhs) noexcept
{
	using std::swap;
	swap(title_, rhs.title_);
	swap(rows_, rhs.rows_);
	swap(default_max_height_, rhs.default_max_height_);
	swap(cols_, rhs.cols_);
	swap(default_max_width_, rhs.default_max_width_);
	swap(row_labels_, rhs.row_labels_);
	swap(row_max_heights_, rhs.row_max_heights_);
	swap(col_labels_, rhs.col_labels_);
	swap(col_max_widths_, rhs.col_max_widths_);
	swap(aligns_, rhs.aligns_);
	swap(cells_, rhs.cells_);
	swap(layout_, rhs.layout_);
}


StringTable::index_type StringTable::index(const int row, const int col) const
{
	return row * this->cols() + col;
}


StringTable::index_type StringTable::safe_index(const int row, const int col)
	const
{
	const auto i { index(row, col) };
	if (exists(i))
	{
		return i;
	}

	std::ostringstream ss;
	ss << "Cell at " << row << "," << col << " cannot be accessed";
	throw std::runtime_error(ss.str());
}


// StringSplitter


std::vector<std::string> StringSplitter::split(const std::string& str,
			const std::size_t max_len) const
{
		return do_split(str, max_len);
}


std::unique_ptr<StringSplitter> StringSplitter::clone() const
{
	return do_clone();
}


// DefaultSplitter


std::vector<std::string> DefaultSplitter::do_split(
		const std::string& str, const std::size_t max_len) const
{
	const auto separator = std::string { " " };

	// Split into substrings separated by a single space
	const auto delimited { details::split(str, separator) };

	auto parts = std::vector<std::string> { };

	using std::begin;
	using std::end;
	std::for_each(begin(delimited), end(delimited),
		[&parts,max_len](std::string str)
		{
			// Split every substring that is still too long
			if (str.length() > max_len)
			{
				auto splitted { details::split(str, max_len) };
				parts.insert(end(parts), begin(splitted), end(splitted));
			} else
			{
				parts.push_back(str);
			}
		});

	return parts;
}


std::unique_ptr<StringSplitter> DefaultSplitter::do_clone() const
{
	return std::make_unique<DefaultSplitter>();
}


// StringTableLayout


StringTableLayout::StringTableLayout(std::unique_ptr<StringSplitter> s)
	: flags_ {  /* print title */              true
			  , /* print row labels */         true
			  , /* print col labels */         true
			  , /* print top delims */         false
			  , /* print row header delims */  false
			  , /* print row inner delims */   false
			  , /* print bottom delims */      false
			  , /* print left outer delims */  false
			  , /* print labels delims */      false
			  , /* print col inner delims */   true
			  , /* print right outer delims */ false }
	, delims_ { /* TOP_DELIM */                "="
		      , /* ROW_HEADER_DELIM */         "="
		      , /* ROW_INNER_DELIM */          "-"
		      , /* BOTTOM_DELIM */             "="
		      , /* LEFT_OUTER_DELIM */         "|"
		      , /* COL_LABELS_DELIM */         "|"
		      , /* COL_INNER_DELIM */          " "
		      , /* RIGHT_OUTER_DELIM */        "|" }
	, splitter_ { std::move(s) }
{
	// empty
}


StringTableLayout::StringTableLayout()
	: StringTableLayout { std::make_unique<DefaultSplitter>() }
{
	// empty
}


StringTableLayout::StringTableLayout(const StringTableLayout& rhs)
	: flags_    { rhs.flags_ }
	, delims_   { rhs.delims_ }
	, splitter_ { std::move(rhs.splitter_->clone()) }
{
	// empty
}


StringTableLayout& StringTableLayout::operator=(const StringTableLayout& rhs)
{
	// TODO Implement reasonable copy assignment for StringTableLayout
	StringTableLayout copy { rhs };
	this->swap(copy);
	return *this;
}


void StringTableLayout::swap(StringTableLayout& rhs) noexcept
{
	using std::swap;
	swap(flags_, rhs.flags_);
	swap(delims_, rhs.delims_);
	swap(splitter_, rhs.splitter_);
}


void StringTableLayout::set_title(const bool f)
{
	return flag_set(Flag::TITLE, f);
}


void StringTableLayout::set_row_labels(const bool f)
{
	return flag_set(Flag::ROW_LABELS, f);
}


void StringTableLayout::set_col_labels(const bool f)
{
	return flag_set(Flag::COL_LABELS, f);
}


void StringTableLayout::set_top_delims(const bool f)
{
	return flag_set(Flag::ROW_TOP_DELIMS, f);
}


void StringTableLayout::set_row_header_delims(const bool f)
{
	return flag_set(Flag::ROW_HEADER_DELIMS, f);
}


void StringTableLayout::set_row_inner_delims(const bool f)
{
	return flag_set(Flag::ROW_INNER_DELIMS, f);
}


void StringTableLayout::set_bottom_delims(const bool f)
{
	return flag_set(Flag::ROW_BOTTOM_DELIMS, f);
}


void StringTableLayout::set_left_outer_delims(const bool f)
{
	return flag_set(Flag::COL_LEFT_OUTER_DELIMS, f);
}


void StringTableLayout::set_col_labels_delims(const bool f)
{
	return flag_set(Flag::COL_LABELS_DELIMS, f);
}


void StringTableLayout::set_col_inner_delims(const bool f)
{
	return flag_set(Flag::COL_INNER_DELIMS, f);
}


void StringTableLayout::set_right_outer_delims(const bool f)
{
	return flag_set(Flag::COL_RIGHT_OUTER_DELIMS, f);
}


bool StringTableLayout::title() const
{
	return flag_get(Flag::TITLE);
}


bool StringTableLayout::row_labels() const
{
	return flag_get(Flag::ROW_LABELS);
}


bool StringTableLayout::col_labels() const
{
	return flag_get(Flag::COL_LABELS);
}


bool StringTableLayout::top_delims() const
{
	return flag_get(Flag::ROW_TOP_DELIMS);
}


bool StringTableLayout::row_header_delims() const
{
	return flag_get(Flag::ROW_HEADER_DELIMS);
}


bool StringTableLayout::row_inner_delims() const
{
	return flag_get(Flag::ROW_INNER_DELIMS);
}


bool StringTableLayout::bottom_delims() const
{
	return flag_get(Flag::ROW_BOTTOM_DELIMS);
}


bool StringTableLayout::left_outer_delims() const
{
	return flag_get(Flag::COL_LEFT_OUTER_DELIMS);
}


bool StringTableLayout::col_labels_delims() const
{
	return flag_get(Flag::COL_LABELS_DELIMS);
}


bool StringTableLayout::col_inner_delims() const
{
	return flag_get(Flag::COL_INNER_DELIMS);
}


bool StringTableLayout::right_outer_delims() const
{
	return flag_get(Flag::COL_RIGHT_OUTER_DELIMS);
}


bool StringTableLayout::flag_get(const Flag f) const
{
	return flags_[static_cast<std::underlying_type_t<Flag>>(f)];
}


void StringTableLayout::flag_set(const Flag f, const bool value)
{
	flags_[static_cast<std::underlying_type_t<Flag>>(f)] = value;
}


void StringTableLayout::set_top_delim(const std::string& d)
{
	delim_set(Index::ROW_TOP_DELIM, d);
}


void StringTableLayout::set_row_header_delim(const std::string& d)
{
	delim_set(Index::ROW_HEADER_DELIM, d);
}


void StringTableLayout::set_row_inner_delim(const std::string& d)
{
	delim_set(Index::ROW_INNER_DELIM, d);
}


void StringTableLayout::set_bottom_delim(const std::string& d)
{
	delim_set(Index::ROW_BOTTOM_DELIM, d);
}


void StringTableLayout::set_left_outer_delim(const std::string& d)
{
	delim_set(Index::COL_LEFT_OUTER_DELIM, d);
}


void StringTableLayout::set_col_labels_delim(const std::string& d)
{
	delim_set(Index::COL_LABELS_DELIM, d);
}


void StringTableLayout::set_col_inner_delim(const std::string& d)
{
	delim_set(Index::COL_INNER_DELIM, d);
}


void StringTableLayout::set_right_outer_delim(const std::string& d)
{
	delim_set(Index::COL_RIGHT_OUTER_DELIM, d);
}


std::string StringTableLayout::top_delim() const
{
	return delim_get(Index::ROW_TOP_DELIM);
}


std::string StringTableLayout::row_header_delim() const
{
	return delim_get(Index::ROW_HEADER_DELIM);
}


std::string StringTableLayout::row_inner_delim() const
{
	return delim_get(Index::ROW_INNER_DELIM);
}


std::string StringTableLayout::bottom_delim() const
{
	return delim_get(Index::ROW_BOTTOM_DELIM);
}


std::string StringTableLayout::left_outer_delim() const
{
	return delim_get(Index::COL_LEFT_OUTER_DELIM);
}


std::string StringTableLayout::col_labels_delim() const
{
	return delim_get(Index::COL_LABELS_DELIM);
}


std::string StringTableLayout::col_inner_delim() const
{
	return delim_get(Index::COL_INNER_DELIM);
}


std::string StringTableLayout::right_outer_delim() const
{
	return delim_get(Index::COL_RIGHT_OUTER_DELIM);
}


void StringTableLayout::delim_set(const Index i, const std::string& value)
{
	delims_[static_cast<std::underlying_type_t<Flag>>(i)] = value;
}


std::string StringTableLayout::delim_get(const Index i) const
{
	return delims_[static_cast<std::underlying_type_t<Flag>>(i)];

}


std::vector<std::string> StringTableLayout::split(const std::string& str,
			const std::size_t max_len) const
{
	return splitter()->split(str, max_len);
}


void StringTableLayout::set_splitter(std::unique_ptr<StringSplitter> s)
{
	splitter_ = std::move(s);
}


const StringSplitter* StringTableLayout::splitter() const
{
	return splitter_.get();
}


// StringTablePrinter::Impl


class StringTablePrinter::Impl
{
public:

	void print(std::ostream &o, const StringTable& t) const;

protected:

	// Those could be virtual since they directly use o <<

	void title(std::ostream &o, const StringTable& t,
		const StringTableLayout& l) const;
	void row_label(std::ostream &o, const StringTable& t, const int row,
		const std::size_t width) const;
	void col_label(std::ostream &o, const StringTable& t, const int col,
		const std::size_t width) const;
	void cell(std::ostream &o, const StringTable& t, const int row,
		const int col, const std::size_t width) const;

	// All that follows seems to be non-virtual

	void col_labels(std::ostream &o, const StringTable& t,
		std::vector<std::size_t> col_widths, const StringTableLayout& l) const;
	void row(std::ostream &o, const StringTable& t, const int row,
		std::vector<std::size_t> col_widths, const StringTableLayout& l) const;
	void rows(std::ostream &o, const StringTable& t,
		std::vector<std::size_t> col_widths, const StringTableLayout& l) const;

	void top_delim(std::ostream &o, const StringTable& t,
		const StringTableLayout& l) const;
	void left_outer_col_delim(std::ostream &o, const StringTable& t,
		const StringTableLayout& l) const;
	void labels_col_delim(std::ostream &o, const StringTable& t,
		const StringTableLayout& l) const;
	void inner_col_delim(std::ostream &o, const StringTable& t,
		const StringTableLayout& l) const;
	void right_outer_col_delim(std::ostream &o, const StringTable& t,
		const StringTableLayout& l) const;
	void bottom_delim(std::ostream &o, const StringTable& t,
		const StringTableLayout& l) const;

	void row_delim(std::ostream &o, const StringTable& t,
		const StringTableLayout& l, const std::size_t width) const;

	void row_delimiters(std::ostream &o, const StringTable& t,
		std::vector<std::size_t> col_widths, const StringTableLayout& l) const;

	std::vector<std::size_t> printed_widths(const StringTable& t,
		const StringTableLayout& l) const;
	std::size_t optimal_row_label_width(const StringTable& t) const;

	using print_label_func = std::function<void(std::ostream &o,
			const StringTable& t, const int row, const std::size_t width)>;

	using print_cell_func = std::function<void(std::ostream &o,
			const StringTable& t, const int row, const int col,
			const std::size_t width)>;

	void row_worker(std::ostream &o, const StringTable& t,
		const int row, std::vector<std::size_t> col_widths,
		const StringTableLayout& l, const print_label_func& label_f,
		const print_cell_func& cell_f) const;
	void row_label_worker(std::ostream &o, const StringTable& t, const int row,
		const print_label_func& f) const;
	void row_cells_worker(std::ostream &o, const StringTable& t, const int row,
		std::vector<std::size_t> col_widths, const StringTableLayout& l,
		const print_cell_func& f) const;

	void empty_cell(std::ostream &o, const std::size_t width) const;
	void line_n(std::ostream &o, const std::size_t width,
		const std::string& text) const;
};


void StringTablePrinter::Impl::title(std::ostream &o, const StringTable &t,
		const StringTableLayout& /* l */) const
{
	if (!t.title().empty())
	{
		o << t.title() << '\n';
	}
}


void StringTablePrinter::Impl::row_label(std::ostream &o, const StringTable& t,
		const int row, const std::size_t width) const
{
	o << std::setw(width) << std::left << t.row_label(row) << std::setfill(' ');
	// Row labels are always aligned LEFT
}


void StringTablePrinter::Impl::col_label(std::ostream &o, const StringTable& t,
		const int col, const std::size_t width) const
{
	o << std::setw(width) << std::left << t.col_label(col) << std::setfill(' ');
	// Col labels are always aligned LEFT
}


void StringTablePrinter::Impl::cell(std::ostream &o, const StringTable& t,
		const int row, const int col, const std::size_t col_width) const
{
	o << std::setw(col_width);

	switch (t.align(col))
	{
		case Align::LEFT:
		{
			o << std::left << t(row, col);
			break;
		}
		case Align::RIGHT:
		{
			o << std::right << t(row, col);
			break;
		}
		case Align::BLOCK:
		{
			o << t(row, col);
			break;
		}
	};
}


void StringTablePrinter::Impl::col_labels(std::ostream &o, const StringTable& t,
		std::vector<std::size_t> col_widths, const StringTableLayout& l) const
{
	// Wrapper for row_label()
	const auto row_label_f = [&](std::ostream &o, const StringTable& t,
		const int /*ignore*/, std::size_t width)
	{
		// Leftmost visual column is the column with the row labels,
		// just fill it
		if (width > 0)
		{
			empty_cell(o, width);
		}
	};

	// Wrapper for col_label()
	const auto col_label_f = [&](std::ostream &o, const StringTable& t,
		const int /* ignore */, const int col, std::size_t width)
	{
		col_label(o, t, col, width);
	};

	row_worker(o, t, -1/*ignored*/, col_widths, l, row_label_f, col_label_f);
}


void StringTablePrinter::Impl::row(std::ostream &o, const StringTable& t,
		const int row, std::vector<std::size_t> col_widths,
		const StringTableLayout& l) const
{
	// Wrapper for row_label()
	const auto row_label_f = std::bind(&StringTablePrinter::Impl::row_label,
		this,
		std::placeholders::_1, std::placeholders::_2,
		std::placeholders::_3, std::placeholders::_4);

	// Wrapper for cell()
	const auto cell_f = std::bind(&StringTablePrinter::Impl::cell, this,
		std::placeholders::_1, std::placeholders::_2,
		std::placeholders::_3, std::placeholders::_4,
		std::placeholders::_5);

	row_worker(o, t, row, col_widths, l, row_label_f, cell_f);
}


void StringTablePrinter::Impl::rows(std::ostream &o, const StringTable& t,
		std::vector<std::size_t> col_widths, const StringTableLayout& l) const
{
	// Table rows
	for (auto r = 0; r < t.rows(); ++r)
	{
		row(o, t, r, col_widths, l);

		// TODO row_delimiters();
	}
}


void StringTablePrinter::Impl::row_worker(std::ostream &o, const StringTable& t,
		const int row, std::vector<std::size_t> col_widths,
		const StringTableLayout& l, const print_label_func& row_label_f,
		const print_cell_func& cell_f) const
{
	if (l.left_outer_delims())
	{
		left_outer_col_delim(o, t, l);
	}

	if (l.row_labels())
	{
		row_label_worker(o, t, row, row_label_f);
		if (l.col_labels_delims())
		{
			labels_col_delim(o, t, l);
		}
	}

	row_cells_worker(o, t, row, col_widths, l, cell_f);

	if (l.right_outer_delims())
	{
		right_outer_col_delim(o, t, l);
	} else
	{
		o << '\n';
	}
}


void StringTablePrinter::Impl::row_label_worker(std::ostream &o,
		const StringTable& t, const int row, const print_label_func& f) const
{
	f(o, t, row, optimal_row_label_width(t));
}


void StringTablePrinter::Impl::row_cells_worker(std::ostream &o,
		const StringTable& t, const int row,
		std::vector<std::size_t> col_widths, const StringTableLayout& l,
		const print_cell_func& cell_f) const
{
	const auto rightmost_col = std::max(0, t.cols() - 1);

	// Collect the splitted fields of columns
	// XXX Since we do access by index, wouldn't a vector be good enough?
	std::map<std::size_t, std::vector<std::string>> fields;

	auto field { fields.begin() };
	auto lines { 0 };
	auto line  { 0 };

	auto width = std::size_t { 0 };

	// Print multiline row
	do
	{
		// Print every col in row
		for (auto c = std::size_t { 0 }; c < t.cols(); ++c)
		{
			const auto& cell_text = (row >= 0) ? t(row, c) : t.col_label(c);

			// Call the actual cell printing function
			if (Align::BLOCK == t.align(c))
			{
				width = t.max_width(c); // col_widths[c] would be optimal width
										//
				if (cell_text.length() <= width)
				{
					// Cell does not need splitting.
					// Every line but the first are empty
					if (line == 0)
					{
						cell_f(o, t, row, c, width);
					} else
					{
						empty_cell(o, width);
					}
				} else // Cell must be splitted
				{
					// Get multiline text field in actual column
					field = fields.find(c);

					if (field != end(fields)) // Is follow-up line
					{
						// Does line exist in this column?
						if (line < field->second.size())
						{
							line_n(o, width, field->second[line]);
						} else
						{
							empty_cell(o, width);
						}

					} else // First line of text field
					{
						line = 0;
						const auto parts { l.split(cell_text, width) };

						if (parts.size() > lines)
						{
							lines = parts.size();
						}
						line_n(o, width, parts[0]);

						fields.insert(std::make_pair(c, parts));
					}
				}
			} else
			{
				width = col_widths[c];

				if (line == 0)
				{
					cell_f(o, t, row, c, width);
				} else
				{
					empty_cell(o, width);
				}
			}

			// Trigger delimiter for inner columns
			if (c < rightmost_col)
			{
				if (l.col_inner_delims())
				{
					inner_col_delim(o, t, l);
				}
			} else
			{
				if (line < lines)
				{
					// Trigger newline for each line of the row except the last.
					// (i.e. do not switch to a new line *after* finishing the
					// row, since this is done by right_outer_col_delim!)
					if (1 < lines - line)
					{
						o << '\n';
					}
					++line;
				}
			}
		} // for

	} while (line < lines);
}


void StringTablePrinter::Impl::empty_cell(std::ostream &o,
		const std::size_t width) const
{
	o << std::setw(width) << ' ' << std::setfill(' ');
}


void StringTablePrinter::Impl::line_n(std::ostream &o, const std::size_t width,
		const std::string& text) const
{
	o << std::setw(width) << std::left << text << std::setfill(' ');
}


void StringTablePrinter::Impl::top_delim(std::ostream &o, const StringTable& t,
		const StringTableLayout& l) const
{
	o << l.top_delim();
}


void StringTablePrinter::Impl::left_outer_col_delim(std::ostream &o,
		const StringTable& t, const StringTableLayout& l) const
{
	o << l.left_outer_delim();
}


void StringTablePrinter::Impl::labels_col_delim(std::ostream &o,
		const StringTable& t, const StringTableLayout& l) const
{
	o << l.col_labels_delim();
}


void StringTablePrinter::Impl::inner_col_delim(std::ostream &o,
		const StringTable& t, const StringTableLayout& l) const
{
	o << l.col_inner_delim();
}


void StringTablePrinter::Impl::right_outer_col_delim(std::ostream &o,
		const StringTable& t, const StringTableLayout& l) const
{
	o << l.right_outer_delim() << '\n';
}


void StringTablePrinter::Impl::bottom_delim(std::ostream &o,
		const StringTable& t, const StringTableLayout& l) const
{
	o << l.bottom_delim();
}


void StringTablePrinter::Impl::row_delim(std::ostream &o, const StringTable& t,
		const StringTableLayout& l, const std::size_t width) const
{
	const auto n { width / l.row_header_delim().length() };
	// FIXME This won't work for any width that is not a multiple of the
	// delimiter length

	for (int i = 0; i < n; ++i)
	{
		o << l.row_header_delim();
	}

	// TODO Only if header row delim requested
}


void StringTablePrinter::Impl::row_delimiters(std::ostream &o,
		const StringTable& t, std::vector<std::size_t> col_widths,
		const StringTableLayout& l) const
{
	// Header row for row labels column
	if (l.left_outer_delims())
	{
		left_outer_col_delim(o, t, l);
	}

	if (l.row_labels())
	{
		this->row_delim(o, t, l, optimal_row_label_width(t));
	}

	if (l.col_labels_delims())
	{
		labels_col_delim(o, t, l);
	}

	// Header row for table columns
	const auto max_col = t.cols() - 1;
	auto c { 0 };
	for (const auto& w : col_widths)
	{
		this->row_delim(o, t, l, w);

		// Set delimiter between inner table columns
		if (l.col_inner_delims() && c < max_col)
		{
			inner_col_delim(o, t, l);
		}
		++c;
	}

	if (l.right_outer_delims())
	{
		right_outer_col_delim(o, t, l);
	} else
	{
		o << '\n';
	}
}


std::vector<std::size_t> StringTablePrinter::Impl::printed_widths(
		const StringTable& t, const StringTableLayout& l) const
{
	std::vector<std::size_t> printed_widths;
	printed_widths.reserve(t.cols());

	auto width = std::size_t { 0 };

	// Collect the real widths for print
	for (auto c = 0; c < t.cols(); ++c)
	{
		width = l.col_labels()
			? std::max(t.optimal_width(c), t.col_label(c).length())
			: t.optimal_width(c);

		if (Align::BLOCK == t.align(c))
		{
			printed_widths.push_back(std::min(width, t.max_width(c)));
		} else
		{
			printed_widths.push_back(width);
		}
	}

	return printed_widths;
}


std::size_t StringTablePrinter::Impl::optimal_row_label_width(
		const StringTable& t) const
{
	if (!t.rows()) { return 0; }

	auto optimal_width { t.row_label(0).length() };
	auto curr_width { optimal_width };

	for (auto r = 1; r < t.rows(); ++r)
	{
		curr_width = t.row_label(r).length();
		if (curr_width > optimal_width)
		{
			optimal_width = curr_width;
		}
	}

	return optimal_width;
}


void StringTablePrinter::Impl::print(std::ostream &o, const StringTable& t)
	const
{
	const auto prev_settings { o.flags() };
	const auto l { t.layout() };

	// Print title
	if (l->title())
	{
		title(o, t, *l);
	}

	// Column widths (w/o row labels)
	const auto col_widths { printed_widths(t, *l) };

	// Print top delims
	if (l->top_delims())
	{
		row_delimiters(o, t, col_widths, *l);
	}

	// Print column labels
	if (l->col_labels())
	{
		col_labels(o, t, col_widths, *l);
		// FIXME + cols() * 1 delim

		// Header row delims
		if (l->row_header_delims() && t.rows() > 0)
		{
			row_delimiters(o, t, col_widths, *l);
		}
	}

	// Print rows
	rows(o, t, col_widths, *l);

	// Print bottom delims
	if (l->bottom_delims())
	{
		row_delimiters(o, t, col_widths, *l);
	}

	o.flags(prev_settings);
}


// StringTablePrinter


StringTablePrinter::StringTablePrinter()
	: impl_ { std::make_unique<Impl>() }
{
	// empty
}


StringTablePrinter::~StringTablePrinter() noexcept = default;


void StringTablePrinter::print(std::ostream &o, const StringTable& t) const
{
	impl_->print(o, t);
}


// non-member functions


std::ostream& operator << (std::ostream &o, const StringTable &table)
{
	if (!table.empty())
	{
		StringTablePrinter p;
		p.print(o, table);
	}

	return o;
}

} // namespace table
} // namespace arcsapp

