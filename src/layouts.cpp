/**
 * \file
 *
 * \brief Formatting interfaces and classes
 */

#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#include "layouts.hpp"
#endif
#ifndef __ARCSTOOLS_TABLE_HPP__
#include "table.hpp"
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
	const auto track   = std::get<0>(t);
	const auto triplet = std::get<1>(t);

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
	auto checksum = std::get<0>(t);
	auto width    = std::get<1>(t);

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
	, field_labels_ { true }
{
	// empty
}


ARIdLayout::ARIdLayout(const bool labels, const bool id, const bool url,
		const bool filename, const bool track_count, const bool disc_id_1,
		const bool disc_id_2, const bool cddb_id)
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
	, field_labels_ { labels }
{
	// empty
}


ARIdLayout::~ARIdLayout() noexcept = default;


bool ARIdLayout::fieldlabels() const
{
	return field_labels_;
}


void ARIdLayout::set_fieldlabels(const bool labels)
{
	field_labels_ = labels;
}


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
	auto arid       = std::get<0>(t);
	auto alt_prefix = std::get<1>(t);

	if (settings().no_flags()) // return ARId as default
	{
		return arid.to_string();
	}

	auto stream = std::ostringstream {};
	auto value = std::string {};

	// TODO Use optimal_label_width?
	auto label_width = fieldlabels() ? optimal_width(labels()) : 0;

	for (const auto& sflag : show_flags())
	{
		if (not settings().flag(to_underlying(sflag))) { continue; }

		if (!stream.str().empty()) { stream << std::endl; }

		if (fieldlabels())
		{
			stream << std::setw(label_width)
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


int TypedRowsTableBase::rows_apply_md_settings(const bool is_album)
{
	int row = 0;
	if (filename())
	{
		// track() is interpreted differently when --tracks-as-cols:
		// Print either track number or file input number if tracks are
		// not available and ignore filename().

		const std::string input_label = is_album
			? defaults::label(CELL_TYPE::TRACK)
			: defaults::label(CELL_TYPE::FILENAME);

		set_row_label(row, input_label);
		++row;
	}
	if (offset())
		{ set_row_label(row, defaults::label(CELL_TYPE::OFFSET)); ++row; }
	if (length())
		{ set_row_label(row, defaults::label(CELL_TYPE::LENGTH)); ++row; }

	return row;
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


// CalcResultLayout


void CalcResultLayout::assertions(ArgsRefTuple t) const
{
	const auto checksums = std::get<0>(t);
	const auto filenames = std::get<1>(t);
	const auto toc       = std::get<2>(t);
	const auto arid      = std::get<3>(t);
	//const auto is_album  = std::get<4>(t); // unused

	const auto total_tracks = checksums.size();

	if (total_tracks == 0)
	{
		throw std::invalid_argument("Missing value: "
				"Need some Checksums to print");
	}

	if (auto track0 = checksums.at(0);
		track0.types().empty() or track0.empty())
	{
		throw std::invalid_argument("Missing value: "
				"Checksums seem to hold no checksums");
	}

	if (!toc and filenames.empty())
	{
		throw std::invalid_argument("Missing value: "
				"Need either TOC data or filenames to print results");
	}

	if (toc && static_cast<uint16_t>(toc->total_tracks()) != total_tracks)
	{
		throw std::invalid_argument("Mismatch: "
				"Checksums for " + std::to_string(total_tracks)
				+ " files/tracks, but TOC specifies "
				+ std::to_string(toc->total_tracks()) + " tracks.");
	}

	if (not (filenames.empty()
				or filenames.size() == total_tracks or filenames.size() == 1))
	{
		throw std::invalid_argument("Mismatch: "
				"Checksums for " + std::to_string(total_tracks)
				+ " files/tracks, but " + std::to_string(filenames.size())
				+ " files.");
	}

	if (not (arid.empty()
		or static_cast<uint16_t>(arid.track_count()) == total_tracks))
	{
		throw std::invalid_argument("Mismatch: "
				"Checksums for " + std::to_string(total_tracks)
				+ " files/tracks, but AccurateRip id specifies "
				+ std::to_string(arid.track_count()) + " tracks.");
	}
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
	const auto checksums = std::get<0>(t);
	const auto filenames = std::get<1>(t);
	const auto toc       = std::get<2>(t);
	//const auto arid      = std::get<3>(t); // TODO Implement printing
	//const auto is_album  = std::get<4>(t);

	const auto types_to_print = ordered_typelist(checksums);

	if (types_to_print.empty())
	{
		throw std::invalid_argument("Missing value: "
				"Checksums seem to hold no checksums");
	}


	// Configure table layout

	TableLayout lyt { 0, 0, label(), track(), offset(), length(), filename() };
	lyt.set_column_delimiter(column_delimiter());

	if (!toc) { lyt.set_offset(false); }
	if (filenames.empty()) { lyt.set_filename(false); }
	// assertion is fulfilled that either not filenames.empty() or non-null toc

	// TODO Use init() instead
	lyt.resize(checksums.size() + label(),
			lyt.total_metadata_columns() + types_to_print.size());
	// assertion is fulfilled that rows >= 1 as well as columns >= 1

	const auto md_offset = lyt.columns_apply_md_settings();
	lyt.columns_apply_cs_settings(types_to_print);
	lyt.set_widths(CELL_TYPE::FILENAME, optimal_width(filenames));


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
						{ cell = filenames.at(row); }
					break;

				case CELL_TYPE::OFFSET   :
					if (offset())
						{ cell = std::to_string(toc->offset(trackno)); }
					break;

				case CELL_TYPE::LENGTH   :
					if (length())
						{ cell = std::to_string((checksums)[row].length()); }
					break;

				case CELL_TYPE::CHECKSUM :
					cstype = types_to_print[col - md_offset];
					cell = lyt.checksum_layout().format(
							checksums[row].get(cstype), lyt.width(col));
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
	const auto checksums = std::get<0>(t);
	const auto filenames = std::get<1>(t);
	const auto toc       = std::get<2>(t);
	//const auto arid      = std::get<3>(t); // unused
	const auto is_album  = std::get<4>(t);

	const auto types_to_print = ordered_typelist(checksums);

	if (types_to_print.empty())
	{
		throw std::invalid_argument("Missing value: "
				"Checksums seem to hold no checksums");
	}

	// Configure table

	TableLayout lyt { 0, 0, label(), track(), offset(), length(), filename() };
	lyt.set_column_delimiter(column_delimiter());

	if (is_album) { lyt.set_track(true); }

	// Deactivate what we cannot access, overriding requested settings.
	if (!toc) { lyt.set_offset(false); }
	if (filenames.empty()) { lyt.set_filename(false); }

	// TODO Use init() instead
	lyt.resize(filename() + offset() + length() + types_to_print.size(),
				checksums.size());

	// Assign row labels

	if (lyt.label())
	{
		int row = lyt.rows_apply_md_settings(is_album);

		for (const auto& type : types_to_print)
		{
			lyt.set_row_label(row, arcstk::checksum::type_name(type));
			++row;
		}
	}

	// Assign column widths: all columns have width 8 and alignment LEFT

	for (std::size_t col = 0; col < lyt.columns(); ++col)
	{
		lyt.set_width(col, 8);
		lyt.set_alignment(col, false);
	}

	// Print table rows

	std::ostringstream o;
	int row = 0;

	if (lyt.filename())
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

	if (lyt.offset())
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

	if (lyt.length())
	{
		if (label()) { lyt.print_label(o, row); }

		int index = 0;
		for (std::size_t col = 0; col < lyt.columns() - 1; ++col, ++index)
		{
			lyt.print_cell(o, col,
				std::to_string(checksums.at(index).length()),
				true);
		}
		lyt.print_cell(o, lyt.columns() - 1,
				std::to_string(checksums.at(index).length()),
				false);

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
				lyt.checksum_layout().format(checksums.at(index).get(type), 8),
				true);
		}

		lyt.print_cell(o, lyt.columns() - 1,
				lyt.checksum_layout().format(checksums.at(index).get(type), 8),
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


void VerifyResultLayout::assertions(const ArgsRefTuple t) const
{
	const auto& checksums = std::get<0>(t);
	const auto& filenames = std::get<1>(t);

	const auto  toc       = std::get<6>(t);
	const auto& arid      = std::get<7>(t);

	const auto total_tracks = checksums.size();

	if (total_tracks == 0)
	{
		throw std::invalid_argument("Missing value: "
				"Need some Checksums to print");
	}

	if (auto track0 = checksums.at(0);
		track0.types().empty() or track0.empty())
	{
		throw std::invalid_argument("Missing value: "
				"Checksums seem to hold no checksums");
	}

	if (!toc and filenames.empty())
	{
		throw std::invalid_argument("Missing value: "
				"Need either TOC data or filenames to print results");
	}

	if (toc && static_cast<uint16_t>(toc->total_tracks()) != total_tracks)
	{
		throw std::invalid_argument("Mismatch: "
				"Checksums for " + std::to_string(total_tracks)
				+ " files/tracks, but TOC specifies "
				+ std::to_string(toc->total_tracks()) + " tracks.");
	}

	if (not (filenames.empty()
			or filenames.size() == total_tracks or filenames.size() == 1))
	{
		throw std::invalid_argument("Mismatch: "
				"Checksums for " + std::to_string(total_tracks)
				+ " files/tracks, but " + std::to_string(filenames.size())
				+ " files.");
	}

	if (not (arid.empty()
			or static_cast<uint16_t>(arid.track_count()) == total_tracks))
	{
		throw std::invalid_argument("Mismatch: "
				"Checksums for " + std::to_string(total_tracks)
				+ " files/tracks, but AccurateRip id specifies "
				+ std::to_string(arid.track_count()) + " tracks.");
	}

	// Specific for verify

	const auto  refsums   = std::get<2>(t);
	const auto  match     = std::get<3>(t);
	const auto  block     = std::get<4>(t);
	//const auto  version   = std::get<5>(t); // unused

	if (refsums.empty())
	{
		throw std::invalid_argument("Missing reference checksums, "
				"nothing to print.");
	}

	if (refsums.size() != total_tracks)
	{
		throw std::invalid_argument("Mismatch: "
				"Reference for " + std::to_string(refsums.size())
				+ " tracks, but Checksums specify "
				+ std::to_string(total_tracks) + " tracks.");
	}

	if (!match)
	{
		throw std::invalid_argument("Missing match information, "
				"nothing to print.");
	}

	if (block < 0)
	{
		throw std::invalid_argument(
			"Index of matching checksum block is negative, nothing to print.");
	}

	if (block > match->total_blocks())
	{
		throw std::invalid_argument("Mismatch: "
				"Match contains no block " + std::to_string(block)
				+ " but contains only "
				+ std::to_string(match->total_blocks()) + " blocks.");
	}
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
	const auto checksums = std::get<0>(t);
	const auto filenames = std::get<1>(t);
	const auto refsums   = std::get<2>(t);
	const auto match     = std::get<3>(t);
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
		if (version)
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
	lyt.set_column_delimiter(column_delimiter());

	// Determine number of rows
	const int total_entries = 1 /* col titles */ + checksums.size();

	if (!toc) { lyt.set_offset(false); }

	// TODO Use init() instead
	lyt.resize(total_entries,
			lyt.total_metadata_columns() + types_to_print.size() + 1);

	const auto md_offset = lyt.columns_apply_md_settings();
	lyt.set_widths(CELL_TYPE::FILENAME, optimal_width(filenames));

	lyt.columns_apply_cs_settings(types_to_print);

	// Print contents

	std::ostringstream out;

	if (label()) { lyt.print_column_titles(out); }

	TYPE cstype = TYPE::ARCS2; // default
	std::string cell{};
	int trackno = 1;

	for (std::size_t row = 0; row < lyt.rows() - 1; ++row, ++trackno)//print row
	{
		for (std::size_t col = 0; col < lyt.columns(); ++col)//print cell
		{
			switch (lyt.type_of(col))
			{
				case CELL_TYPE::TRACK    :
					if (track())
						{ cell = std::to_string(trackno); }
					break;

				case CELL_TYPE::FILENAME :
					if (filename())
						{ cell = filenames.size() > 1
							? filenames.at(row)
							: *filenames.begin(); }
					break;

				case CELL_TYPE::OFFSET   :
					if (offset())
						{ cell = std::to_string(toc->offset(trackno)); }
					break;

				case CELL_TYPE::LENGTH   :
					if (length())
						{ cell = std::to_string(checksums.at(row).length()); }
					break;

				case CELL_TYPE::CHECKSUM : // "Theirs" column
					cell = lyt.checksum_layout().format(refsums[row],
							lyt.width(col));
					break;

				case CELL_TYPE::MATCH    : // "Mine" columns (may be one or two)
					cstype = types_to_print[col - md_offset - 1];
					if (match->track(block, row, cstype == TYPE::ARCS2))
					{
						cell = match_symbol();
					} else
					{
						cell = lyt.checksum_layout().format(
							checksums.at(row).get(cstype),
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

