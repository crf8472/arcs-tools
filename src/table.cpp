/**
 * \file
 *
 * \brief Implementation of printable tables.
 */

#ifndef __ARCSTOOLS_TABLE_HPP__
#include "table.hpp"
#endif

namespace arcsapp
{

/**
 * \brief Private implementation of a StringTableStructure
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

	void set_table_title(const std::string &title);
	const std::string& table_title() const;

	void set_width(const int col, const int width);
	int width(const int col) const;

	void set_dynamic_width(const int col);
	bool has_dynamic_width(const int col) const;

	void set_alignment(const int col, const bool align);
	bool alignment(const int col) const;

	void set_type(const int col, const int type);
	int type(const int col) const;

	void set_title(const int col, const std::string &title);
	std::string title(const int col) const;

	void set_label(const int row, const std::string &label);
	std::string label(const int row) const;

	void set_col_delim(const std::string &delim);
	std::string col_delim() const;

	/**
	 * \brief Optimal width for printing the label column
	 */
	int optimal_label_width() const;

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
	 * \brief Number of rows.
	 */
	int rows_;

	/**
	 * \brief Number of columns.
	 */
	int cols_;

	/**
	 * \brief Title of this table.
	 */
	std::string title_;

	/**
	 * \brief The column widths.
	 */
	std::vector<int> widths_;

	/**
	 * \brief Dynamic width flags.
	 */
	std::vector<bool> dyn_width_;

	/**
	 * \brief The column alignments.
	 */
	std::vector<bool> alignments_;

	/**
	 * \brief The column types.
	 */
	std::vector<int> col_types_;

	/**
	 * \brief The column titles.
	 */
	std::vector<std::string> col_titles_;

	/**
	 * \brief The row titles.
	 */
	std::vector<std::string> row_labels_;

	/**
	 * \brief Column delimiter.
	 */
	std::string column_delim_;
};


StringTableStructure::Impl::Impl(const int rows, const int cols)
	: rows_         { rows }
	, cols_         { cols }
	, title_        {}
	, widths_       (cols)
	, dyn_width_    (cols)
	, alignments_   (cols)
	, col_types_    (cols)
	, col_titles_   (cols)
	, row_labels_   (rows)
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


void StringTableStructure::Impl::set_table_title(const std::string &title)
{
	title_ = title;
}


const std::string& StringTableStructure::Impl::table_title() const
{
	return title_;
}


void StringTableStructure::Impl::set_width(const int col, const int width)
{
	widths_[col] = width;
}


int StringTableStructure::Impl::width(const int col) const
{
	return widths_[col];
}


void StringTableStructure::Impl::set_dynamic_width(const int col)
{
	dyn_width_[col] = true;
}


bool StringTableStructure::Impl::has_dynamic_width(const int col) const
{
	return dyn_width_[col];
}


void StringTableStructure::Impl::set_alignment(const int col, const bool align)
{
	alignments_[col] = align;
}


bool StringTableStructure::Impl::alignment(const int col) const
{
	return alignments_[col];
}


void StringTableStructure::Impl::set_title(const int col,
		const std::string &title)
{
	col_titles_[col] = title;
}


std::string StringTableStructure::Impl::title(const int col) const
{
	return col_titles_[col];
}


void StringTableStructure::Impl::set_label(const int row,
		const std::string &label)
{
	row_labels_[row] = label;
}


std::string StringTableStructure::Impl::label(const int row) const
{
	return row_labels_[row];
}


void StringTableStructure::Impl::set_type(const int col, const int type)
{
	col_types_[col] = type;
}


int StringTableStructure::Impl::type(const int col) const
{
	return col_types_[col];
}


void StringTableStructure::Impl::set_col_delim(const std::string &delim)
{
	column_delim_ = delim;
}


std::string StringTableStructure::Impl::col_delim() const
{
	return column_delim_;
}


int StringTableStructure::Impl::optimal_label_width() const
{
	return optimal_width(row_labels_);
}


