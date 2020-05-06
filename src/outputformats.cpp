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

using arcsdec::FileFormat;


template <typename E>
constexpr auto to_underlying(E e) noexcept
{
    return static_cast<std::underlying_type_t<E>>(e);
}


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
		flags_ |= (1 << idx); // <= true
	} else
	{
		flags_ &= ~(0 << idx); // <= false
	}
}


bool WithInternalFlags::flag(const int idx) const
{
	return flags_ & (1 << idx);
}


// ARIdFormat


ARIdFormat::ARIdFormat()
	: WithInternalFlags(0xFFFFFFFF) // all flags true
	, lines_()
{
	// empty
}


ARIdFormat::ARIdFormat(const bool &url, const bool &filename,
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
	, lines_()
{
	// empty
}


ARIdFormat::~ARIdFormat() noexcept = default;


bool ARIdFormat::url() const
{
	return flag(0);
}


void ARIdFormat::set_url(const bool url)
{
	this->set_flag(0, url);
}


bool ARIdFormat::filename() const
{
	return flag(1);
}


void ARIdFormat::set_filename(const bool filename)
{
	this->set_flag(1, filename);
}


bool ARIdFormat::track_count() const
{
	return flag(2);
}


void ARIdFormat::set_trackcount(const bool trackcount)
{
	this->set_flag(2, trackcount);
}


bool ARIdFormat::disc_id_1() const
{
	return flag(3);
}


void ARIdFormat::set_disc_id_1(const bool disc_id_1)
{
	this->set_flag(3, disc_id_1);
}


bool ARIdFormat::disc_id_2() const
{
	return flag(4);
}


void ARIdFormat::set_disc_id_2(const bool disc_id_2)
{
	this->set_flag(4, disc_id_2);
}


bool ARIdFormat::cddb_id() const
{
	return flag(5);
}


void ARIdFormat::set_cddb_id(const bool cddb_id)
{
	this->set_flag(5, cddb_id);
}


std::unique_ptr<Lines> ARIdFormat::do_lines()
{
	return std::move(lines_);
}


void ARIdFormat::format(const ARId &id, const std::string &alt_prefix)
{
	lines_ = std::move(this->do_format(id, alt_prefix));
}


// ARIdTableFormat


ARIdTableFormat::ARIdTableFormat()
	: ARIdFormat()
{
	// empty
}


ARIdTableFormat::ARIdTableFormat(const bool &url, const bool &filename,
		const bool &track_count, const bool &disc_id_1, const bool &disc_id_2,
		const bool &cddb_id)
	: ARIdFormat(url, filename, track_count, disc_id_1, disc_id_2, cddb_id)
{
	// empty
}


ARIdTableFormat::~ARIdTableFormat() noexcept = default;


std::unique_ptr<Lines> ARIdTableFormat::do_format(const ARId &id,
			const std::string &alt_prefix) const
{
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
}


// WithARId


WithARId::WithARId()
	: arid_format_(nullptr)
{
	// empty
}


WithARId::WithARId(std::unique_ptr<ARIdFormat> arid_format)
	: arid_format_(std::move(arid_format))
{
	// empty
}


WithARId::~WithARId() noexcept = default;


void WithARId::set_arid_format(std::unique_ptr<ARIdFormat> format)
{
	if (arid_format_)
	{
		arid_format_.reset();
	}

	arid_format_ = std::move(format);
}


ARIdFormat* WithARId::arid_format()
{
	return arid_format_ ? arid_format_.get() : nullptr;
}


// WithMetadataFlagMethods


WithMetadataFlagMethods::WithMetadataFlagMethods(const bool track,
		const bool offset, const bool length, const bool filename)
	: WithInternalFlags(
			0 | track | (offset << 1) | (length << 2) | (filename << 3))
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


std::vector<int32_t> WithMetadataFlagMethods::get_lengths(
		const Checksums &checksums) const
{
	std::vector<int32_t> checksum_lengths{};
	checksum_lengths.reserve(checksums.size());
	for (const auto& checksum : checksums)
	{
		checksum_lengths.push_back(checksum.length());
	}
	return checksum_lengths;
}


int WithMetadataFlagMethods::optimal_width(const std::vector<std::string> &list)
	const
{
	std::size_t width = 0;

	for (const auto& entry : list)
	{
		if (entry.length() > width)
		{
			width = entry.length();
		}
	}

	return width;
}


// ChecksumsResultPrinter


void ChecksumsResultPrinter::out(std::ostream &out, const Checksums &checksums,
		const TOC &toc, const ARId &arid)
{
	this->do_out(out, checksums, toc, arid);
}


void ChecksumsResultPrinter::out(std::ostream &out, const Checksums &checksums,
			const std::vector<std::string> &strings)
{
	this->do_out(out, checksums, strings);
}


// AlbumTableBase


AlbumTableBase::AlbumTableBase(const int rows, const int columns)
	: WithMetadataFlagMethods(true, true, true, true)
	, StringTableBase(rows, columns)
{
	// empty
}


AlbumTableBase::AlbumTableBase(const int rows, const int columns,
			const bool track, const bool offset, const bool length,
			const bool filename)
	: WithMetadataFlagMethods(track, offset, length, filename)
	, StringTableBase(rows, columns)
{
	// empty
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


int AlbumTableBase::convert_from(const AlbumTableBase::COL_TYPE type) const
{
	return to_underlying(type);
}


AlbumTableBase::COL_TYPE AlbumTableBase::convert_to(const int type) const
{
	return static_cast<COL_TYPE>(type);
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


int AlbumTableBase::add_metadata(const std::vector<std::string> &filenames,
		const std::vector<int32_t> &offsets,
		const std::vector<int32_t> &lengths)
{
	int col_idx = 0;

	// The order of the 4 metadata columns is not configurable
	// It is: | track | filename | offset | length |
	// But no column is mandatory.

	if (this->track())
	{
		this->set_title(col_idx, "Track");
		this->set_width(col_idx, this->title(col_idx).length());

		for (std::size_t row_idx = 0; row_idx < filenames.size(); ++row_idx)
		{
			this->update_cell(row_idx, col_idx, std::to_string(row_idx + 1));
		}
		++col_idx;
	}

	if (this->filename())
	{
		this->set_title(col_idx, "File");
		this->set_width(col_idx, optimal_width(filenames));

		for (std::size_t row_idx = 0; row_idx < filenames.size(); ++row_idx)
		{
			this->update_cell(row_idx, col_idx, filenames[row_idx]);
		}
		++col_idx;
	}

	if (this->offset())
	{
		this->set_title(col_idx, "Offset");
		this->set_width(col_idx, 7);

		for (std::size_t row_idx = 0; row_idx < filenames.size(); ++row_idx)
		{
			this->update_cell(row_idx, col_idx,
					std::to_string(offsets[row_idx]));
		}
		++col_idx;
	}

	if (this->length())
	{
		this->set_title(col_idx, "Length");
		this->set_width(col_idx, 7);

		for (std::size_t row_idx = 0; row_idx < filenames.size(); ++row_idx)
		{
			this->update_cell(row_idx, col_idx,
					std::to_string(lengths[row_idx]));
		}

		++col_idx;
	}

	return col_idx;
}


// AlbumChecksumsTableFormat


AlbumChecksumsTableFormat::AlbumChecksumsTableFormat(const int rows,
		const int columns, const bool track, const bool offset,
		const bool length, const bool filename)
	: WithMetadataFlagMethods(track, offset, length, filename)
	, StringTableBase(rows, columns)
	, AlbumTableBase(rows, columns, track, offset, length, filename)
	, hexlayout_()
{
	hexlayout_.set_show_base(false);
	hexlayout_.set_uppercase(true);
}


AlbumChecksumsTableFormat::~AlbumChecksumsTableFormat() = default;


void AlbumChecksumsTableFormat::format(const Checksums &checksums,
		const ARId &arid, const TOC &toc) // TODO prefix!
{
	// ARId
	if (this->arid_format())
	{
		this->arid_format()->format(arid, "");
		//lines_->append(*this->arid_format()->lines());
		// FIXME ARId is missing in output
	}

	// Add filenames, offsets, lengths
	auto md_columns = this->add_metadata(arcstk::toc::get_filenames(toc),
			arcstk::toc::get_offsets(toc),
			get_lengths(checksums));
	this->add_checksums(md_columns, checksums);
}


void AlbumChecksumsTableFormat::format(const Checksums &checksums,
		const std::vector<std::string> &filenames)
{
	// Add filenames and lengths
	auto md_columns = this->add_metadata(filenames, {}, get_lengths(checksums));
	this->add_checksums(md_columns, checksums);
}


void AlbumChecksumsTableFormat::add_checksums(const int start_col,
		const Checksums &checksums)
{
	int col_idx = start_col;

	for (const auto& type : checksums[0].types()) // iterate checksum types
	{
		this->set_title(col_idx, arcstk::checksum::type_name(type));
		this->set_width(col_idx, 8); // TODO Magic number: 32 bit

		for (std::size_t row_idx = 0; row_idx < checksums.size(); ++row_idx)
		{
			this->update_cell(row_idx, col_idx, hexlayout_.format(
						checksums[row_idx].get(type).value(),
						this->width(col_idx)));
		}
		++col_idx;
	}
}


void AlbumChecksumsTableFormat::setup_metadata_cols()
{
	for (std::size_t col = 0; col < columns(); ++col)
	{
		if (type_of(col) == COL_TYPE::TRACK)
		{
			set_title(col, "Track");
			set_width(col, title(col).length());
		} else
		if (type_of(col) == COL_TYPE::FILENAME)
		{
			set_title(col, "Filename");
			//width is added in do_out() with concrete optimal width
		} else
		if (type_of(col) == COL_TYPE::OFFSET)
		{
			set_title(col, "Offset");
			set_width(col, 7);
		} else
		if (type_of(col) == COL_TYPE::LENGTH)
		{
			set_title(col, "Length");
			set_width(col, 7);
		} else
		if (type_of(col) == COL_TYPE::CHECKSUM)
		{
			set_width(col, 8);
		}
	}
}


void AlbumChecksumsTableFormat::do_out(std::ostream &out,
		const Checksums &checksums, const TOC &toc, const ARId &/*arid*/)
{
	const auto& defined_types = arcstk::checksum::types;

	// Assume identical type sets in each track
	const auto& used_types    = checksums[0].types();

	// Construct types_to_print as list of all used types in the order they
	// appear in arcstk::checksum::types
	using cstype = arcstk::checksum::type;
	std::vector<cstype> types_to_print{};
	types_to_print.reserve(defined_types.size());
	const auto absent { used_types.end() };
	std::copy_if(used_types.begin(), used_types.end(),
			std::back_inserter(types_to_print),
			[used_types, absent](const cstype& t)
			{
				return used_types.find(t) != absent;
			});

	resize(1/*titles*/ + checksums.size(),
			total_metadata_columns() + used_types.size());

	// Assign column types to columns

	std::size_t col = 0;

	if (track())    { assign_type(col, COL_TYPE::TRACK);    ++col; }
	if (filename())
	{
		assign_type(col, COL_TYPE::FILENAME);
		set_width(col, optimal_width(arcstk::toc::get_filenames(toc)));
		++col;
	}
	if (offset())   { assign_type(col, COL_TYPE::OFFSET);   ++col; }
	if (length())   { assign_type(col, COL_TYPE::LENGTH);   ++col; }

	const auto md_offset = col;

	auto type_to_print { types_to_print.cbegin() };
	while (col < columns() and type_to_print != types_to_print.cend())
	{
		assign_type(col, COL_TYPE::CHECKSUM);
		set_title(col, arcstk::checksum::type_name(*type_to_print));

		++col;
		++type_to_print;
	}
	setup_metadata_cols();

	// Print Column titles

	for (col = 0; col < columns(); ++col)
	{
		out << std::setw(width(col)) << std::left << title(col)
			<< column_delimiter();
	}
	out << std::endl;

	// Row contents

	using COL_TYPE = AlbumTableBase::COL_TYPE;
	std::string cell{};
	int trackno = 1;;

	for (std::size_t row = 0; row < rows() - 1; ++row, ++trackno) // print row
	{
		for (col = 0; col < columns(); ++col) // print cell
		{
			if (track()    and type_of(col) == COL_TYPE::TRACK)
			{
				cell = std::to_string(trackno);
			} else
			if (filename() and type_of(col) == COL_TYPE::FILENAME)
			{
				cell = toc.filename(trackno);
			} else
			if (offset()   and type_of(col) == COL_TYPE::OFFSET)
			{
				cell = std::to_string(toc.offset(trackno));
			} else
			if (length()   and type_of(col) == COL_TYPE::LENGTH)
			{
				cell = std::to_string(checksums[row].length());
			} else
			{
				auto type = types_to_print[col - md_offset];

				cell = hexlayout_.format(checksums[row].get(type).value(),
						width(col));
			}

			out << std::setw(width(col))
				<< (alignment(col) > 0 ? std::left : std::right)
				<< cell
				<< column_delimiter();
		}
		out << std::endl;
	}
}


void AlbumChecksumsTableFormat::do_out(std::ostream &out,
		const Checksums &checksums,
		const std::vector<std::string> &filenames)
{
	const auto& defined_types = arcstk::checksum::types;

	// Assume identical type sets in each track
	const auto& used_types    = checksums[0].types();

	// Construct types_to_print as list of all used types in the order they
	// appear in arcstk::checksum::types
	using cstype = arcstk::checksum::type;
	std::vector<cstype> types_to_print{};
	types_to_print.reserve(defined_types.size());
	const auto absent { used_types.end() };
	std::copy_if(used_types.begin(), used_types.end(),
			std::back_inserter(types_to_print),
			[used_types, absent](const cstype& t)
			{
				return used_types.find(t) != absent;
			});

	resize(1/*titles*/ + checksums.size(),
			total_metadata_columns() + used_types.size());

	// Assign column types to columns

	std::size_t col = 0;

	//if (track())    { assign_type(col, COL_TYPE::TRACK);    ++col; }
	if (filename())
	{
		assign_type(col, COL_TYPE::FILENAME);
		set_width(col, optimal_width(filenames));
		++col;
	}
	//if (offset())   { assign_type(col, COL_TYPE::OFFSET);   ++col; }
	if (length())   { assign_type(col, COL_TYPE::LENGTH);   ++col; }

	const auto md_offset = col;

	auto type_to_print { types_to_print.cbegin() };
	while (col < columns() and type_to_print != types_to_print.cend())
	{
		assign_type(col, COL_TYPE::CHECKSUM);
		set_title(col, arcstk::checksum::type_name(*type_to_print));

		++col;
		++type_to_print;
	}
	setup_metadata_cols();

	// Print Column titles

	for (col = 0; col < columns(); ++col)
	{
		out << std::setw(width(col)) << std::left << title(col)
			<< column_delimiter();
	}
	out << std::endl;

	// Row contents

	using COL_TYPE = AlbumTableBase::COL_TYPE;
	std::string cell{};
	int trackno = 1;;

	for (std::size_t row = 0; row < rows() - 1; ++row, ++trackno) // print row
	{
		for (col = 0; col < columns(); ++col) // print cell
		{
			//if (track()    and type_of(col) == COL_TYPE::TRACK)
			//{
			//	cell = std::to_string(trackno);
			//} else
			if (filename() and type_of(col) == COL_TYPE::FILENAME)
			{
				cell = filenames.at(row);
			} else
			//if (offset()   and type_of(col) == COL_TYPE::OFFSET)
			//{
			//	cell = std::to_string(toc.offset(trackno));
			//} else
			if (length()   and type_of(col) == COL_TYPE::LENGTH)
			{
				cell = std::to_string(checksums[row].length());
			} else
			{
				auto type = types_to_print[col - md_offset];

				cell = hexlayout_.format(checksums[row].get(type).value(),
						width(col));
			}

			out << std::setw(width(col))
				<< (alignment(col) > 0 ? std::left : std::right)
				<< cell
				<< column_delimiter();
		}
		out << std::endl;
	}
}


std::unique_ptr<Lines> AlbumChecksumsTableFormat::do_lines()
{
	return nullptr;
}


// AlbumMatchTableFormat


AlbumMatchTableFormat::AlbumMatchTableFormat(const int rows,
			const bool track, const bool offset, const bool length,
			const bool filename)
	: WithMetadataFlagMethods(track, offset, length, filename)
	, StringTableBase(rows, (track + offset + length + filename + 2))
	, AlbumTableBase(rows, (track + offset + length + filename + 2),
			track, offset, length, filename)
	, lines_()
	, hexl_()
{
	hexl_.set_uppercase(true);
}


AlbumMatchTableFormat::~AlbumMatchTableFormat() noexcept = default;


void AlbumMatchTableFormat::format(
		const Checksums &checksums, const ARResponse &response,
		const Match &match, const int block, const bool version,
		const ARId &arid, const std::unique_ptr<TOC> toc)
{
	lines_ = std::make_unique<DefaultLines>();

	// ARId
	if (this->arid_format())
	{
		this->arid_format()->format(arid, "");
		lines_->append(*this->arid_format()->lines());
	}

	// Determine number of columns

	// Number of metadata columns
	int md_columns = track() + filename() + offset() + length();

	// Add Names, Offsets, Lengths
	this->add_metadata(arcstk::toc::get_filenames(toc),
			arcstk::toc::get_offsets(toc),
			get_lengths(checksums));

	// Add Mine Checksums, Their Checksums, Match
	this->add_checksums_match(md_columns, checksums, response, match, block,
			version);

	auto table_lines = this->print();
	lines_->append(*table_lines);
}


void AlbumMatchTableFormat::format(
		const Checksums &checksums, const ARResponse &response,
		const Match &match, const int block, const bool version,
		const std::vector<std::string> &filenames)
{
	lines_ = std::make_unique<DefaultLines>();

	// Determine number of columns

	// Number of metadata columns
	int md_columns = track() + filename() + offset() + length();

	// Add filenames and lengths
	this->add_metadata(filenames,
			std::vector<int32_t>(checksums.size()) /* empty */,
			get_lengths(checksums));

	// Add match info
	this->add_checksums_match(md_columns, checksums, response, match, block,
			version);

	auto table_lines = this->print();
	lines_->append(*table_lines);
}


void AlbumMatchTableFormat::add_checksums_match(const int start_col,
		const Checksums &checksums, const ARResponse &response,
		const Match &match, const int block, const bool version)
{
	int col_idx = start_col;

	//this->set_title(col_idx, arcstk::checksum::type_name(cstype));
	this->set_title(col_idx, "Local");
	this->set_width(col_idx, 8);
	//this->register_layout(col_idx, &hexl_);

	this->set_title(col_idx + 1, "Accurate");
	this->set_width(col_idx + 1, 8);

	ARCS_LOG_DEBUG << "Table initialized";
	using type = arcstk::checksum::type;
	auto cstype = version ? type::ARCS2 : type::ARCS1;

	unsigned int row_idx { 0 };
	for (const auto& track : response[block])
	{
		if (row_idx < checksums.size())
		{
			this->update_cell(row_idx, col_idx,
				std::to_string(checksums[row_idx].get(cstype).value()));
		} else
		{
			this->update_cell(row_idx, col_idx, "");
		}

		if (match.track(block, row_idx, version))
		{
			// match
			this->update_cell(row_idx, col_idx + 1, "==");
		} else
		{
			// no match, show AccurateRip value
			this->update_cell(row_idx, col_idx + 1,
					std::to_string(Checksum(track.arcs()).value()));
		}

		++row_idx;
	}
}


std::unique_ptr<Lines> AlbumMatchTableFormat::do_lines()
{
	return std::move(lines_);
}


// ARTripletFormat


ARTripletFormat::ARTripletFormat()
	: lines_()
{
	// empty
}


ARTripletFormat::~ARTripletFormat() noexcept = default;


void ARTripletFormat::format(const unsigned int &track,
		const ARTriplet &triplet)
{
	lines_ = this->do_format(track, triplet);
}


std::unique_ptr<Lines> ARTripletFormat::do_lines()
{
	return std::move(lines_);
}


std::unique_ptr<Lines> ARTripletFormat::do_format(const unsigned int &track,
		const ARTriplet &triplet) const
{
	std::stringstream linestr;

	std::ios_base::fmtflags dec_flags;
	dec_flags  =  linestr.flags();
	dec_flags &= ~linestr.basefield;
	dec_flags &= ~linestr.adjustfield;

	dec_flags |= linestr.right;        // align fields to the right margin
	dec_flags |= linestr.dec;          // set decimal representation


	std::ios_base::fmtflags hex_flags;
	hex_flags  =  linestr.flags();
	hex_flags &= ~linestr.basefield;
	hex_flags &= ~linestr.adjustfield;

	hex_flags |= linestr.right;        // align fields to the right margin
	hex_flags |= linestr.hex;          // set hexadecimal representation
	hex_flags |= linestr.uppercase;    // show digits A-F in uppercase

	linestr.flags(dec_flags);
	linestr << "Track " << std::setw(2) << std::setfill('0') << track << ": ";

	linestr.flags(hex_flags);
	linestr << std::setw(8) << std::setfill('0')
			<< static_cast<unsigned int>(triplet.arcs());

	linestr.flags(dec_flags);
	linestr << " (" << std::setw(2) << std::setfill('0')
			<< static_cast<unsigned int>(triplet.confidence())
			<< ") ";

	linestr.flags(hex_flags);
	linestr << std::setw(8) << std::setfill('0')
			<< static_cast<unsigned int>(triplet.frame450_arcs());

	auto lines { std::make_unique<DefaultLines>() };
	lines->append(linestr.str());

	return lines;
}


// ARBlockFormat


ARBlockFormat::ARBlockFormat()
	: WithARId()
	, lines_()
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


void ARBlockFormat::format(const ARBlock &block)
{
	lines_ = this->do_format(block);
}


ARTripletFormat* ARBlockFormat::triplet_fmt()
{
	return triplet_format_.get();
}


std::unique_ptr<Lines> ARBlockFormat::do_lines()
{
	return std::move(lines_);
}


// ARBlockTableFormat


ARBlockTableFormat::ARBlockTableFormat() = default;


ARBlockTableFormat::~ARBlockTableFormat() noexcept = default;


std::unique_ptr<Lines> ARBlockTableFormat::do_format(const ARBlock &block)
{
	auto lines { std::make_unique<DefaultLines>() };

	if (arid_format())
	{
		arid_format()->format(block.id(), "");
		lines->append(*arid_format()->lines());
	}

	if (triplet_fmt())
	{
		uint32_t trk = 0;
		for (const auto& triplet : block)
		{
			++trk;

			triplet_fmt()->format(trk, triplet);
			lines->append(*triplet_fmt()->lines());
		}
	}

	return lines;
}


// OffsetsFormat


OffsetsFormat::~OffsetsFormat() noexcept = default;


void OffsetsFormat::format(const std::vector<uint32_t> &offsets)
{
	this->do_format(offsets);
}


std::unique_ptr<Lines> OffsetsFormat::do_lines()
{
	return std::move(lines_);
}


void OffsetsFormat::do_format(const std::vector<uint32_t> &offsets)
{
	std::stringstream linestr;

	std::ios_base::fmtflags dec_flags;
	dec_flags  =  linestr.flags();
	dec_flags &= ~linestr.basefield;
	dec_flags &= ~linestr.adjustfield;

	dec_flags |= linestr.right;        // align fields to the right margin
	dec_flags |= linestr.dec;          // set decimal representation

	linestr.flags(dec_flags);

	auto lines { this->lines_.get() };

	for (uint8_t i = 0; i < offsets.size(); ++i)
	{
		linestr << "Track ";

		linestr << std::setw(2) << static_cast<unsigned int>(i + 1);

		linestr << ": ";

		linestr << std::setw(6) << static_cast<unsigned int>(offsets[i]);

		lines->append(linestr.str());

		linestr.str("");
	}
}


// FormatCollector


FormatCollector::FormatCollector()
	: info_ {}
{
	// empty
}


void FormatCollector::add(const FileReaderDescriptor &descriptor)
{
	auto name = descriptor.name();

	auto formats = descriptor.formats();

	std::stringstream desc;
	for (const auto& f : formats)
	{
		desc << arcsdec::name(f) << ",";
	}

	info_.push_back( { name, desc.str(), "-", "-" } );
}


std::vector<std::array<std::string, 4>> FormatCollector::info() const
{
	return info_;
}


// FormatList


FormatList::FormatList(const std::size_t entry_count)
	: StringTableBase(entry_count, 4)
	, curr_row_(0)
{
	int col = -1;

	this->set_title(++col, "Name");
	this->set_title(++col, "Short Desc.");
	this->set_title(++col, "Lib");
	this->set_title(++col, "Version");
}


void FormatList::format(const std::string &fmt_name, const std::string &desc,
		const std::string &lib_name, const std::string &version)
{
	this->add_data(fmt_name, desc, lib_name, version);
}


void FormatList::add_data(const std::string &fmt_name, const std::string &desc,
		const std::string &lib_name, const std::string &version)
{
	int col = -1;

	this->update_cell(curr_row_, ++col, fmt_name);
	this->update_cell(curr_row_, ++col, desc);
	this->update_cell(curr_row_, ++col, lib_name);
	this->update_cell(curr_row_, ++col, version);

	++curr_row_;
}


std::unique_ptr<Lines> FormatList::do_lines()
{
	return this->print();
}

