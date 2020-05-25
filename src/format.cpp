/**
 * \file
 *
 * \brief Formatting interfaces and classes
 */

#ifndef __ARCSTOOLS_FORMAT_HPP__
#include "format.hpp"
#endif

#include <algorithm>           // for copy_if
#include <iomanip>             // for operator<<, setw, setfill
#include <stdexcept>           // for out_of_range
#include <utility>             // for move
#include <vector>              // for vector, vector<>::reference, _Bit_refe...


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
		flags_ &= ~(1 << idx); // <= false
	}
}


bool WithInternalFlags::flag(const int idx) const
{
	return flags_ & (1 << idx);
}


bool WithInternalFlags::no_flags() const
{
	return flags_ == 0;
}


bool WithInternalFlags::only_one_flag() const
{
	return flags_ && !(flags_ & (flags_ - 1));
}


bool WithInternalFlags::only(const int idx) const
{
	return flag(idx) && only_one_flag();
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
			| (url         << to_underlying(ARID_FLAG::URL))
			| (filename    << to_underlying(ARID_FLAG::FILENAME))
			| (track_count << to_underlying(ARID_FLAG::TRACKS))
			| (disc_id_1   << to_underlying(ARID_FLAG::ID1))
			| (disc_id_2   << to_underlying(ARID_FLAG::ID2))
			| (cddb_id     << to_underlying(ARID_FLAG::CDDBID))
		)
{
	// empty
}


ARIdLayout::~ARIdLayout() noexcept = default;


bool ARIdLayout::url() const
{
	return flag(to_underlying(ARID_FLAG::URL));
}


void ARIdLayout::set_url(const bool url)
{
	this->set_flag(to_underlying(ARID_FLAG::URL), url);
}


bool ARIdLayout::filename() const
{
	return flag(to_underlying(ARID_FLAG::FILENAME));
}


void ARIdLayout::set_filename(const bool filename)
{
	this->set_flag(to_underlying(ARID_FLAG::FILENAME), filename);
}


bool ARIdLayout::track_count() const
{
	return flag(to_underlying(ARID_FLAG::TRACKS));
}


void ARIdLayout::set_trackcount(const bool trackcount)
{
	this->set_flag(to_underlying(ARID_FLAG::TRACKS), trackcount);
}


bool ARIdLayout::disc_id_1() const
{
	return flag(to_underlying(ARID_FLAG::ID1));
}


void ARIdLayout::set_disc_id_1(const bool disc_id_1)
{
	this->set_flag(to_underlying(ARID_FLAG::ID1), disc_id_1);
}


bool ARIdLayout::disc_id_2() const
{
	return flag(to_underlying(ARID_FLAG::ID2));
}


void ARIdLayout::set_disc_id_2(const bool disc_id_2)
{
	this->set_flag(to_underlying(ARID_FLAG::ID2), disc_id_2);
}


bool ARIdLayout::cddb_id() const
{
	return flag(to_underlying(ARID_FLAG::CDDBID));
}


void ARIdLayout::set_cddb_id(const bool cddb_id)
{
	this->set_flag(to_underlying(ARID_FLAG::CDDBID), cddb_id);
}


