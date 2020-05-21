#ifndef __ARCSTOOLS_PRINTERS_HPP__
#include "printers.hpp"
#endif

#include <algorithm>  // for max, copy_if
#include <iomanip>    // for operator<<, setw, setfill
#include <iostream>   // for ostream, operator<<, basic_ostream, endl, size_t
#include <iterator>   // for back_insert_iterator, back_inserter
#include <set>        // for operator!=, set, set<>::const_iterator
#include <string>     // for string, operator<<, to_string, char_traits, ope...
#include <vector>     // for vector, allocator

#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>
#endif

#ifndef __ARCSTOOLS_FORMAT_HPP__
#include "format.hpp"
#endif


using arcstk::ARId;
using arcstk::Checksum;
using arcstk::TOC;


// ARIdPrinter


void ARIdPrinter::out(std::ostream &out, const ARId &arid,
		const std::string &prefix)
{
	const auto flags = out.flags();
	this->do_out(out, arid, prefix);
	out.flags(flags);
}


// ARTripletPrinter


void ARTripletPrinter::out(std::ostream &out, const int track,
		const ARTriplet &triplet)
{
	const auto flags = out.flags();
	this->do_out(out, track, triplet);
	out.flags(flags);
}


// ARBlockPrinter


void ARBlockPrinter::out(std::ostream &out, const ARBlock &block)
{
	const auto flags = out.flags();
	this->do_out(out, block);
	out.flags(flags);
}


// ChecksumsResultPrinter


void ChecksumsResultPrinter::out(std::ostream &out, const Checksums &checksums,
		const std::vector<std::string> &filenames,
		const TOC *toc, const ARId &arid)
{
	const auto flags = out.flags();
	this->do_out(out, checksums, filenames, toc, arid);
	out.flags(flags);
}


// MatchResultPrinter


void MatchResultPrinter::out(std::ostream &out, const Checksums &checksums,
			const std::vector<std::string> &filenames,
			const ARResponse &response,
			const Match &match, const int block, const bool version,
			const TOC *toc, const ARId &arid)
{
	const auto flags = out.flags();
	this->do_out(out, checksums, filenames, response, match, block, version,
			toc, arid);
	out.flags(flags);
}


// AlbumTableBase


AlbumTableBase::AlbumTableBase(const int rows, const int columns)
	: StringTableLayout(rows, columns)
	, WithMetadataFlagMethods(true, true, true, true)
{
	// empty
}


AlbumTableBase::AlbumTableBase(const int rows, const int columns,
			const bool show_track, const bool show_offset,
			const bool show_length, const bool show_filename)
	: StringTableLayout(rows, columns)
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


// ARIdTableFormat


ARIdTableFormat::ARIdTableFormat(const bool &url, const bool &filename,
		const bool &track_count, const bool &disc_id_1, const bool &disc_id_2,
		const bool &cddb_id)
	: ARIdLayout(url, filename, track_count, disc_id_1, disc_id_2, cddb_id)
	, StringTableLayout(
			url + filename + track_count + disc_id_1 + disc_id_2 + cddb_id,
			only_one_flag() ? 1 : 2 // one or two columns
		)
	, row_labels_ { "URL", "Filename", "Tracks", "ID1", "ID2", "CDDB ID" }
	, show_flags_ { ARID_FLAG::URL, ARID_FLAG::FILENAME, ARID_FLAG::TRACKS,
		ARID_FLAG::ID1, ARID_FLAG::ID2, ARID_FLAG::CDDBID }
{
	this->init(rows(), columns());
}


ARIdTableFormat::~ARIdTableFormat() noexcept = default;


void ARIdTableFormat::init(const int /* rows */, const int /* cols */)
{
	set_alignment(0, -1);

	if (columns() > 1) // If there is a label column, its width is known
	{
		set_width(0, optimal_width(row_labels_) + 2 /* colon + 1 WS */);
	}
}


std::string ARIdTableFormat::do_format(const ARId &id,
			const std::string &alt_prefix) const
{
	if (no_flags()) // return ARId as default
	{
		return id.to_string();
	}

	const bool label_requested = columns() > 1;

	std::stringstream stream;
	stream << std::left;

	HexLayout hex;
	hex.set_uppercase(true);

	std::string value;

	for (const auto& sflag : show_flags_)
	{
		if (not flag(to_underlying(sflag))) { continue; }

		if (label_requested)
		{
			if (!stream.str().empty()) { stream << std::endl; }
			print_label(stream, sflag);
		}

		switch (sflag)
		{
			case ARID_FLAG::URL:
				value = id.url();
				if (not alt_prefix.empty())
				{
					value.replace(0, id.prefix().length(), alt_prefix);
					// FIXME If alt_prefix does not end with '/' ?
				}
				break;
			case ARID_FLAG::FILENAME:
				value = id.filename();
				break;
			case ARID_FLAG::TRACKS:
				value = std::to_string(id.track_count());
				break;
			case ARID_FLAG::ID1:
				value = hex.format(id.disc_id_1(), 8);
				break;
			case ARID_FLAG::ID2:
				value = hex.format(id.disc_id_2(), 8);
				break;
			case ARID_FLAG::CDDBID:
				value = hex.format(id.cddb_id(), 8);
				break;
			default:
				break;
		}

		stream << std::setw(value.length()) << value;
	}

	return stream.str();
}


