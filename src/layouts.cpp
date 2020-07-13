/**
 * \file
 *
 * \brief Formatting interfaces and classes
 */

#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#include "layouts.hpp"
#endif

#include <algorithm>           // for copy_if
#include <iomanip>             // for operator<<, setw, setfill
#include <sstream>             // for ostringstream
#include <stdexcept>           // for out_of_range
#include <utility>             // for move
#include <vector>              // for vector, vector<>::reference, _Bit_refe...


namespace arcsapp
{


/**
 * \brief Generate a list of the types used in \c checksums in the order they
 * appear in arcstk::checksum::types.
 *
 * \param[in] checksums The Checksums to inspect for checksums::types
 *
 * \return List of occurring checksum::type in the order libarcstk defines them
 */
std::vector<arcstk::checksum::type> ordered_typelist(const Checksums &checksums)
{
	// Assume identical type sets in each track
	const auto& used_types    = checksums[0].types();
	const auto& defined_types = arcstk::checksum::types;

	// Construct a list of all used types in the order they
	// appear in arcstk::checksum::types

	std::vector<arcstk::checksum::type> types_to_print { };
	types_to_print.reserve(defined_types.size());

	std::copy_if(defined_types.begin(), defined_types.end(),
			std::back_inserter(types_to_print),
			[used_types](const arcstk::checksum::type& t)
			{
				return used_types.find(t) != used_types.end();
			});

	return types_to_print;
}


namespace defaults
{

std::string label(const CELL_TYPE type)
{
	std::string title = "";

	switch (type)
	{
		case CELL_TYPE::TRACK    : title = "Track";    break;
		case CELL_TYPE::FILENAME : title = "Filename"; break;
		case CELL_TYPE::OFFSET   : title = "Offset";   break;
		case CELL_TYPE::LENGTH   : title = "Length";   break;
		case CELL_TYPE::MATCH    : title = "Theirs";   break;
		case CELL_TYPE::CHECKSUM : title = "Mine";     break;
	}

	return title;
}

int width(const CELL_TYPE type)
{
	const int default_w = defaults::label(type).length();
	int min_w = 0;

	switch (type)
	{
		case CELL_TYPE::TRACK    : min_w =  2; break;
		case CELL_TYPE::FILENAME : min_w = 12; break;
		case CELL_TYPE::OFFSET   :
		case CELL_TYPE::LENGTH   : min_w =  7; break;
		case CELL_TYPE::MATCH    :
		case CELL_TYPE::CHECKSUM : min_w =  8; break;
	}

	return std::max(min_w, default_w);
}

} // namespace defaults


int convert_from(const CELL_TYPE type)
{
	return to_underlying(type);
}


CELL_TYPE convert_to(const int type)
{
	return static_cast<CELL_TYPE>(type);
}


// InternalFlags


InternalFlags::InternalFlags(const uint32_t flags)
	: flags_(flags)
{
	// empty
}


void InternalFlags::set_flag(const int idx, const bool value)
{
	if (value)
	{
		flags_ |= (1 << idx);  // <= true
	} else
	{
		flags_ &= ~(1 << idx); // <= false
	}
}


bool InternalFlags::flag(const int idx) const
{
	return flags_ & (1 << idx);
}


bool InternalFlags::no_flags() const
{
	return flags_ == 0;
}


bool InternalFlags::only_one_flag() const
{
	return flags_ && !(flags_ & (flags_ - 1));
}


bool InternalFlags::only(const int idx) const
{
	return flag(idx) && only_one_flag();
}


// ARTripletLayout


std::string ARTripletLayout::do_format(ArgsRefTuple t) const
{
	//assertions(t);

	const auto track   = *std::get<0>(t);
	const auto triplet = *std::get<1>(t);

	HexLayout hex; // TODO Make this configurable, inherit from WithChecksums...
	hex.set_show_base(false);
	hex.set_uppercase(true);

	const int width_arcs = 8;
	const int width_conf = 2;

	const auto unparsed_value = std::string { "????????" };

	std::ostringstream out;

	// TODO Make label configurable
	out << "Track " << std::setw(2) << std::setfill('0') << track << ": ";

	if (triplet.arcs_valid())
	{
		out << std::setw(width_arcs)
			<< hex.format(Checksum { triplet.arcs() }, width_arcs);
	} else
	{
		out << std::setw(width_arcs) << unparsed_value;
	}

	out << " ";

	out << "(";
	if (triplet.confidence_valid())
	{
		out << std::setw(width_conf) << std::setfill('0')
			<< static_cast<unsigned int>(triplet.confidence());
	} else
	{
		out << "??";
	}
	out << ") ";

	if (triplet.frame450_arcs_valid())
	{
		out << std::setw(width_arcs)
			<< hex.format(Checksum { triplet.frame450_arcs() }, width_arcs);
	} else
	{
		out << std::setw(width_arcs) << unparsed_value;
	}

	out << std::endl;

	return out.str();
}


// HexLayout


HexLayout::HexLayout()
	: WithInternalFlags()
{
	set_show_base(false);
	set_uppercase(true);
}


void HexLayout::set_show_base(const bool base)
{
	settings().set_flag(0, base);
}


bool HexLayout::shows_base() const
{
	return settings().flag(0);
}


void HexLayout::set_uppercase(const bool uppercase)
{
	settings().set_flag(1, uppercase);
}


bool HexLayout::is_uppercase() const
{
	return settings().flag(1);
}


std::string HexLayout::do_format(ArgsRefTuple t) const
{
	auto checksum = *std::get<0>(t);
	auto width    = *std::get<1>(t);

	std::ostringstream ss;

	if (shows_base())
	{
		ss << std::showbase;
	}

	if (is_uppercase())
	{
		ss << std::uppercase;
	}

	ss << std::hex << std::setw(width) << std::setfill('0') << checksum.value();

	return ss.str();
}


// WithChecksumLayout


WithChecksumLayout::WithChecksumLayout()
	: checksum_layout_ { std::make_unique<HexLayout>() }
{
	// empty
}


WithChecksumLayout::~WithChecksumLayout() noexcept = default;


void WithChecksumLayout::set_checksum_layout(
		std::unique_ptr<ChecksumLayout> &layout)
{
	checksum_layout_ = std::move(layout);
}


const ChecksumLayout& WithChecksumLayout::checksum_layout() const
{
	return *checksum_layout_;
}


// ARIdLayout


ARIdLayout::ARIdLayout()
	: WithInternalFlags { 0xFFFFFFFF } // all flags true
{
	// empty
}


ARIdLayout::ARIdLayout(const bool id, const bool url, const bool filename,
		const bool track_count, const bool disc_id_1, const bool disc_id_2,
		const bool cddb_id)
	: WithInternalFlags(
			0
			| (id          << to_underlying(ARID_FLAG::ID))
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


bool ARIdLayout::id() const
{
	return settings().flag(to_underlying(ARID_FLAG::ID));
}


void ARIdLayout::set_id(const bool id)
{
	settings().set_flag(to_underlying(ARID_FLAG::ID), id);
}


bool ARIdLayout::url() const
{
	return settings().flag(to_underlying(ARID_FLAG::URL));
}


void ARIdLayout::set_url(const bool url)
{
	settings().set_flag(to_underlying(ARID_FLAG::URL), url);
}


bool ARIdLayout::filename() const
{
	return settings().flag(to_underlying(ARID_FLAG::FILENAME));
}


void ARIdLayout::set_filename(const bool filename)
{
	settings().set_flag(to_underlying(ARID_FLAG::FILENAME), filename);
}


bool ARIdLayout::track_count() const
{
	return settings().flag(to_underlying(ARID_FLAG::TRACKS));
}


void ARIdLayout::set_trackcount(const bool trackcount)
{
	settings().set_flag(to_underlying(ARID_FLAG::TRACKS), trackcount);
}


bool ARIdLayout::disc_id_1() const
{
	return settings().flag(to_underlying(ARID_FLAG::ID1));
}


void ARIdLayout::set_disc_id_1(const bool disc_id_1)
{
	settings().set_flag(to_underlying(ARID_FLAG::ID1), disc_id_1);
}


bool ARIdLayout::disc_id_2() const
{
	return settings().flag(to_underlying(ARID_FLAG::ID2));
}


void ARIdLayout::set_disc_id_2(const bool disc_id_2)
{
	settings().set_flag(to_underlying(ARID_FLAG::ID2), disc_id_2);
}


bool ARIdLayout::cddb_id() const
{
	return settings().flag(to_underlying(ARID_FLAG::CDDBID));
}


void ARIdLayout::set_cddb_id(const bool cddb_id)
{
	settings().set_flag(to_underlying(ARID_FLAG::CDDBID), cddb_id);
}


bool ARIdLayout::has_only(const ARID_FLAG flag) const
{
	return settings().only(to_underlying(flag));
}


auto ARIdLayout::show_flags() const -> decltype( show_flags_ )
{
	return show_flags_;
}


auto ARIdLayout::labels() const -> decltype( labels_ )
{
	return labels_;
}


std::string ARIdLayout::hex_id(const uint32_t id) const
{
	std::ostringstream out;

	out << std::hex << std::uppercase << std::setfill('0')
		<< std::setw(8) << id;

	return out.str();
}


// ARIdTableLayout


std::string ARIdTableLayout::do_format(ArgsRefTuple t) const
{
	auto arid       = *std::get<0>(t);
	auto alt_prefix = std::get<1>(t) ? *std::get<1>(t) : std::string{};

	if (settings().no_flags()) // return ARId as default, ignore any Hex layout settings
	{
		return arid.to_string();
	}

	const int total_labels = id() + url() + filename() + track_count() +
		disc_id_1() + disc_id_2() + cddb_id();

	auto stream = std::ostringstream {};

	auto value = std::string {};

	for (const auto& sflag : show_flags())
	{
		if (not settings().flag(to_underlying(sflag))) { continue; }

		if (total_labels > 1)
		{
			if (!stream.str().empty()) { stream << std::endl; }

			stream << std::setw(8 /* length of 'Filename', longest label */)
				<< std::left
				<< labels()[to_underlying(sflag)]
				<< " ";
		}

		switch (sflag)
		{
			case ARID_FLAG::ID:
				value = arid.to_string();
				break;
			case ARID_FLAG::URL:
				value = arid.url();
				if (not alt_prefix.empty())
				{
					value.replace(0, arid.prefix().length(), alt_prefix);
					// FIXME If alt_prefix does not end with '/' ?
				}
				break;
			case ARID_FLAG::FILENAME:
				value = arid.filename();
				break;
			case ARID_FLAG::TRACKS:
				value = std::to_string(arid.track_count());
				break;
			case ARID_FLAG::ID1:
				value = hex_id(arid.disc_id_1());
				break;
			case ARID_FLAG::ID2:
				value = hex_id(arid.disc_id_2());
				break;
			case ARID_FLAG::CDDBID:
				value = hex_id(arid.cddb_id());
				break;
			default:
				break;
		}

		stream << std::setw(value.length()) << value;
	}

	if (true) { stream << std::endl; } // TODO Make configurable

	return stream.str();
}


// WithARIdLayout


WithARIdLayout::WithARIdLayout()
	: arid_layout_(nullptr)
{
	// empty
}


WithARIdLayout::WithARIdLayout(std::unique_ptr<ARIdLayout> arid_layout)
	: arid_layout_(std::move(arid_layout))
{
	// empty
}


WithARIdLayout::~WithARIdLayout() noexcept = default;


void WithARIdLayout::set_arid_layout(std::unique_ptr<ARIdLayout> format)
{
	if (arid_layout_)
	{
		arid_layout_.reset();
	}

	arid_layout_ = std::move(format);
}


ARIdLayout* WithARIdLayout::arid_layout()
{
	return arid_layout_ ? arid_layout_.get() : nullptr;
}


// WithMetadataFlagMethods


WithMetadataFlagMethods::WithMetadataFlagMethods(
		const bool show_label,
		const bool show_track,
		const bool show_offset,
		const bool show_length,
		const bool show_filename)
	: WithInternalFlags(0
			|  show_label   //  0
			| (show_track    << 1)
			| (show_offset   << 2)
			| (show_length   << 3)
			| (show_filename << 4))
{
	// empty
}


WithMetadataFlagMethods::~WithMetadataFlagMethods() noexcept = default;


bool WithMetadataFlagMethods::label() const
{
	return settings().flag(0);
}


void WithMetadataFlagMethods::set_label(const bool &label)
{
	settings().set_flag(0, label);
}


bool WithMetadataFlagMethods::track() const
{
	return settings().flag(1);
}


void WithMetadataFlagMethods::set_track(const bool &track)
{
	settings().set_flag(1, track);
}


bool WithMetadataFlagMethods::offset() const
{
	return settings().flag(2);
}


void WithMetadataFlagMethods::set_offset(const bool &offset)
{
	settings().set_flag(2, offset);
}


bool WithMetadataFlagMethods::length() const
{
	return settings().flag(3);
}


void WithMetadataFlagMethods::set_length(const bool &length)
{
	settings().set_flag(3, length);
}


bool WithMetadataFlagMethods::filename() const
{
	return settings().flag(4);
}


void WithMetadataFlagMethods::set_filename(const bool &filename)
{
	settings().set_flag(4, filename);
}


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
	o << std::setw(optimal_label_width())
		<< std::left // TODO Make Configurable?
		<< row_label(row)
		<< column_delimiter();
}


void StringTableStructure::print_cell(std::ostream &o, const int col,
		const std::string &text, const bool with_delim) const
{
	o << std::setw(width(col))
		<< (alignment(col) > 0 ? std::left : std::right)
		<< text;

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
			const bool dyn_col_widths, const bool allow_append_rows)
	: StringTableStructure(rows, columns)
	, flags_ { dyn_col_widths, allow_append_rows }
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


bool StringTableBase::has_dynamic_widths() const
{
	return flags_[0];
}


bool StringTableBase::has_appending_rows() const
{
	return flags_[1];
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

	// Handle dynamic column width
	if (auto curr_width = static_cast<std::size_t>(width(col));
		has_dynamic_widths() && text.length() > curr_width)
	{
		set_width(col, text.length());
	}

	this->do_update_cell(row, col, text);
}


std::string StringTableBase::cell(const int row, const int col) const
{
	this->bounds_check(row, col);

	auto text = impl_->cell(row, col);

	// Truncate cell content on column width
	if (auto curr_width = static_cast<std::size_t>(width(col));
			text.length() > curr_width)
	{
		return text.substr(0, curr_width - 1) + "~";
	}

	return text;
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
	out << std::setw(label_width) << std::left << std::setfill(' ') << ' ';
	table.print_column_titles(out);

	// Row contents
	for (std::size_t row = 0; row < table.rows(); ++row)
	{
		table.print_label(out, row);

		for (std::size_t col = 0; col < table.columns(); ++col)
		{
			table.print_cell(out, col, table.cell(row, col),
					col < table.columns() - 1);
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


// TypedRowsTableBase


TypedRowsTableBase::TypedRowsTableBase(
		const int rows,
		const int columns,
		const bool label,
		const bool track,
		const bool offset,
		const bool length,
		const bool filename)
	: WithARIdLayout()
	, WithChecksumLayout()
	, StringTableStructure(rows, columns)
	, WithMetadataFlagMethods(label, track, offset, length, filename)
{
	// empty
}


// TypedColsTableBase


TypedColsTableBase::TypedColsTableBase(
		const int rows,
		const int columns,
		const bool label,
		const bool track,
		const bool offset,
		const bool length,
		const bool filename)
	: WithARIdLayout()
	, WithChecksumLayout()
	, StringTableStructure(rows, columns)
	, WithMetadataFlagMethods(label, track, offset, length, filename)
{
	// empty
}


void TypedColsTableBase::assign_type(const int col, const CELL_TYPE type)
{
	set_type(col, convert_from(type));
}


CELL_TYPE TypedColsTableBase::type_of(const int col) const
{
	return convert_to(type(col));
}


void TypedColsTableBase::set_widths(const CELL_TYPE type, const int width)
{
	for (std::size_t col = 0; col < columns(); ++col)
	{
		if (type_of(col) == type) { set_width(col, width); }
	}
}


int TypedColsTableBase::total_metadata_columns() const
{
	int md_cols = 0;

	if (this->track())    { ++md_cols; }
	if (this->filename()) { ++md_cols; }
	if (this->offset())   { ++md_cols; }
	if (this->length())   { ++md_cols; }

	return md_cols;
}


int TypedColsTableBase::columns_apply_md_settings()
{
	std::size_t md_cols = 0;

	if (track())    { assign_type(md_cols, CELL_TYPE::TRACK);    ++md_cols; }
	if (filename()) { assign_type(md_cols, CELL_TYPE::FILENAME); ++md_cols; }
	if (offset())   { assign_type(md_cols, CELL_TYPE::OFFSET);   ++md_cols; }
	if (length())   { assign_type(md_cols, CELL_TYPE::LENGTH);   ++md_cols; }

	CELL_TYPE type = CELL_TYPE::TRACK;
	for (std::size_t c = 0; c < md_cols; ++c)
	{
		type = type_of(c);

		set_title(c, defaults::label(type));
		set_width(c, defaults::width(type));
	}

	return md_cols;
}


// TableUser


TableUser::TableUser(const bool label, const bool track,
			const bool offset, const bool length, const bool filename,
			const std::string &coldelim)
	: WithMetadataFlagMethods(label, track, offset, length, filename)
	, coldelim_ { coldelim }
{
	// empty
}


std::string TableUser::column_delimiter() const
{
	return coldelim_;
}


void TableUser::set_column_delimiter(const std::string &coldelim)
{
	coldelim_ = coldelim;
}


// CalcAlbumTableLayout


int CalcAlbumTableLayout::TableLayout::columns_apply_cs_settings(
		const std::vector<arcstk::checksum::type> &types)
{
	// Type each column to the "right" of the metadata columns as 'CHECKSUM'
	// and apply defaults

	std::size_t col = total_metadata_columns();
	auto type_to_print = types.cbegin();
	while (col < columns() and type_to_print != types.cend())
	{
		assign_type(col, CELL_TYPE::CHECKSUM);
		set_title(col, arcstk::checksum::type_name(*type_to_print));
		set_width(col, defaults::width(CELL_TYPE::CHECKSUM));

		++col;
		++type_to_print;
	}

	return col - total_metadata_columns();
}


std::string CalcAlbumTableLayout::do_format(ArgsRefTuple t) const
{
	//assertions(t);

	const auto checksums = std::get<0>(t);
	const auto filenames = std::get<1>(t);
	const auto toc       = std::get<2>(t);
	//const auto arid      = std::get<3>(t); // TODO Implement printing
	//const auto is_album  = std::get<4>(t);

	const auto types_to_print = ordered_typelist(*checksums);

	if (types_to_print.empty())
	{
		throw std::invalid_argument("Missing value: "
				"Checksums seem to hold no checksums");
	}


	// Configure table layout

	TableLayout lyt { 0, 0, label(), track(), offset(), length(), filename() };
	//lyt.set_column_delimiter(coldelim_);

	if (!toc) { lyt.set_offset(false); }
	if (filenames->empty()) { lyt.set_filename(false); }
	// assertion is fulfilled that either not filenames.empty() or non-null toc

	// TODO Use init() instead
	lyt.resize(checksums->size() + label(),
			lyt.total_metadata_columns() + types_to_print.size());
	// assertion is fulfilled that rows >= 1 as well as columns >= 1

	const auto md_offset = lyt.columns_apply_md_settings();
	lyt.columns_apply_cs_settings(types_to_print);
	lyt.set_widths(CELL_TYPE::FILENAME, optimal_width(*filenames));


	// Print table

	std::ostringstream out;

	if (label()) { lyt.print_column_titles(out); }

	arcstk::checksum::type cstype = arcstk::checksum::type::ARCS2; // default
	std::string cell{};
	int trackno = 1;;

	for (std::size_t row = 0; row < lyt.rows() - 1; ++row, ++trackno) // print row
	{
		for (std::size_t col = 0; col < lyt.columns(); ++col) // print cell
		{
			switch (lyt.type_of(col))
			{
				case CELL_TYPE::TRACK    :
					if (track())
						{ cell = std::to_string(trackno); }
					break;

				case CELL_TYPE::FILENAME :
					if (filename())
						{ cell = filenames->at(row); }
					break;

				case CELL_TYPE::OFFSET   :
					if (offset())
						{ cell = std::to_string(toc->offset(trackno)); }
					break;

				case CELL_TYPE::LENGTH   :
					if (length())
						{ cell = std::to_string((*checksums)[row].length()); }
					break;

				case CELL_TYPE::CHECKSUM :
					cstype = types_to_print[col - md_offset];
					cell = lyt.checksum_layout().format(
							(*checksums)[row].get(cstype), lyt.width(col));
					break;

				case CELL_TYPE::MATCH    : break;
			}

			lyt.print_cell(out, col, cell, col < lyt.columns() - 1);
		}
		out << std::endl;
	}

	return out.str();
}


// CalcTracksTableLayout


std::string CalcTracksTableLayout::do_format(ArgsRefTuple t) const
{
	//assertions(t);

	const auto checksums = std::get<0>(t);
	const auto filenames = std::get<1>(t);
	const auto toc       = std::get<2>(t);
	//const auto arid      = std::get<3>(t); // TODO Implement printing
	const auto is_album  = std::get<4>(t);

	const auto types_to_print = ordered_typelist(*checksums);

	if (types_to_print.empty())
	{
		throw std::invalid_argument("Missing value: "
				"Checksums seem to hold no checksums");
	}

	// Configure table

	TableLayout lyt { 0, 0, label(), track(), offset(), length(), filename() };
	//lyt.set_column_delimiter(coldelim_);

	if (is_album) { lyt.set_track(true); }
	if (!toc) { lyt.set_offset(false); }
	if (!filenames or filenames->empty()) { lyt.set_filename(false); }
	// assertion is fulfilled that either not filenames.empty() or non-null toc

	const bool show_input = filename(); // Overrides --no-track-nos

	// TODO Use init() instead
	lyt.resize(show_input + offset() + length() + types_to_print.size(),
				checksums->size());

	// Assign row labels
	if (label())
	{
		// Determine row labels (we must know the optimal width when printing)
		// XXX TypedColsTableBase does this kind of stuff in apply_md_settings()

		int row = 0;
		if (show_input)
		{
			// track() is interpreted differently when --tracks-as-cols:
			// Print either track number or file input number if tracks are
			// not available. Conversely, ignore filename().

			const std::string input_label = is_album
				? defaults::label(CELL_TYPE::TRACK)
				: defaults::label(CELL_TYPE::FILENAME);

			lyt.set_row_label(row, input_label);
			++row;
		}
		for (const auto& type : types_to_print)
		{
			lyt.set_row_label(row, arcstk::checksum::type_name(type));
			++row;
		}
		if (offset())
			{ lyt.set_row_label(row, defaults::label(CELL_TYPE::OFFSET));
				++row; }
		if (length())
			{ lyt.set_row_label(row, defaults::label(CELL_TYPE::LENGTH));
				++row; }
		// ... skip filename ...
	}

	// Assign column widths
	for (std::size_t col = 0; col < lyt.columns(); ++col) // print cell
	{
		lyt.set_width(col, 8); // TODO Magic number, only ok while no filenames
		lyt.set_alignment(col, false);
	}

	// Print table rows

	std::ostringstream o;
	int row = 0;

	if (show_input)
	{
		if (label()) { lyt.print_label(o, row); }

		int trackno = 1;
		for (std::size_t col = 0; col < lyt.columns() - 1; ++col)
		{
			lyt.print_cell(o, col, std::to_string(trackno), true);
			++trackno;
		}
		lyt.print_cell(o, lyt.columns() - 1, std::to_string(trackno), false);

		o << std::endl;
		++row;
	}

	for (const auto& type : types_to_print)
	{
		if (label()) { lyt.print_label(o, row); }

		int index = 0;
		for (std::size_t col = 0; col < lyt.columns() - 1; ++col, ++index)
		{
			lyt.print_cell(o, col,
				lyt.checksum_layout().format(checksums->at(index).get(type), 8),
				true);
		}

		lyt.print_cell(o, lyt.columns() - 1,
				lyt.checksum_layout().format(checksums->at(index).get(type), 8),
				false);

		o << std::endl;
		++row;
	}

	if (offset())
	{
		if (label()) { lyt.print_label(o, row); }

		int trackno = 1;
		for (std::size_t col = 0; col < lyt.columns() - 1; ++col)
		{
			lyt.print_cell(o, col, std::to_string(toc->offset(trackno)), true);
			++trackno;
		}
		lyt.print_cell(o, lyt.columns() - 1,
			std::to_string(toc->offset(trackno)), false);

		o << std::endl;
		++row;
	}

	if (length())
	{
		if (label()) { lyt.print_label(o, row); }

		int index = 0;
		for (std::size_t col = 0; col < lyt.columns() - 1; ++col, ++index)
		{
			lyt.print_cell(o, col,
				std::to_string(checksums->at(index).length()),
				true);
		}
		lyt.print_cell(o, lyt.columns() - 1,
				std::to_string(checksums->at(index).length()),
				false);

		o << std::endl;
		++row;
	}

	return o.str();
}


// VerifyResultLayout



void VerifyResultLayout::set_match_symbol(const std::string &match_symbol)
{
	match_symbol_ = match_symbol;
}


const std::string& VerifyResultLayout::match_symbol() const
{
	return match_symbol_;
}


// VerifyTableLayout


int VerifyTableLayout::TableLayout::columns_apply_cs_settings(
		const std::vector<arcstk::checksum::type> &types)
{
	// Type each column on the "right" as 'CHECKSUM' and 'MATCH' in
	// alternating order and apply defaults

	auto type_to_print = types.cbegin();
	std::size_t col_idx = total_metadata_columns();

	// Add the column with reference values ("Theirs")

	assign_type(col_idx, CELL_TYPE::CHECKSUM);
	set_title(col_idx, "Theirs");
	set_width(col_idx, defaults::width(CELL_TYPE::CHECKSUM));

	++col_idx;

	// Add the columns with my values

	using TYPE = arcstk::checksum::type;

	while (col_idx < columns() and type_to_print != types.cend())
	{
		assign_type(col_idx, CELL_TYPE::MATCH);
		set_title(col_idx,
				"Mine v" + std::to_string(1 + (*type_to_print == TYPE::ARCS2)));
		set_width(col_idx, defaults::width(CELL_TYPE::MATCH));

		++col_idx;

		++type_to_print;
	}

	return col_idx - total_metadata_columns();
}


std::string VerifyTableLayout::do_format(ArgsRefTuple t) const
{
	//assertions(t);

	const auto checksums = std::get<0>(t);
	const auto filenames = std::get<1>(t);
	const auto refsums   = std::get<2>(t);
	const auto match     = *std::get<3>(t);
	const auto block     = std::get<4>(t);
	const auto version   = std::get<5>(t);
	const auto toc       = std::get<6>(t);
	//const auto arid      = std::get<7>(t);

	using TYPE = arcstk::checksum::type;

	std::vector<TYPE> types_to_print;
	if (!version)
	{
		types_to_print = std::vector<TYPE>{ TYPE::ARCS2, TYPE::ARCS1 };
	} else
	{
		if (*version)
		{
			types_to_print.push_back(TYPE::ARCS2);
		} else
		{
			types_to_print.push_back(TYPE::ARCS1);
		}
	}
	types_to_print.shrink_to_fit();

	// Configure table layout

	TableLayout lyt { 0, 0, label(), track(), offset(), length(), filename() };
	//lyt.set_column_delimiter(coldelim_);

	// Determine number of rows
	const int total_entries = 1 /* col titles */
		+ (toc ? checksums->size()
			   : std::max(checksums->size(), refsums->size()));

	if (!toc) { lyt.set_offset(false); }

	// TODO Use init() instead
	lyt.resize(total_entries,
			lyt.total_metadata_columns() + types_to_print.size() + 1);

	const auto md_offset = lyt.columns_apply_md_settings();
	lyt.set_widths(CELL_TYPE::FILENAME, optimal_width(*filenames));

	lyt.columns_apply_cs_settings(types_to_print);

	// Print contents

	std::ostringstream out;

	if (label()) { lyt.print_column_titles(out); }

	TYPE cstype = TYPE::ARCS2; // default
	std::string cell{};
	int trackno = 1;

	for (std::size_t row = 0; row < lyt.rows() - 1; ++row, ++trackno) // print row
	{
		for (std::size_t col = 0; col < lyt.columns(); ++col) // print cell
		{
			switch (lyt.type_of(col))
			{
				case CELL_TYPE::TRACK    :
					if (track())
						{ cell = std::to_string(trackno); }
					break;

				case CELL_TYPE::FILENAME :
					if (filename())
						{ cell = filenames->at(row); }
					break;

				case CELL_TYPE::OFFSET   :
					if (offset())
						{ cell = std::to_string(toc->offset(trackno)); }
					break;

				case CELL_TYPE::LENGTH   :
					if (length())
						{ cell = std::to_string(checksums->at(row).length()); }
					break;

				case CELL_TYPE::CHECKSUM : // "Theirs" column
					cell = lyt.checksum_layout().format((*refsums)[row],
							lyt.width(col));
					break;

				case CELL_TYPE::MATCH    : // "Mine" columns (may be one or two)
					cstype = types_to_print[col - md_offset - 1];
					if (match->track(*block, row, cstype == TYPE::ARCS2))
					{
						cell = match_symbol();
					} else
					{
						cell = lyt.checksum_layout().format(
							checksums->at(row).get(cstype),
							lyt.width(col));
					}
					break;
			}

			lyt.print_cell(out, col, cell, col < lyt.columns() - 1);
		}

		out << std::endl;
	}

	return out.str();
}

} // namespace arcsapp