void StringTableStructure::Impl::resize(const int rows, const int cols)
{
	rows_ = rows;
	cols_ = cols;

	widths_.resize(columns());
	dyn_width_.resize(columns());
	alignments_.resize(columns());
	col_types_.resize(columns());

	col_titles_.resize(this->columns());
	row_labels_.resize(this->rows());
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

	swap(rows_, rhs.rows_);
	swap(cols_, rhs.cols_);
	swap(widths_, rhs.widths_);
	swap(dyn_width_, rhs.dyn_width_);
	swap(alignments_, rhs.alignments_);
	swap(col_types_, rhs.col_types_);
	swap(col_titles_, rhs.col_titles_);
	swap(row_labels_, rhs.row_labels_);
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


// StringTableStructure


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


StringTableStructure::StringTableStructure(StringTableStructure &&rhs)
	noexcept
	: impl_ { std::move(rhs.impl_) }
{
	// empty
}


StringTableStructure::~StringTableStructure() noexcept
= default;


std::size_t StringTableStructure::rows() const
{
	return this->do_rows();
}


std::size_t StringTableStructure::columns() const
{
	return this->do_columns();
}


void StringTableStructure::set_table_title(const std::string &title)
{
	this->do_set_table_title(title);
}


const std::string& StringTableStructure::table_title() const
{
	return this->do_table_title();
}


void StringTableStructure::set_width(const int col, const int width)
{
	this->do_set_width(col, width);
}


int StringTableStructure::width(const int col) const
{
	return this->do_width(col);
}


void StringTableStructure::set_dynamic_width(const int col)
{
	this->do_set_dynamic_width(col);
}


bool StringTableStructure::has_dynamic_width(const int col) const
{
	return this->do_has_dynamic_width(col);
}


void StringTableStructure::set_alignment(const int col, const bool align)
{
	this->do_set_alignment(col, align);
}


bool StringTableStructure::alignment(const int col) const
{
	return this->do_alignment(col);
}


void StringTableStructure::set_type(const int col, const int type)
{
	this->do_set_type(col, type);
}


int StringTableStructure::type(const int col) const
{
	return this->do_type(col);
}


void StringTableStructure::set_title(const int col, const std::string &title)
{
	this->do_set_title(col, title);
}


std::string StringTableStructure::title(const int col) const
{
	return this->do_title(col);
}


void StringTableStructure::set_row_label(const int row, const std::string &label)
{
	this->do_set_row_label(row, label);
}


std::string StringTableStructure::row_label(const int row) const
{
	return this->do_row_label(row);
}


void StringTableStructure::set_column_delimiter(const std::string &delim)
{
	this->do_set_column_delimiter(delim);
}


std::string StringTableStructure::column_delimiter() const
{
	return this->do_column_delimiter();
}


void StringTableStructure::resize(const int rows, const int cols)
{
	this->do_resize(rows, cols);
}


void StringTableStructure::bounds_check(const int row, const int col) const
{
	this->do_bounds_check(row, col);
}


int StringTableStructure::optimal_label_width() const
{
	return impl_->optimal_label_width();
}


void StringTableStructure::update_dimensions(const int rows, const int cols) const
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
	update_dimensions(rows, cols);
}


void StringTableStructure::do_set_table_title(const std::string &title)
{
	impl_->set_table_title(title);
}


const std::string & StringTableStructure::do_table_title() const
{
	return impl_->table_title();
}


void StringTableStructure::do_set_width(const int col, const int width)
{
	impl_->set_width(col, width);
}


int StringTableStructure::do_width(const int col) const
{
	return impl_->width(col);
}


void StringTableStructure::do_set_dynamic_width(const int col)
{
	impl_->set_dynamic_width(col);
}


bool StringTableStructure::do_has_dynamic_width(const int col) const
{
	return impl_->has_dynamic_width(col);
}


void StringTableStructure::do_set_alignment(const int col, const bool align)
{
	impl_->set_alignment(col, align);
}


bool StringTableStructure::do_alignment(const int col) const
{
	return impl_->alignment(col);
}


void StringTableStructure::do_set_type(const int col, const int type)
{
	impl_->set_type(col, type);
}


int StringTableStructure::do_type(const int col) const
{
	return impl_->type(col);
}