void ARIdTableFormat::do_out(std::ostream &out, const ARId &id,
		const std::string &alt_prefix)
{
	out << format(id, alt_prefix) << std::endl;
}


void ARIdTableFormat::print_label(std::ostream &out,
		const ARIdLayout::ARID_FLAG flag) const
{
	out << std::setw(width(0)) << row_labels_[to_underlying(flag)] + ":";
}


// AlbumChecksumsTableFormat


AlbumChecksumsTableFormat::AlbumChecksumsTableFormat(const int rows,
		const int columns, const bool show_track, const bool show_offset,
		const bool show_length, const bool show_filename)
	: AlbumTableBase(rows, columns,
			show_track, show_offset, show_length, show_filename)
	, hexlayout_()
{
	this->init(rows, columns);
}


AlbumChecksumsTableFormat::~AlbumChecksumsTableFormat() noexcept
= default;


void AlbumChecksumsTableFormat::init(const int /* rows */, const int /* cols */)
{
	hexlayout_.set_show_base(false);
	hexlayout_.set_uppercase(true);
}


int AlbumChecksumsTableFormat::columns_apply_cs_settings(
		const std::vector<arcstk::checksum::type> &types)
{
	// Type each column on the "right" as 'CHECKSUM' and apply defaults
	std::size_t col = total_metadata_columns();
	auto type_to_print = types.cbegin();
	while (col < columns() and type_to_print != types.cend())
	{
		assign_type(col, COL_TYPE::CHECKSUM);
		set_title(col, arcstk::checksum::type_name(*type_to_print));
		set_width(col, default_width(COL_TYPE::CHECKSUM));

		++col;
		++type_to_print;
	}

	return col - total_metadata_columns();
}


void AlbumChecksumsTableFormat::do_out(std::ostream &out,
		const Checksums &checksums,
		const std::vector<std::string> &filenames,
		const TOC *toc, const ARId &/*arid*/)
{
	auto types_to_print = typelist(checksums);

	resize(1/*titles*/ + checksums.size(),
			total_metadata_columns() + types_to_print.size());

	// Apply column settings

	const auto md_offset = columns_apply_settings();
	columns_apply_cs_settings(types_to_print);
	set_widths(COL_TYPE::FILENAME, optimal_width(filenames));

	if (!toc) { set_offset(false); }

	// Print table

	print_column_titles(out);

	arcstk::checksum::type cstype = arcstk::checksum::type::ARCS2; // default
	std::string cell{};
	int trackno = 1;;

	for (std::size_t row = 0; row < rows() - 1; ++row, ++trackno) // print row
	{
		for (std::size_t col = 0; col < columns(); ++col) // print cell
		{
			switch (type_of(col))
			{
				case COL_TYPE::TRACK    :
					if (track())
						{ cell = std::to_string(trackno); }
					break;

				case COL_TYPE::FILENAME :
					if (filename())
						{ cell = filenames.at(row); }
					break;

				case COL_TYPE::OFFSET   :
					if (offset())
						{ cell = std::to_string(toc->offset(trackno)); }
					break;

				case COL_TYPE::LENGTH   :
					if (length())
						{ cell = std::to_string(checksums[row].length()); }
					break;

				case COL_TYPE::CHECKSUM :
					cstype = types_to_print[col - md_offset];
					cell = hexlayout_.format(
							checksums[row].get(cstype).value(),
							width(col));
					break;

				case COL_TYPE::MATCH    : break;
			}

			out << std::setw(width(col))
				<< (alignment(col) > 0 ? std::left : std::right)
				<< cell
				<< column_delimiter();
		}
		out << std::endl;
	}
}


// AlbumMatchTableFormat


AlbumMatchTableFormat::AlbumMatchTableFormat(const int rows,
			const bool show_track,
			const bool show_offset,
			const bool show_length,
			const bool show_filename)
	: AlbumTableBase(rows,
			(show_track + show_offset + show_length + show_filename + 2),
			show_track, show_offset, show_length, show_filename)
	, hexlayout_()
{
	this->init(rows, this->columns());
}


AlbumMatchTableFormat::~AlbumMatchTableFormat() noexcept = default;


void AlbumMatchTableFormat::init(const int /* rows */, const int /* cols */)
{
	hexlayout_.set_show_base(false);
	hexlayout_.set_uppercase(true);
}


