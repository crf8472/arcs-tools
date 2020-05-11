#include <type_traits>
#ifndef __ARCSTOOLS_OUTPUTFORMATS_HPP__
#include "outputformats.hpp"
#endif

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#ifndef __ARCSTOOLS_FORMAT_HPP__
#include "format.hpp"
#endif


using arcstk::ARId;
using arcstk::Checksum;
using arcstk::TOC;


template <typename E>
constexpr auto to_underlying(E e) noexcept
{
    return static_cast<std::underlying_type_t<E>>(e);
}


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


ARIdTableFormat::ARIdTableFormat()
	: ARIdLayout()
	, StringTableLayout(0, 0)
{
	this->init(0, 0);
}


ARIdTableFormat::ARIdTableFormat(const bool &url, const bool &filename,
		const bool &track_count, const bool &disc_id_1, const bool &disc_id_2,
		const bool &cddb_id)
	: ARIdLayout(url, filename, track_count, disc_id_1, disc_id_2, cddb_id)
	, StringTableLayout(0, 0)
{
	this->init(0, 0);
}


ARIdTableFormat::~ARIdTableFormat() noexcept = default;


void ARIdTableFormat::init(const int /* rows */, const int /* cols */)
{
	// empty
}


std::string ARIdTableFormat::do_format(const ARId &id,
			const std::string &/*alt_prefix*/) const
{
	return id.to_string();
}


void ARIdTableFormat::do_out(std::ostream &out, const ARId &id,
		const std::string &alt_prefix)
{
	out << format(id, alt_prefix) << std::endl;
	/*
	std::ios_base::fmtflags dec_flags;
	dec_flags  =  std::cout.flags();
	dec_flags &= ~std::cout.basefield;
	dec_flags &= ~std::cout.adjustfield;

	dec_flags |= std::cout.right;        // align fields to the right margin
	dec_flags |= std::cout.dec;          // set decimal representation


	std::ios_base::fmtflags hex_flags;
	hex_flags  =  std::cout.flags();
	hex_flags &= ~std::cout.basefield;
	hex_flags &= ~std::cout.adjustfield;

	hex_flags |= std::cout.right;        // align fields to the right margin
	hex_flags |= std::cout.hex;          // set hexadecimal representation
	hex_flags |= std::cout.uppercase;    // show digits A-F in uppercase


	std::unique_ptr<Lines> lines { std::make_unique<DefaultLines>() };

	std::stringstream linestr;

	if (url())
	{
		const bool url_only = not (filename() or track_count() or disc_id_1()
			or disc_id_2() or cddb_id());

		linestr.flags(dec_flags);

		if (not url_only) { linestr << "URL:       "; }

		if (alt_prefix.empty())
		{
			linestr << id.url();
		} else
		{
			std::string modified_url { id.url() };
			modified_url.replace(0, id.prefix().length(), alt_prefix);

			linestr << modified_url;
		}

		lines->append(linestr.str());
		linestr.str("");
	}

	if (filename())
	{
		const bool filename_only = not (url() or track_count() or disc_id_1()
			or disc_id_2() or cddb_id());

		linestr.flags(dec_flags);

		if (not filename_only) { linestr << "Filename:  "; }

		linestr << id.filename();

		lines->append(linestr.str());
		linestr.str("");
	}

	if (track_count())
	{
		linestr.flags(dec_flags);

		const bool tracks_only = not (url() or filename() or disc_id_1()
			or disc_id_2() or cddb_id());

		if (not tracks_only)
		{
			linestr << "Tracks:    " << std::setw(2) << std::setfill('0')
				<< static_cast<unsigned int>(id.track_count());
		} else
		{
			linestr << static_cast<unsigned int>(id.track_count());
		}

		lines->append(linestr.str());
		linestr.str("");
	}

	if (disc_id_1())
	{
		linestr.flags(hex_flags);

		const bool id1_only = not (url() or filename() or track_count()
			or disc_id_2() or cddb_id());

		if (not id1_only)
		{
			linestr << "Disc id 1: " << std::setw(8) << std::setfill('0')
				<< static_cast<unsigned int>(id.disc_id_1());
		} else
		{
			linestr << static_cast<unsigned int>(id.disc_id_1());
		}

		lines->append(linestr.str());
		linestr.str("");
	}

	if (disc_id_2())
	{
		linestr.flags(hex_flags);

		const bool id2_only = not (url() or filename() or track_count()
			or disc_id_1() or cddb_id());

		if (not id2_only)
		{
			linestr << "Disc id 2: " << std::setw(8) << std::setfill('0')
				<< static_cast<unsigned int>(id.disc_id_2());
		} else
		{
			linestr << static_cast<unsigned int>(id.disc_id_2());
		}

		lines->append(linestr.str());
		linestr.str("");
	}

	if (cddb_id())
	{
		linestr.flags(hex_flags);

		const bool cddb_only = not (url() or filename() or track_count()
			or disc_id_1() or disc_id_2());

		if (not cddb_only)
		{
			linestr << "CDDB id:   " << std::setw(8) << std::setfill('0')
				<< static_cast<unsigned int>(id.cddb_id());
		} else
		{
			linestr << static_cast<unsigned int>(id.cddb_id());
		}

		lines->append(linestr.str());
		linestr.str("");
	}

	return lines;
	*/
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


AlbumChecksumsTableFormat::~AlbumChecksumsTableFormat() = default;


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

	out << "Track " << std::setw(2) << std::setfill('0') << track << ": ";

	out << std::setw(8) << std::setfill('0') << hex.format(triplet.arcs(), 8);

	out << " (" << std::setw(2) << std::setfill('0')
			<< static_cast<unsigned int>(triplet.confidence())
			<< ") ";

	out << std::setw(8) << std::setfill('0')
		<< hex.format(triplet.frame450_arcs(), 8);

	out << std::endl;
}


// ARBlockFormat


ARBlockFormat::ARBlockFormat()
	: WithARId()
	, triplet_format_(nullptr)
{
	// empty
}


ARBlockFormat::~ARBlockFormat() noexcept = default;


void ARBlockFormat::set_triplet_format(std::unique_ptr<ARTripletFormat> format)
{
	if (triplet_format_)
	{
		triplet_format_.reset();
	}

	triplet_format_ = std::move(format);
}


const ARTripletFormat& ARBlockFormat::triplet_format() const
{
	return *triplet_format_;
}


ARTripletFormat* ARBlockFormat::triplet_fmt()
{
	return triplet_format_.get();
}


// ARBlockTableFormat


void ARBlockTableFormat::do_out(std::ostream &out, const ARBlock &block)
{
	if (arid_layout())
	{
		auto arid = arid_layout()->format(block.id(), "");
		out << arid;
	}

	if (triplet_fmt())
	{
		uint32_t trk = 0;
		for (const auto& triplet : block)
		{
			++trk;

			triplet_fmt()->out(out, trk, triplet);
		}
	}
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