bool ARIdLayout::has_only(const ARID_FLAG flag) const
{
	return only(to_underlying(flag));
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


TableStructure::~TableStructure() noexcept = default;


std::size_t TableStructure::rows() const
{
	return this->do_rows();
}


std::size_t TableStructure::columns() const
{
	return this->do_columns();
}


void TableStructure::set_width(const int col, const int width)
{
	this->do_set_width(col, width);
}


int TableStructure::width(const int col) const
{
	return this->do_width(col);
}


void TableStructure::set_alignment(const int col, const bool align)
{
	this->do_set_alignment(col, align);
}


bool TableStructure::alignment(const int col) const
{
	return this->do_alignment(col);
}


void TableStructure::set_title(const int col, const std::string &title)
{
	this->do_set_title(col, title);
}


std::string TableStructure::title(const int col) const
{
	return this->do_title(col);
}


void TableStructure::set_type(const int col, const int type)
{
	this->do_set_type(col, type);
}


int TableStructure::type(const int col) const
{
	return this->do_type(col);
}


void TableStructure::set_column_delimiter(const std::string &delim)
{
	this->do_set_column_delimiter(delim);
}


std::string TableStructure::column_delimiter() const
{
	return this->do_column_delimiter();
}


void TableStructure::resize(const int rows, const int cols)
{
	this->do_resize(rows, cols);
}


void TableStructure::bounds_check(const int row, const int col) const
{
	this->do_bounds_check(row, col);
}


/**
 * \brief Private implementation of a StringTableLayout
 */
class StringTableStructure::Impl final
{
public:

	Impl(const int rows, const int columns);

	Impl(const Impl &rhs);

	Impl(Impl &&rhs) noexcept;

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
	 * \brief Resize to new dimensions
	 */
	void resize(const int rows, const int cols);

	/**
	 * \brief Perform a bounds check
	 */
	void bounds_check(const int row, const int col) const;

	void bounds_check_row(const int row) const;

	void bounds_check_col(const int col) const;

	bool legal_row(const int row) const;

	bool legal_col(const int col) const;

	bool legal_width(const int width) const;

	void swap(Impl rhs);

private:

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


StringTableStructure::Impl::Impl(const int rows, const int cols)
	: widths_       (cols)
	, alignments_   (cols)
	, titles_       (cols)
	, types_        (cols)
	, rows_         { rows }
	, cols_         { cols }
	, column_delim_ { " " }
{
	// empty
}


StringTableStructure::Impl::Impl(const Impl &rhs) = default;


StringTableStructure::Impl::Impl(Impl &&rhs) noexcept = default;


StringTableStructure::Impl::~Impl() noexcept = default;


int StringTableStructure::Impl::rows() const
{
	return rows_;
}


int StringTableStructure::Impl::columns() const
{
	return cols_;
}


void StringTableStructure::Impl::set_width(const int col, const int width)
{
	widths_[col] = width;
}


int StringTableStructure::Impl::width(const int col)
{
	return widths_[col];
}


void StringTableStructure::Impl::set_alignment(const int col, const bool align)
{
	alignments_[col] = align;
}


bool StringTableStructure::Impl::alignment(const int col)
{
	return alignments_[col];
}


void StringTableStructure::Impl::set_title(const int col,
		const std::string &title)
{
	titles_[col] = title;
}


std::string StringTableStructure::Impl::title(const int col) const
{
	return titles_[col];
}


void StringTableStructure::Impl::set_type(const int col, const int type)
{
	types_[col] = type;
}


int StringTableStructure::Impl::type(const int col) const
{
	return types_[col];
}


void StringTableStructure::Impl::set_col_delim(const std::string &delim)
{
	column_delim_ = delim;
}


std::string StringTableStructure::Impl::col_delim() const
{
	return column_delim_;
}


void StringTableStructure::Impl::resize(const int rows, const int cols)
{
	rows_ = rows;
	cols_ = cols;

	widths_.resize(columns());
	alignments_.resize(columns());
	titles_.resize(columns());
	types_.resize(columns());
}


void StringTableStructure::Impl::bounds_check(const int row, const int col)
	const
{
	this->bounds_check_col(col);
	this->bounds_check_row(row);
}


void StringTableStructure::Impl::bounds_check_row(const int row) const
{
	if (not legal_row(row))
	{
		throw std::out_of_range("Row index " + std::to_string(row) +
				" illegal");
	}
}


void StringTableStructure::Impl::bounds_check_col(const int col) const
{
	if (not legal_col(col))
	{
		throw std::out_of_range("Column index " + std::to_string(col) +
				"illegal");
	}
}


void StringTableStructure::Impl::swap(StringTableStructure::Impl rhs)
{
	using std::swap;

	swap(widths_, rhs.widths_);
	swap(alignments_, rhs.alignments_);
	swap(titles_, rhs.titles_);
	swap(types_, rhs.types_);
	swap(rows_, rhs.rows_);
	swap(cols_, rhs.cols_);
	swap(column_delim_, rhs.column_delim_);
}


bool StringTableStructure::Impl::legal_row(const int row) const
{
	return row >= 0 && row < rows();
}


bool StringTableStructure::Impl::legal_col(const int col) const
{
	return col >= 0 && col < columns();
}


bool StringTableStructure::Impl::legal_width(const int width) const
{
	return width > 0 && width < 32; // defines maximum width
}


// StringTableLayout


StringTableStructure::StringTableStructure(const int rows, const int cols)
	: impl_ { std::make_unique<Impl>(rows, cols) }
{
	// empty
}


StringTableStructure::StringTableStructure(const StringTableStructure &rhs)
	: impl_ { std::make_unique<Impl>(*rhs.impl_) }
{
	// empty
}


StringTableStructure::~StringTableStructure() noexcept
= default;


void StringTableStructure::resize_layout(const int rows, const int cols) const
{
	impl_->resize(rows, cols);
}


bool StringTableStructure::legal_row(const int row) const
{
	return impl_->legal_row(row);
}


bool StringTableStructure::legal_col(const int col) const
{
	return impl_->legal_col(col);
}


void StringTableStructure::bounds_check_row(const int row) const
{
	return impl_->bounds_check_row(row);
}


void StringTableStructure::bounds_check_col(const int col) const
{
	return impl_->bounds_check_col(col);
}


std::size_t StringTableStructure::do_rows() const
{
	return impl_->rows();
}


std::size_t StringTableStructure::do_columns() const
{
	return impl_->columns();
}


void StringTableStructure::do_resize(const int rows, const int cols)
{
	resize_layout(rows, cols);
}


void StringTableStructure::do_set_width(const int col, const int width)
{
	impl_->set_width(col, width);
}


int StringTableStructure::do_width(const int col) const
{
	return impl_->width(col);
}


void StringTableStructure::do_set_alignment(const int col, const bool align)
{
	impl_->set_alignment(col, align);
}


bool StringTableStructure::do_alignment(const int col) const
{
	return impl_->alignment(col);
}


void StringTableStructure::do_set_title(const int col, const std::string &title)
{
	impl_->set_title(col, title);
}


std::string StringTableStructure::do_title(const int col) const
{
	return impl_->title(col);
}


void StringTableStructure::do_set_type(const int col, const int type)
{
	impl_->set_type(col, type);
}


int StringTableStructure::do_type(const int col) const
{
	return impl_->type(col);
}


void StringTableStructure::do_set_column_delimiter(const std::string &delim)
{
	impl_->set_col_delim(delim);
}


std::string StringTableStructure::do_column_delimiter() const
{
	return impl_->col_delim();
}


void StringTableStructure::do_bounds_check(const int row, const int col) const
{
	impl_->bounds_check(row, col);
}


/**
 * \brief Private implementation of a StringTableBase
 */
class StringTable::Impl final
{
public:

	Impl(const int rows, const int columns);

	Impl(const Impl &rhs);

	Impl(Impl &&rhs) noexcept;

	/**
	 * \brief Resize to new dimensions
	 */
	void resize(const int rows, const int cols);

	std::string cell(const int row, const int col) const;

	void update_cell(const int row, const int col, const std::string &text);

	int current_row() const;

	void swap(Impl rhs);

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

	/**
	 * \brief The table content.
	 */
	std::vector<std::string> cells_;

	/**
	 * \brief The first row that is not filled
	 */
	int current_row_;

	/**
	 * \brief Number of columns.
	 */
	int cols_; // TODO Redundant value, also in StringTableLayout
};


StringTable::Impl::Impl(const int rows, const int cols)
	: cells_ (rows * cols)
	, current_row_ (0)
	, cols_ (cols)
{
	// empty
}


StringTable::Impl::	Impl(const Impl &rhs) = default;


StringTable::Impl::	Impl(Impl &&rhs) noexcept = default;


void StringTable::Impl::resize(const int rows, const int cols)
{
	cols_ = cols;
	cells_.resize(rows * cols);
}


std::string StringTable::Impl::cell(const int row, const int col) const
{
	return cells_[index(row, col)];
}


void StringTable::Impl::update_cell(const int row, const int col,
		const std::string &text)
{
	cells_[index(row, col)] = text;
	if (row >= current_row_)
	{
		current_row_ = row + 1;
	}
}


int StringTable::Impl::current_row() const
{
	return current_row_;
}


void StringTable::Impl::swap(Impl rhs)
{
	using std::swap;

	swap(cells_, rhs.cells_);
	swap(cols_, rhs.cols_);
}


int StringTable::Impl::index(const int row, const int col) const
{
	return row * cols_ + col;
}


// StringTable


StringTable::StringTable(const int rows, const int columns)
	: StringTableStructure(rows, columns)
	, impl_(std::make_unique<StringTable::Impl>(rows, columns))
{
	// empty
}


StringTable::StringTable(const StringTable &rhs)
	: StringTableStructure(rhs)
	, impl_ { std::make_unique<StringTable::Impl>(*rhs.impl_) }
{
	// empty
}


StringTable::~StringTable() noexcept = default;


void StringTable::update_cell(const int row, const int col,
		const std::string &text)
{
	bounds_check_col(col);

	if (allow_grow_rows_)
	{
		if (not legal_row(row))
		{
			if (row > max_rows())
			{
				bounds_check_row(row); // just do what normally would happen
			} else
			{
				resize(row + 1, columns());
			}
		}
	} else
	{
		bounds_check_row(row);
	}

	this->do_update_cell(row, col, text);
}


std::string StringTable::cell(const int row, const int col) const
{
	this->bounds_check(row, col);

	auto text = impl_->cell(row, col);

	if (auto w1dth = static_cast<std::size_t>(width(col));
			text.length() > w1dth)
	{
		return text.substr(0, w1dth - 1) + "~";
	}

	return text;
}


std::string StringTable::operator() (const int row, const int col) const
{
	return impl_->cell(row, col);
}


int StringTable::current_row() const
{
	return impl_->current_row();
}


int StringTable::max_rows() const
{
	return 73; // FIXME Creepy, isn't it?
}


void StringTable::init(const int /* row */, const int /* col */)
{
	// empty
}


void StringTable::do_update_cell(const int row, const int col,
		const std::string &text)
{
	this->bounds_check(row, col);
	impl_->update_cell(row, col, text);
}


std::string StringTable::do_cell(const int row, const int col) const
{
	this->bounds_check(row, col);
	return impl_->cell(row, col);
}


void StringTable::do_resize(const int rows, const int cols)
{
	resize_layout(rows, cols); // resize StrinTableStructure
	impl_->resize(rows, cols);
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


// AlbumTableBase


AlbumTableBase::AlbumTableBase(const int rows, const int columns)
	: StringTableStructure(rows, columns)
	, WithMetadataFlagMethods(true, true, true, true)
{
	// empty
}


AlbumTableBase::AlbumTableBase(const int rows, const int columns,
			const bool show_track, const bool show_offset,
			const bool show_length, const bool show_filename)
	: StringTableStructure(rows, columns)
	, WithMetadataFlagMethods(
			show_track, show_offset, show_length, show_filename)
{
	// empty
}


int AlbumTableBase::total_metadata_columns() const
{
	int md_cols = 0;

	if (this->track())    { ++md_cols; }
	if (this->filename()) { ++md_cols; }
	if (this->offset())   { ++md_cols; }
	if (this->length())   { ++md_cols; }

	return md_cols;
}


void AlbumTableBase::assign_type(const int col,
		const AlbumTableBase::COL_TYPE type)
{
	set_type(col, convert_from(type));
}


AlbumTableBase::COL_TYPE AlbumTableBase::type_of(const int col) const
{
	return convert_to(type(col));
}


int AlbumTableBase::default_width(const COL_TYPE type) const
{
	const int default_w = default_title(type).length();
	int min_w = 0;

	switch (type)
	{
		case COL_TYPE::TRACK    : min_w =  2; break;
		case COL_TYPE::FILENAME : min_w = 12; break;
		case COL_TYPE::OFFSET   :
		case COL_TYPE::LENGTH   : min_w =  7; break;
		case COL_TYPE::MATCH    :
		case COL_TYPE::CHECKSUM : min_w =  8; break;
	}

	return std::max(min_w, default_w);
}


std::string AlbumTableBase::default_title(const COL_TYPE type) const
{
	std::string title = "";

	switch (type)
	{
		case COL_TYPE::TRACK    : title = "Track";    break;
		case COL_TYPE::FILENAME : title = "Filename"; break;
		case COL_TYPE::OFFSET   : title = "Offset";   break;
		case COL_TYPE::LENGTH   : title = "Length";   break;
		case COL_TYPE::MATCH    : title = "Theirs";   break;
		case COL_TYPE::CHECKSUM : title = "Mine";     break;
	}

	return title;
}


void AlbumTableBase::set_widths(const COL_TYPE type, const int width)
{
	for (std::size_t col = 0; col < columns(); ++col)
	{
		if (type_of(col) == type) { set_width(col, width); }
	}
}


int AlbumTableBase::columns_apply_settings()
{
	std::size_t md_cols = 0;

	if (track())    { assign_type(md_cols, COL_TYPE::TRACK);    ++md_cols; }
	if (filename()) { assign_type(md_cols, COL_TYPE::FILENAME); ++md_cols; }
	if (offset())   { assign_type(md_cols, COL_TYPE::OFFSET);   ++md_cols; }
	if (length())   { assign_type(md_cols, COL_TYPE::LENGTH);   ++md_cols; }

	COL_TYPE type = COL_TYPE::TRACK;
	for (std::size_t c = 0; c < md_cols; ++c)
	{
		type = type_of(c);

		set_title(c, default_title(type));
		set_width(c, default_width(type));
	}

	return md_cols;
}


int AlbumTableBase::convert_from(const AlbumTableBase::COL_TYPE type) const
{
	return to_underlying(type);
}


AlbumTableBase::COL_TYPE AlbumTableBase::convert_to(const int type) const
{
	return static_cast<COL_TYPE>(type);
}


std::vector<arcstk::checksum::type> AlbumTableBase::typelist(
		const Checksums &checksums) const
{

	// Assume identical type sets in each track
	const auto& used_types = checksums[0].types();
	const auto absent { used_types.end() };

	// Construct a list of all used types in the order they
	// appear in arcstk::checksum::types

	const auto& defined_types = arcstk::checksum::types;

	std::vector<arcstk::checksum::type> types_to_print{};
	types_to_print.reserve(defined_types.size());

	std::copy_if(defined_types.begin(), defined_types.end(),
			std::back_inserter(types_to_print),
			[used_types, absent](const arcstk::checksum::type& t)
			{
				return used_types.find(t) != absent;
			});

	return types_to_print;
}


void AlbumTableBase::print_column_titles(std::ostream &out) const
{
	for (std::size_t col = 0; col < columns(); ++col)
	{
		out << std::setw(width(col)) << std::left << title(col)
			<< column_delimiter();
	}

	out << std::endl;
}