int AlbumMatchTableFormat::columns_apply_cs_settings(
		const std::vector<arcstk::checksum::type> &types)
{
	// Type each column on the "right" as 'CHECKSUM' and 'MATCH' in
	// alternating order and apply defaults

	auto type_to_print = types.cbegin();
	std::size_t col_idx = total_metadata_columns();

	while (col_idx < columns() and type_to_print != types.cend())
	{
		assign_type(col_idx, COL_TYPE::CHECKSUM);
		set_title(col_idx, arcstk::checksum::type_name(*type_to_print));
		set_width(col_idx, default_width(COL_TYPE::CHECKSUM));

		++col_idx;

		assign_type(col_idx, COL_TYPE::MATCH);
		set_title(col_idx, default_title(COL_TYPE::MATCH));
		set_width(col_idx, default_width(COL_TYPE::MATCH));

		++col_idx;

		++type_to_print;
	}

	return col_idx - total_metadata_columns();
}


void AlbumMatchTableFormat::do_out(std::ostream &out,
			const Checksums &checksums,
			const std::vector<std::string> &filenames,
			const ARResponse &response,
			const Match &match, const int best, const bool version,
			const TOC *toc, const ARId &/*arid*/)
{
	using TYPE = arcstk::checksum::type;

	std::vector<TYPE> types_to_print = { version ? TYPE::ARCS2 : TYPE::ARCS1 };

	const int total_entries = 1/*titles*/ +
		(toc ? checksums.size()
			: std::max(static_cast<int>(checksums.size()),
				response.tracks_per_block()));

	// Configure table

	resize(total_entries, total_metadata_columns() + types_to_print.size() * 2);

	const auto md_offset = columns_apply_settings();
	columns_apply_cs_settings(types_to_print);
	set_widths(COL_TYPE::FILENAME, optimal_width(filenames));

	if (!toc) { set_offset(false); }

	// Print contents

	print_column_titles(out);

	using COL_TYPE = AlbumTableBase::COL_TYPE;
	TYPE cstype = TYPE::ARCS2; // default
	std::string cell{};
	int trackno = 1;
	int m_columns = 0;

	for (std::size_t row = 0; row < rows() - 1; ++row, ++trackno) // print row
	{
		for (std::size_t col = 0; col < columns(); ++col) // print cell
		{
			switch (type_of(col))
			{
				case COL_TYPE::TRACK    :
					if (track())
						{ cell = std::to_string(trackno); }
					break;

				case COL_TYPE::FILENAME :
					if (filename())
						{ cell = filenames.at(row); }
					break;

				case COL_TYPE::OFFSET   :
					if (offset())
						{ cell = std::to_string(toc->offset(trackno)); }
					break;

				case COL_TYPE::LENGTH   :
					if (length())
						{ cell = std::to_string(checksums[row].length()); }
					break;

				case COL_TYPE::CHECKSUM :
					cstype = types_to_print[col - md_offset - m_columns];
					cell = hexlayout_.format(
							checksums[row].get(cstype).value(),
							width(col));
					break;

				case COL_TYPE::MATCH    :
					cstype = types_to_print[col - md_offset - m_columns];
					++ m_columns;
					if (match.track(best, row, version))
					{
						cell = "   ==   ";
					} else
					{
						cell = hexlayout_.format(response[best].at(row).arcs(),
							width(col));
					}
					break;
			}

			out << std::setw(width(col))
				<< (alignment(col) > 0 ? std::left : std::right)
				<< cell
				<< column_delimiter();
		}

		out << std::endl;
		m_columns = 0;
	}
}


// ARTripletFormat


void ARTripletFormat::do_out(std::ostream &out, const int track,
		const ARTriplet &triplet)
{
	HexLayout hex;
	hex.set_show_base(false);
	hex.set_uppercase(true);

	const int width_arcs = 8;
	const int width_conf = 2;

	out << "Track " << std::setw(2) << std::setfill('0') << track << ": ";

	if (triplet.arcs_valid())
	{
		out << std::setw(width_arcs)
			<< hex.format(triplet.arcs(), width_arcs);
	} else
	{
		out << std::setw(width_arcs) << "????????";
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
			<< hex.format(triplet.frame450_arcs(), width_arcs);
	} else
	{
		out << std::setw(width_arcs) << "????????";
	}

	out << std::endl;
}


// FormatList


FormatList::FormatList(const std::size_t entries)
	: StringTable(entries, 4)
	, curr_row_(0)
{
	this->init(static_cast<int>(rows()), static_cast<int>(columns()));
}


void FormatList::init(const int /* rows */, const int /* cols */)
{
	int col = -1;

	set_title(++col, "Name");
	set_width(col, title(col).length() + 6);
	set_alignment(col, true);

	set_title(++col, "Short Desc.");
	set_width(col, title(col).length() + 4);
	set_alignment(col, true);

	set_title(++col, "Lib");
	set_width(col, title(col).length() + 5);
	set_alignment(col, true);

	set_title(++col, "Version");
	set_width(col, title(col).length());
	set_alignment(col, true);
}


void FormatList::append_line(const std::string &fmt_name,
		const std::string &desc,
		const std::string &lib_name,
		const std::string &version)
{
	int col = -1;

	this->update_cell(curr_row_, ++col, fmt_name);
	this->update_cell(curr_row_, ++col, desc);
	this->update_cell(curr_row_, ++col, lib_name);
	this->update_cell(curr_row_, ++col, version);

	++curr_row_;
}
