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
inline namespace v_1_0_0
{
namespace details
{


std::string trim(std::string s)
{
	static const auto whitespace = [](int c) { return !std::isspace(c); };

	using std::begin;
	using std::end;

	// left
	s.erase(begin(s), std::find_if(begin(s), end(s), whitespace));

	using std::rbegin;
	using std::rend;

	// right
	s.erase(std::find_if(rbegin(s), rend(s), whitespace).base(), end(s));

	return s;
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

	for (auto i = std::size_t { 0 }; i < str.length(); i += max_len)
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
 *
 * If the specified vector \c v is empty, it will be resized to new size
 * \c rows. If \c v contains elements, \c rows new elements will be inserted
 * after index \c row.
 *
 * \tparam T Vector value_type to use
 *
 * \param[in] v				Vector to modify
 * \param[in] elements      Number of elements to insert or to resize to
 * \param[in] after_pos     Index to insert elements after
 * \param[in] default_value Default value for new rows/cells
 */
template <typename T>
void insert_or_resize(std::vector<T>& v, const std::size_t elements,
	const int after_pos, const T& default_value)
{
	if (after_pos < 0 || elements == 0)
	{
		return;
	}

	if (v.empty())
	{
		v.resize(elements);
	} else
	{
		using std::begin;
		using std::end;

		const auto exceeds { static_cast<std::size_t>(after_pos) >= v.size() };

		// Step to the nearest: 'row' or to last element
		const auto pos { exceeds ? end(v) : begin(v) += after_pos };

		// Actual amount of rows to insert
		auto amount { elements };
		if (exceeds)
		{
			// Add the amount of elements necessary to fill the vector up to
			// position 'after_pos'.
			amount += static_cast<std::size_t>(after_pos) - v.size() + 1;
		}

		v.insert(pos, amount, default_value);
	}
}

} // details


namespace table
{

// PrintableTable


std::string PrintableTable::title() const
{
	return do_title();
}


const std::string& PrintableTable::ref(int row, int col) const
{
	return do_ref(row, col);
}


std::string PrintableTable::cell(int row, int col) const
{
	return do_cell(row, col);
}


std::size_t PrintableTable::rows() const
{
	return do_rows();
}


std::string PrintableTable::row_label(int row) const
{
	return do_row_label(row);
}


std::size_t PrintableTable::max_height(int row) const
{
	return do_max_height(row);
}


std::size_t PrintableTable::cols() const
{
	return do_cols();
}


std::string PrintableTable::col_label(int col) const
{
	return do_col_label(col);
}


std::size_t PrintableTable::max_width(int col) const
{
	return do_max_width(col);
}


Align PrintableTable::align(int col) const
{
	return do_align(col);
}


std::size_t PrintableTable::optimal_width(const int col) const
{
	return do_optimal_width(col);
}


bool PrintableTable::empty() const
{
	return do_empty();
}


const StringTableLayout* PrintableTable::layout() const
{
	return do_layout();
}


// StringTable


StringTable::StringTable(const std::string& title, const std::size_t rows,
		const std::size_t cols)
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


StringTable::StringTable(const std::size_t rows, const std::size_t cols)
	: StringTable(std::string{/*empty title*/}, rows, cols)
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


void StringTable::set_title(const std::string& title)
{
	title_ = title;
}


std::string StringTable::operator()(int row, int col) const
{
	return cells_[safe_index(row, col)];
}


std::string& StringTable::operator()(int row, int col)
{
	return cells_[safe_index(row, col)];
}


std::string& StringTable::cell(int row, int col)
{
	const auto r { static_cast<std::size_t>(row < 0 ? 0 : row) };
	if (r >= this->rows())
	{
		this->append_rows(r - this->rows() + 1u);
	}
	return this->operator()(row, col);
}


void StringTable::set_row_label(int row, const std::string& label)
{
	row_labels_[i(row)] = label;
}


void StringTable::set_max_height(int row, std::size_t height)
{
	row_max_heights_[i(row)] = height;
}


std::size_t StringTable::default_max_height() const
{
	return default_max_height_;
}


void StringTable::set_col_label(int col, const std::string& label)
{
	col_labels_[i(col)] = label;
}


void StringTable::set_max_width(int col, std::size_t width)
{
	col_max_widths_[i(col)] = width;
}


std::size_t StringTable::default_max_width() const
{
	return default_max_width_;
}


void StringTable::set_align(int col, Align align)
{
	aligns_[i(col)] = align;
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


void StringTable::insert_rows_after(const std::size_t rows, const int row)
{
	if (!rows) // nothing to insert
	{
		return;
	}

	if (!this->cols()) // no table
	{
		throw std::runtime_error(
				"Cannot insert row in table with zero columns");
	}

	using details::insert_or_resize;

	// Adjust table cell store

	insert_or_resize(cells_, rows * this->cols(), /* amount of cells */
			index(row, this->cols()), /* position after specified row */
			std::string{});

	rows_ += rows;

	// Adjust internal stores for labels + heights

	insert_or_resize(row_labels_,      rows, row, std::string{});
	insert_or_resize(row_max_heights_, rows, row, default_max_height());
}


void StringTable::append_rows(const std::size_t rows)
{
	insert_rows_after(rows, this->rows() > 0 ? this->rows() - 1 : 0);
}


void StringTable::set_layout(std::unique_ptr<StringTableLayout> l)
{
	layout_ = std::move(l);
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
	return static_cast<index_type>(row) * this->cols()
		+ static_cast<index_type>(col);
}


StringTable::index_type StringTable::i(const int i) const
{
	return static_cast<index_type>(i);
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


std::string StringTable::do_title() const
{
	return title_;
}


const std::string& StringTable::do_ref(int row, int col) const
{
	return cells_[safe_index(row, col)];
}


std::string StringTable::do_cell(int row, int col) const
{
	return this->operator()(row, col);
}


std::size_t StringTable::do_rows() const
{
	return rows_;
}


std::string StringTable::do_row_label(int row) const
{
	return row_labels_[i(row)];
}


std::size_t StringTable::do_max_height(int row) const
{
	return row_max_heights_[i(row)];
}


std::size_t StringTable::do_cols() const
{
	return cols_;
}


std::string StringTable::do_col_label(int col) const
{
	return col_labels_[i(col)];
}


std::size_t StringTable::do_max_width(int col) const
{
	return col_max_widths_[i(col)];
}


Align StringTable::do_align(int col) const
{
	return aligns_[i(col)];
}


std::size_t StringTable::do_optimal_width(const int col) const
{
	auto width = std::size_t { 0 };
	auto optimal_width { width };

	for(auto row = std::size_t { 0 }; row < rows(); ++row)
	{
		width = cell(row, col).length();
		if (width > optimal_width) { optimal_width = width; }
	}

	return optimal_width;
}


bool StringTable::do_empty() const
{
	return cells_.empty();
}


const StringTableLayout* StringTable::do_layout() const
{
	return layout_.get();
}


bool operator== (const StringTable& lhs, const StringTable& rhs)
{
	return lhs.cells_ == rhs.cells_
		&& lhs.title_ == rhs.title_
		&& lhs.rows_  == rhs.rows_
		&& lhs.default_max_height_ == rhs.default_max_height_
		&& lhs.cols_  == rhs.cols_
		&& lhs.default_max_width_ == rhs.default_max_width_
		&& lhs.row_labels_ == rhs.row_labels_
		&& lhs.row_max_heights_ == rhs.row_max_heights_
		&& lhs.col_labels_ == rhs.col_labels_
		&& lhs.col_max_widths_ == rhs.col_max_widths_
		&& lhs.aligns_ == rhs.aligns_;
	// Ignore Layout when testing equality
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

	auto parts = std::vector<std::string>{};

	using std::begin;
	using std::end;

	std::for_each(begin(delimited), end(delimited),
		[&parts, max_len](std::string s)
		{
			// Split every substring that is still too long
			if (s.length() > max_len)
			{
				auto splitted { details::split(s, max_len) };
				parts.insert(end(parts), begin(splitted), end(splitted));
			} else
			{
				parts.push_back(s);
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
	, splitter_  { std::move(s) }
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
	, splitter_ { rhs.splitter_->clone() }
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
	flag_set(Flag::TITLE, f);
}


void StringTableLayout::set_row_labels(const bool f)
{
	flag_set(Flag::ROW_LABELS, f);
}


void StringTableLayout::set_col_labels(const bool f)
{
	flag_set(Flag::COL_LABELS, f);
}


void StringTableLayout::set_top_delims(const bool f)
{
	flag_set(Flag::ROW_TOP_DELIMS, f);
}


void StringTableLayout::set_row_header_delims(const bool f)
{
	flag_set(Flag::ROW_HEADER_DELIMS, f);
}


void StringTableLayout::set_row_inner_delims(const bool f)
{
	flag_set(Flag::ROW_INNER_DELIMS, f);
}


void StringTableLayout::set_bottom_delims(const bool f)
{
	flag_set(Flag::ROW_BOTTOM_DELIMS, f);
}


void StringTableLayout::set_left_outer_delims(const bool f)
{
	flag_set(Flag::COL_LEFT_OUTER_DELIMS, f);
}


void StringTableLayout::set_col_labels_delims(const bool f)
{
	flag_set(Flag::COL_LABELS_DELIMS, f);
}


void StringTableLayout::set_col_inner_delims(const bool f)
{
	flag_set(Flag::COL_INNER_DELIMS, f);
}


void StringTableLayout::set_right_outer_delims(const bool f)
{
	flag_set(Flag::COL_RIGHT_OUTER_DELIMS, f);
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
	delim_set(Delimiter::ROW_TOP_DELIM, d);
}


void StringTableLayout::set_row_header_delim(const std::string& d)
{
	delim_set(Delimiter::ROW_HEADER_DELIM, d);
}


void StringTableLayout::set_row_inner_delim(const std::string& d)
{
	delim_set(Delimiter::ROW_INNER_DELIM, d);
}


void StringTableLayout::set_bottom_delim(const std::string& d)
{
	delim_set(Delimiter::ROW_BOTTOM_DELIM, d);
}


void StringTableLayout::set_left_outer_delim(const std::string& d)
{
	delim_set(Delimiter::COL_LEFT_OUTER_DELIM, d);
}


void StringTableLayout::set_col_labels_delim(const std::string& d)
{
	delim_set(Delimiter::COL_LABELS_DELIM, d);
}


void StringTableLayout::set_col_inner_delim(const std::string& d)
{
	delim_set(Delimiter::COL_INNER_DELIM, d);
}


void StringTableLayout::set_right_outer_delim(const std::string& d)
{
	delim_set(Delimiter::COL_RIGHT_OUTER_DELIM, d);
}


std::string StringTableLayout::top_delim() const
{
	return delim_get(Delimiter::ROW_TOP_DELIM);
}


std::string StringTableLayout::row_header_delim() const
{
	return delim_get(Delimiter::ROW_HEADER_DELIM);
}


std::string StringTableLayout::row_inner_delim() const
{
	return delim_get(Delimiter::ROW_INNER_DELIM);
}


std::string StringTableLayout::bottom_delim() const
{
	return delim_get(Delimiter::ROW_BOTTOM_DELIM);
}


std::string StringTableLayout::left_outer_delim() const
{
	return delim_get(Delimiter::COL_LEFT_OUTER_DELIM);
}


std::string StringTableLayout::col_labels_delim() const
{
	return delim_get(Delimiter::COL_LABELS_DELIM);
}


std::string StringTableLayout::col_inner_delim() const
{
	return delim_get(Delimiter::COL_INNER_DELIM);
}


std::string StringTableLayout::right_outer_delim() const
{
	return delim_get(Delimiter::COL_RIGHT_OUTER_DELIM);
}


void StringTableLayout::delim_set(const Delimiter i, const std::string& value)
{
	delims_[static_cast<std::underlying_type_t<Flag>>(i)] = value;
}


std::string StringTableLayout::delim_get(const Delimiter i) const
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


// CellDecorator


CellDecorator::CellDecorator(const std::size_t n)
	: flags_(n) // initializes all to FALSE
{
	// empty
}


CellDecorator::CellDecorator(const CellDecorator& rhs)
	: flags_(rhs.flags_)
{
	// empty
}


CellDecorator::index_type CellDecorator::i(const int index) const
{
	return static_cast<index_type>(index);
}


void CellDecorator::set(const int idx)
{
	flags_[i(idx)] = true;
}


void CellDecorator::unset(const int idx)
{
	flags_[i(idx)] = false;
}


bool CellDecorator::is_set(const int idx) const
{
	return flags_[i(idx)];
}


std::string CellDecorator::decorate(const int i, std::string&& s) const
{
	return is_set(i)
		? do_decorate_set(std::move(s))
		: do_decorate_unset(std::move(s));
}


std::unique_ptr<CellDecorator> CellDecorator::clone() const
{
	return do_clone();
}

} // namespace table


namespace details
{

DecoratorRegistry::DecoratorRegistry()
	: internal_registry_ { /*empty*/ }
{
	// empty
}

void DecoratorRegistry::register_to_col(const int j,
		std::unique_ptr<CellDecorator> d)
{
	this->add_decorator(std::move(d), col_idx(j));
}


const CellDecorator* DecoratorRegistry::col_decorator(const int j) const
{
	return this->get_decorator(col_idx(j));
}


void DecoratorRegistry::register_to_row(const int i,
		std::unique_ptr<CellDecorator> d)
{
	this->add_decorator(std::move(d), row_idx(i));
}


const CellDecorator* DecoratorRegistry::row_decorator(const int i) const
{
	return this->get_decorator(row_idx(i));
}


void DecoratorRegistry::mark_decorated(const int i, const int j)
{
	this->set_flag(i, j, true);
}


void DecoratorRegistry::unmark_decorated(const int i, const int j)
{
	this->set_flag(i, j, false);
}


bool DecoratorRegistry::is_decorated(const int i, const int j)
{
	if (is_decorated_worker(row_decorator(i), j))
	{
		return true;
	}

	return is_decorated_worker(col_decorator(j), i);
}


void DecoratorRegistry::add_decorator(std::unique_ptr<CellDecorator> d,
		const int idx)
{
	internal_registry_[idx] = std::move(d);
}


CellDecorator* DecoratorRegistry::get_decorator(const int idx) const
{
	using std::end;
	auto d { internal_registry_.find(idx) };
	return (end(internal_registry_) != d) ? d->second.get() : nullptr;
}


int DecoratorRegistry::row_idx(const int i) const
{
	return i < 0 ? i : (-1) * i; // Row indices are always < 0
}


int DecoratorRegistry::col_idx(const int j) const
{
	return j < 0 ? (-1) * j : j; // Col indices are always > 0
}


void DecoratorRegistry::set_flag(const int i, const int j, const bool flag)
{
	auto dec { get_decorator(col_idx(j)) }; // decorator for column j
	this->set_flag_worker(dec, i, flag);

	dec = get_decorator(row_idx(i)); // decorator for row i
	this->set_flag_worker(dec, j, flag);
}


void DecoratorRegistry::set_flag_worker(CellDecorator* d, const int n,
		const bool flag)
{
	if (d)
	{
		flag ? d->set(n) : d->unset(n);
	}
}


bool DecoratorRegistry::is_decorated_worker(const CellDecorator* d, const int n)
{
	return d && d->is_set(n);
}


bool operator == (const DecoratorRegistry& lhs, const DecoratorRegistry& rhs)
	noexcept
{
	return lhs.internal_registry_ == rhs.internal_registry_;
}

} // namespace details


namespace table
{

// DecoratedStringTable


DecoratedStringTable::DecoratedStringTable(const std::string& title,
		const std::size_t rows, const std::size_t cols)
	: table_    { std::make_unique<StringTable>(title, rows, cols) }
	, registry_ { std::make_unique<details::DecoratorRegistry>() }
{
	// empty
}


DecoratedStringTable::DecoratedStringTable(const std::size_t rows,
		const std::size_t cols)
	: DecoratedStringTable(std::string{/*empty title*/}, rows, cols)
{
	// empty
}


std::string& DecoratedStringTable::operator() (int row, int col)
{
	return table_->operator()(row, col);
}


void DecoratedStringTable::register_to_row(const int i,
		std::unique_ptr<CellDecorator> d)
{
	registry_->register_to_row(i, std::move(d));
}


const CellDecorator* DecoratedStringTable::row_decorator(const int i) const
{
	return registry_->row_decorator(i);
}


void DecoratedStringTable::register_to_col(const int j,
		std::unique_ptr<CellDecorator> d)
{
	registry_->register_to_col(j, std::move(d));
}


const CellDecorator* DecoratedStringTable::col_decorator(const int j) const
{
	return registry_->col_decorator(j);
}


void DecoratedStringTable::mark_decorated(const int i, const int j)
{
	registry_->mark_decorated(i, j);
}


void DecoratedStringTable::unmark_decorated(const int i, const int j)
{
	registry_->unmark_decorated(i, j);
}


std::unique_ptr<PrintableTable> DecoratedStringTable::remove_inner_table()
{
	std::unique_ptr<PrintableTable> table { std::move(table_) };
	return table;
}


const StringTable* DecoratedStringTable::table() const
{
	return table_.get();
}


void DecoratedStringTable::set_layout(std::unique_ptr<StringTableLayout> l)
{
	table_->set_layout(std::move(l));
}


void DecoratedStringTable::set_col_label(int col, const std::string& label)
{
	table_->set_col_label(col, label);
}


void DecoratedStringTable::set_row_label(int row, const std::string& label)
{
	table_->set_row_label(row, label);
}


void DecoratedStringTable::set_align(int col, table::Align align)
{
	table_->set_align(col, align);
}


std::string DecoratedStringTable::do_title() const
{
	return table()->title();
}


const std::string& DecoratedStringTable::do_ref(int row, int col) const
{
	return table()->ref(row, col);
}


std::string DecoratedStringTable::do_cell(int row, int col) const
{
	if (auto cold { col_decorator(col) }; cold)
	{
		if (auto rowd { row_decorator(row) }; rowd)
		{
			return rowd->decorate(row,
					cold->decorate(row, table()->cell(row, col)));
		}

		return cold->decorate(row, table()->cell(row, col));
	}

	return table()->cell(row, col);
}


std::size_t DecoratedStringTable::do_rows() const
{
	return table()->rows();
}


std::string DecoratedStringTable::do_row_label(int row) const
{
	return table()->row_label(row);
}


std::size_t DecoratedStringTable::do_max_height(int row) const
{
	return table()->max_height(row);
}


std::size_t DecoratedStringTable::do_cols() const
{
	return table()->cols();
}


std::string DecoratedStringTable::do_col_label(int col) const
{
	return table()->col_label(col);
}


std::size_t DecoratedStringTable::do_max_width(int col) const
{
	return table()->max_width(col);
}


table::Align DecoratedStringTable::do_align(int col) const
{
	return table()->align(col);
}


std::size_t DecoratedStringTable::do_optimal_width(const int col) const
{
	return table()->optimal_width(col);
}


bool DecoratedStringTable::do_empty() const
{
	return table()->empty();
}


const StringTableLayout* DecoratedStringTable::do_layout() const
{
	return table()->layout();
}


bool operator==(const DecoratedStringTable& lhs,
		const DecoratedStringTable& rhs)
{
	return lhs.table_ == rhs.table_ && lhs.registry_ == rhs.registry_;
}


// TablePrinter::Impl


class TablePrinter::Impl final
{
public:

	/**
	 * \brief Print table.
	 *
	 * \param[in] o Stream to print table to
	 * \param[in] t Table to print
	 */
	void print(std::ostream& o, const PrintableTable& t) const;

protected:

	/**
	 * \brief Print title.
	 */
	void title(std::ostream& o, const PrintableTable& t,
		const StringTableLayout& l) const;

	/**
	 * \brief Print single row label.
	 */
	void row_label(std::ostream& o, const PrintableTable& t, const int row,
		const std::size_t width) const;

	/**
	 * \brief Print single column label.
	 */
	void col_label(std::ostream& o, const PrintableTable& t, const int col,
		const std::size_t width) const;

	/**
	 * \brief Print row of all column labels.
	 *
	 * Calls empty_cell(), col_label(), and row_worker().
	 */
	void col_labels(std::ostream& o, const PrintableTable& t,
		const std::vector<std::size_t>& col_widths, const StringTableLayout& l)
		const;

	/**
	 * \brief Print row of cells.
	 *
	 * Calls row_worker().
	 */
	void row(std::ostream& o, const PrintableTable& t, const int row,
		const std::vector<std::size_t>& col_widths, const StringTableLayout& l)
		const;

	/**
	 * \brief Print all rows.
	 *
	 * Calls row().
	 */
	void rows(std::ostream& o, const PrintableTable& t,
		const std::vector<std::size_t>& col_widths, const StringTableLayout& l)
		const;

	using print_label_func = std::function<void(std::ostream& o,
			const PrintableTable& t, const int row, const std::size_t width)>;

	using print_cell_func = std::function<void(std::ostream& o,
			const PrintableTable& t, const int row, const int col,
			const std::size_t width)>;

	/**
	 * \brief Worker to print single row.
	 *
	 * Calls row_label_worker(), row_cells_worker(), row_labels() and some
	 * delimiter printing functions.
	 */
	void row_worker(std::ostream& o, const PrintableTable& t,
		const int row, const std::vector<std::size_t>& col_widths,
		const StringTableLayout& l, const print_label_func& label_f,
		const print_cell_func& cell_f) const;

	/**
	 * \brief Worker to print single row label.
	 */
	void row_label_worker(std::ostream& o, const PrintableTable& t,
		const int row, const print_label_func& f) const;

	/**
	 * \brief Worker to print all cells of a row.
	 */
	void row_cells_worker(std::ostream& o, const PrintableTable& t,
		const int row, const std::vector<std::size_t>& col_widths,
		const StringTableLayout& l, const print_cell_func& f) const;

	/**
	 * \brief Print single cell.
	 */
	void cell(std::ostream& o, const PrintableTable& t, const int row,
		const int col, const std::size_t width) const;

	/**
	 * \brief Print single empty cell.
	 */
	void empty_cell(std::ostream& o, const std::size_t width) const;

	/**
	 * \brief Print subsequent line in multiline cell.
	 */
	void line_n(std::ostream& o, const std::size_t width,
		const std::string& text) const;

	void top_delim(std::ostream& o, const PrintableTable& t,
		const StringTableLayout& l) const;
	void left_outer_col_delim(std::ostream& o, const PrintableTable& t,
		const StringTableLayout& l) const;
	void labels_col_delim(std::ostream& o, const PrintableTable& t,
		const StringTableLayout& l) const;
	void inner_col_delim(std::ostream& o, const PrintableTable& t,
		const StringTableLayout& l) const;
	void right_outer_col_delim(std::ostream& o, const PrintableTable& t,
		const StringTableLayout& l) const;
	void bottom_delim(std::ostream& o, const PrintableTable& t,
		const StringTableLayout& l) const;
	void row_delim(std::ostream& o, const PrintableTable& t,
		const std::size_t width, const std::string& delim) const;
	void row_delimiters(std::ostream& o, const PrintableTable& t,
		const std::vector<std::size_t>& col_widths, const StringTableLayout& l,
		const std::string& delim) const;

	std::vector<std::size_t> printed_widths(const PrintableTable& t,
		const StringTableLayout& l) const;

	std::size_t optimal_row_label_width(const PrintableTable& t) const;
};


void TablePrinter::Impl::title(std::ostream& o, const PrintableTable& t,
		const StringTableLayout& /* l */) const
{
	if (!t.title().empty())
	{
		o << t.title() << '\n';
	}
}


void TablePrinter::Impl::row_label(std::ostream& o, const PrintableTable& t,
		const int row, const std::size_t width) const
{
	o << std::setw(width) << std::left << t.row_label(row) << std::setfill(' ');
	// Row labels are always aligned LEFT
}


void TablePrinter::Impl::col_label(std::ostream& o, const PrintableTable& t,
		const int col, const std::size_t width) const
{
	o << std::setw(width) << std::left << t.col_label(col) << std::setfill(' ');
	// Col labels are always aligned LEFT
}


void TablePrinter::Impl::col_labels(std::ostream& o, const PrintableTable& t,
		const std::vector<std::size_t>& col_widths, const StringTableLayout& l)
		const
{
	// Wrapper for row_label()
	const auto row_label_f = [&](std::ostream& out,
		const PrintableTable& /*ignore*/, const int /*ignore*/,
		std::size_t width)
	{
		// Leftmost visual column is the column with the row labels,
		// just fill it
		if (width > 0)
		{
			empty_cell(out, width);
		}
	};

	// Wrapper for col_label()
	const auto col_label_f = [&](std::ostream& out, const PrintableTable& table,
		const int /*ignore*/, const int col, std::size_t width)
	{
		col_label(out, table, col, width);
	};

	row_worker(o, t, -1/*ignored*/, col_widths, l, row_label_f, col_label_f);
}


void TablePrinter::Impl::row(std::ostream& o, const PrintableTable& t,
		const int row, const std::vector<std::size_t>& col_widths,
		const StringTableLayout& l) const
{
	// Wrapper for row_label()
	const auto row_label_f = std::bind(&TablePrinter::Impl::row_label,
		this,
		std::placeholders::_1, std::placeholders::_2,
		std::placeholders::_3, std::placeholders::_4);

	// Wrapper for cell()
	const auto cell_f = std::bind(&TablePrinter::Impl::cell, this,
		std::placeholders::_1, std::placeholders::_2,
		std::placeholders::_3, std::placeholders::_4,
		std::placeholders::_5);

	row_worker(o, t, row, col_widths, l, row_label_f, cell_f);
}


void TablePrinter::Impl::rows(std::ostream& o, const PrintableTable& t,
		const std::vector<std::size_t>& col_widths,
		const StringTableLayout& l) const
{
	// Table rows
	for (auto r = std::size_t { 0 }; r < t.rows() - 1; ++r)
	{
		row(o, t, r, col_widths, l);

		if (l.row_inner_delims())
		{
			row_delimiters(o, t, col_widths, l, l.row_inner_delim());
		}
	}

	row(o, t, t.rows() - 1, col_widths, l);
}


void TablePrinter::Impl::row_worker(std::ostream& o, const PrintableTable& t,
		const int row, const std::vector<std::size_t>& col_widths,
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


void TablePrinter::Impl::row_label_worker(std::ostream& o,
		const PrintableTable& t, const int row, const print_label_func& f) const
{
	f(o, t, row, optimal_row_label_width(t));
}


void TablePrinter::Impl::row_cells_worker(std::ostream& o,
		const PrintableTable& t, const int row,
		const std::vector<std::size_t>& col_widths, const StringTableLayout& l,
		const print_cell_func& cell_f) const
{
	const auto rightmost_col = t.cols() > 0 ? t.cols() - 1 : 0;

	// Collect the splitted fields of columns
	// XXX Since we do access by index, wouldn't a vector be good enough?
	std::map<std::size_t, std::vector<std::string>> fields;

	auto field { fields.begin() };

	auto lines = std::size_t { 0 };
	auto line  = std::size_t { 0 };
	auto width = std::size_t { 0 };

	// Print multiline row
	do
	{
		// Print every col in row
		for (auto c = std::size_t { 0 }; c < t.cols(); ++c)
		{
			const auto& cell_text = (row >= 0) ? t.ref(row, c) : t.col_label(c);

			// Call the actual cell printing function
			if (Align::BLOCK == t.align(c))
			{
				width = t.max_width(c); // col_widths[c] would be optimal width

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


void TablePrinter::Impl::cell(std::ostream& o, const PrintableTable& t,
		const int row, const int col, const std::size_t col_width) const
{
	o << std::setw(col_width);

	switch (t.align(col))
	{
		case Align::LEFT:
		{
			o << std::left << t.cell(row, col);
			break;
		}
		case Align::RIGHT:
		{
			o << std::right << t.cell(row, col);
			break;
		}
		case Align::BLOCK:
		{
			o << t.cell(row, col);
			break;
		}
		default: /* unused, avoid warning */
		{
			o << t.cell(row, col);
			break;
		}
	};
}


void TablePrinter::Impl::empty_cell(std::ostream& o,
		const std::size_t width) const
{
	o << std::setw(width) << ' ' << std::setfill(' ');
}


void TablePrinter::Impl::line_n(std::ostream& o, const std::size_t width,
		const std::string& text) const
{
	o << std::setw(width) << std::left << text << std::setfill(' ');
}


void TablePrinter::Impl::top_delim(std::ostream& o, const PrintableTable& /*t\*/,
		const StringTableLayout& l) const
{
	o << l.top_delim();
}


void TablePrinter::Impl::left_outer_col_delim(std::ostream& o,
		const PrintableTable& /*t\*/, const StringTableLayout& l) const
{
	o << l.left_outer_delim();
}


void TablePrinter::Impl::labels_col_delim(std::ostream& o,
		const PrintableTable& /*t\*/, const StringTableLayout& l) const
{
	o << l.col_labels_delim();
}


void TablePrinter::Impl::inner_col_delim(std::ostream& o,
		const PrintableTable& /*t\*/, const StringTableLayout& l) const
{
	o << l.col_inner_delim();
}


void TablePrinter::Impl::right_outer_col_delim(std::ostream& o,
		const PrintableTable& /*t\*/, const StringTableLayout& l) const
{
	o << l.right_outer_delim() << '\n';
}


void TablePrinter::Impl::bottom_delim(std::ostream& o,
		const PrintableTable& /*t\*/, const StringTableLayout& l) const
{
	o << l.bottom_delim();
}


void TablePrinter::Impl::row_delim(std::ostream& o,
		const PrintableTable& /*t\*/,
		const std::size_t width,
		const std::string& delim) const
{
	const auto n { width / delim.length() };
	// FIXME This won't work for any width that is not a multiple of the
	// delimiter length

	for (auto i = std::size_t { 0 }; i < n; ++i)
	{
		o << delim;
	}

	// TODO Only if header row delim requested
}


void TablePrinter::Impl::row_delimiters(std::ostream& o,
		const PrintableTable& t, const std::vector<std::size_t>& col_widths,
		const StringTableLayout& l, const std::string& delim) const
{
	// Header row for row labels column
	if (l.left_outer_delims())
	{
		left_outer_col_delim(o, t, l);
	}

	if (l.row_labels())
	{
		this->row_delim(o, t, optimal_row_label_width(t), delim);
	}

	if (l.col_labels_delims())
	{
		labels_col_delim(o, t, l);
	}

	// Header row for table columns
	const auto max_col = t.cols() - 1;
	auto c = std::size_t { 0 };
	for (const auto& w : col_widths)
	{
		this->row_delim(o, t, w, delim);

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


std::vector<std::size_t> TablePrinter::Impl::printed_widths(
		const PrintableTable& t, const StringTableLayout& l) const
{
	std::vector<std::size_t> printed_widths;
	printed_widths.reserve(t.cols());

	auto width = std::size_t { 0 };

	// Collect the real widths for print
	for (auto c = std::size_t { 0 }; c < t.cols(); ++c)
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


std::size_t TablePrinter::Impl::optimal_row_label_width(
		const PrintableTable& t) const
{
	if (!t.rows()) { return 0; }

	auto optimal_width { t.row_label(0).length() };
	auto curr_width { optimal_width };

	for (auto r = std::size_t { 1 }; r < t.rows(); ++r)
	{
		curr_width = t.row_label(r).length();
		if (curr_width > optimal_width)
		{
			optimal_width = curr_width;
		}
	}

	return optimal_width;
}


void TablePrinter::Impl::print(std::ostream& o, const PrintableTable& t)
	const
{
	const auto prev_settings { o.flags() };
	const auto* l { t.layout() };

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
		row_delimiters(o, t, col_widths, *l, l->top_delim());
	}

	// Print column labels
	if (l->col_labels())
	{
		col_labels(o, t, col_widths, *l);
		// FIXME + cols() * 1 delim

		// Header row delims
		if (l->row_header_delims() && t.rows() > 0)
		{
			row_delimiters(o, t, col_widths, *l, l->row_header_delim());
		}
	}

	// Print rows
	rows(o, t, col_widths, *l); // also prints row_inner_delims

	// Print bottom delims
	if (l->bottom_delims())
	{
		row_delimiters(o, t, col_widths, *l, l->bottom_delim());
	}

	o.flags(prev_settings);
}


// TablePrinter


TablePrinter::TablePrinter()
	: impl_ { std::make_unique<Impl>() }
{
	// empty
}


TablePrinter::~TablePrinter() noexcept = default;


void TablePrinter::print(std::ostream& o, const PrintableTable& table) const
{
	impl_->print(o, table);
}


// non-member functions


std::ostream& operator << (std::ostream& o, const PrintableTable& table)
{
	if (!table.empty())
	{
		TablePrinter p;
		p.print(o, table);
	}

	return o;
}


std::ostream& operator << (std::ostream& o,
		const std::unique_ptr<PrintableTable>& table)
{
	return o << *table;
}

} // namespace table
} // namespace v_1_0_0
} // namespace arcsapp