void StringTableStructure::do_set_title(const int col, const std::string &title)
{
	impl_->set_title(col, title);
}


std::string StringTableStructure::do_title(const int col) const
{
	return impl_->title(col);
}


void StringTableStructure::do_set_row_label(const int row,
		const std::string &label)
{
	impl_->set_label(row, label);
}


std::string StringTableStructure::do_row_label(const int row) const
{
	return impl_->label(row);
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


void StringTableStructure::print_title(std::ostream &out) const
{
	out << table_title() << std::endl;
}


void StringTableStructure::print_column_titles(std::ostream &out) const
{
	for (std::size_t col = 0; col < columns(); ++col)
	{
		out << std::setw(width(col)) << std::left << std::setfill(' ')
			<< title(col);

		if (col < columns() - 1) // Avoid delimiter after rightmost column
		{
			out << column_delimiter();
		}
	}

	out << std::endl;
}


void StringTableStructure::print_label(std::ostream &o, const int row) const
{
	if (optimal_label_width() == 0) { return; }

	o << std::setw(optimal_label_width())
		<< std::left // TODO Make Configurable?
		<< row_label(row)
		<< column_delimiter(); // TODO Always with delimiter?
}


void StringTableStructure::print_cell(std::ostream &o, const int col,
		const std::string &text, const bool with_delim) const
{
	if (auto curr_width = static_cast<std::size_t>(width(col));
			not has_dynamic_width(col) and text.length() > curr_width)
	{
		// Truncate cell (on column width)
		o << std::setw(width(col))
			<< (alignment(col) > 0 ? std::left : std::right)
			<< text.substr(0, curr_width - 1) + "~";
	} else
	{
		o << std::setw(width(col))
			<< (alignment(col) > 0 ? std::left : std::right)
			<< text;
	}

	if (with_delim)
	{
		o << column_delimiter();
	}
}


/**
 * \brief Private implementation of a StringTableBase
 */
class StringTableBase::Impl final
{
public:

	Impl(const int rows, const int columns, StringTableBase* owner);

	Impl(const Impl &rhs, StringTableBase* owner);

	Impl(const Impl &rhs) = delete; // Avoid warning for not providing

	Impl(Impl &&rhs, StringTableBase* owner) noexcept;

	Impl(Impl &&rhs) noexcept = delete;

	void resize(const int rows, const int cols);

	std::string cell(const int row, const int col) const;

	void update_cell(const int row, const int col, const std::string &text);

	int current_row() const;

	void swap(Impl rhs);

	Impl& operator = (const Impl &rhs) = delete;

	Impl& operator = (Impl &&rhs) noexcept = delete;

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
	 * \brief The "owning" StringTableBase.
	 */
	StringTableBase* owner_;

	/**
	 * \brief The first row that is not filled
	 */
	int current_row_;
};


StringTableBase::Impl::Impl(const int rows, const int cols,
		StringTableBase* owner)
	: cells_ (rows * cols)
	, owner_ { owner }
	, current_row_ { 0 }
{
	// empty
}


StringTableBase::Impl::	Impl(const Impl &rhs, StringTableBase *owner)
	: cells_ (rhs.cells_)
	, owner_ { owner }
	, current_row_ { rhs.current_row_ }
{
	// empty
}


StringTableBase::Impl::	Impl(Impl &&rhs, StringTableBase *owner) noexcept
	: cells_ (std::move(rhs.cells_))
	, owner_ { owner }
	, current_row_ { std::move(rhs.current_row_) }
{
	// empty
}


void StringTableBase::Impl::resize(const int rows, const int cols)
{
	cells_.resize(rows * cols);
}


std::string StringTableBase::Impl::cell(const int row, const int col) const
{
	return cells_[index(row, col)];
}


void StringTableBase::Impl::update_cell(const int row, const int col,
		const std::string &text)
{
	cells_[index(row, col)] = text;
	if (row >= current_row_)
	{
		current_row_ = row + 1;
	}
}


int StringTableBase::Impl::current_row() const
{
	return current_row_;
}


void StringTableBase::Impl::swap(Impl rhs)
{
	using std::swap;

	swap(cells_, rhs.cells_);
	swap(current_row_, rhs.current_row_);
	swap(owner_, rhs.owner_);
}


int StringTableBase::Impl::index(const int row, const int col) const
{
	return row * owner_->columns() + col;
}


// StringTableBase


StringTableBase::StringTableBase(const int rows, const int columns,
			const bool has_appending_rows)
	: StringTableStructure(rows, columns)
	, flags_ { has_appending_rows }
	, impl_(std::make_unique<StringTableBase::Impl>(rows, columns, this))
{
	// empty
}


StringTableBase::StringTableBase(const StringTableBase &rhs)
	: StringTableStructure(rhs)
	, flags_ { rhs.flags_ }
	, impl_  { std::make_unique<StringTableBase::Impl>(*rhs.impl_, this) }
{
	// empty
}


StringTableBase::StringTableBase(StringTableBase &&rhs) noexcept
	: StringTableStructure(std::move(rhs))
	, flags_ { std::move(rhs.flags_) }
	, impl_  { std::make_unique<StringTableBase::Impl>(std::move(*rhs.impl_),
			this) }
{
	// empty
}


StringTableBase::~StringTableBase() noexcept = default;


StringTableBase& StringTableBase::operator = (const StringTableBase &rhs)
{
	impl_ = std::make_unique<Impl>(std::move(*rhs.impl_), this);

	return *this;
}


StringTableBase& StringTableBase::operator = (StringTableBase &&rhs) noexcept
{
	impl_ = std::make_unique<Impl>(std::move(*rhs.impl_), this);

	return *this;
}


bool StringTableBase::has_appending_rows() const
{
	return flags_[0];
}


void StringTableBase::update_cell(const int row, const int col,
		const std::string &text)
{
	bounds_check_col(col);

	// Handle growth
	if (has_appending_rows())
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

	// Handle text with more width than column
	if (text.length() > static_cast<std::size_t>(width(col)))
	{
		if (has_dynamic_width(col))
		{
			set_width(col, text.length());
		} else
		{
			// TODO else: Multiline cell:
			//
		}
	}

	this->do_update_cell(row, col, text);
}


std::string StringTableBase::cell(const int row, const int col) const
{
	this->bounds_check(row, col);

	return impl_->cell(row, col);
}


std::string StringTableBase::operator() (const int row, const int col) const
{
	return impl_->cell(row, col);
}


int StringTableBase::current_row() const
{
	return impl_->current_row();
}


int StringTableBase::max_rows() const
{
	return 73; // FIXME Creepy, isn't it?
}


bool StringTableBase::empty() const
{
	return impl_->current_row() == 0;
}


void StringTableBase::do_update_cell(const int row, const int col,
		const std::string &text)
{
	// bounds checking is done in update_cell()
	impl_->update_cell(row, col, text);
}


std::string StringTableBase::do_cell(const int row, const int col) const
{
	this->bounds_check(row, col);
	return impl_->cell(row, col);
}


void StringTableBase::do_resize(const int rows, const int cols)
{
	update_dimensions(rows, cols); // resize StringTableStructure
	impl_->resize(rows, cols);
}


std::ostream& operator << (std::ostream &out, const StringTableBase &table)
{
	// Table title
	if (const auto& title = table.table_title(); not title.empty())
	{
		table.print_title(out);
	}

	const auto label_width = table.optimal_label_width();

	std::ios_base::fmtflags prev_settings = out.flags();

	// Column titles
	if (label_width > 0)
	{
		out << std::setw(label_width) << std::left << std::setfill(' ') << ' ';
	}
	table.print_column_titles(out);
	std::string text;

	// Row contents
	const auto last_col = table.columns() - 1;
	for (std::size_t row = 0; row < table.rows(); ++row)
	{
		table.print_label(out, row);
		for (std::size_t col = 0; col < table.columns(); ++col)
		{
			table.print_cell(out, col, table.cell(row, col), col < last_col);
		}
		out << std::endl;
	}

	out.flags(prev_settings);

	return out;
}


// StringTable


void StringTable::do_init(const int /* row */, const int /* col */)
{
	// empty
}

} // namespace arcsapp

