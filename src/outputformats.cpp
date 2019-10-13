#ifndef __ARCSTOOLS_OUTPUTFORMATS_HPP__
#include "outputformats.hpp"
#endif

#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
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
			| (filename << 1)
			| (track_count << 2)
			| (disc_id_1 << 3)
			| (disc_id_2 << 4)
			| (cddb_id << 5)
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


void AlbumTableBase::add_data(const TOC &toc)
{
	std::vector<uint32_t> parsed_lengths{};
	parsed_lengths.reserve(toc.track_count());

	for (unsigned int row_idx = 0; row_idx < toc.track_count(); ++row_idx)
	{
		parsed_lengths.push_back(toc.parsed_length(row_idx+1));
	}

	this->add_data(arcstk::toc::get_filenames(toc),
		arcstk::toc::get_offsets(toc), parsed_lengths);
}


void AlbumTableBase::add_data(const std::vector<std::string> &filenames,
		const std::vector<uint32_t> &offsets,
		const std::vector<uint32_t> &lengths)
{
	int col_idx = 0;

	if (this->track())
	{
		this->set_column_name(col_idx, "Track");
		this->set_column_width(col_idx, this->column_name(col_idx).length());

		for (unsigned int row_idx = 0; row_idx < filenames.size(); ++row_idx)
		{
			this->update_cell(row_idx, col_idx, row_idx + 1);
		}
		++col_idx;
	}

	if (this->filename())
	{
		this->set_column_name(col_idx, "File");
		unsigned int col_width = 0;

		// Adjust column width to longest filename
		for (const auto& file : filenames)
		{
			if (file.length() > col_width)
			{
				col_width = file.length();
			}
		}

		this->set_column_width(col_idx, col_width);

		for (unsigned int row_idx = 0; row_idx < filenames.size(); ++row_idx)
		{
			this->update_cell(row_idx, col_idx, filenames[row_idx]);
		}
		++col_idx;
	}

	if (this->offset())
	{
		this->set_column_name(col_idx, "Offset");
		this->set_column_width(col_idx, 7);

		for (unsigned int row_idx = 0; row_idx < filenames.size(); ++row_idx)
		{
			this->update_cell(row_idx, col_idx, offsets[row_idx]);
		}
		++col_idx;
	}

	if (this->length())
	{
		this->set_column_name(col_idx, "Length");
		this->set_column_width(col_idx, 7);

		for (unsigned int row_idx = 0; row_idx < filenames.size(); ++row_idx)
		{
			this->update_cell(row_idx, col_idx, lengths[row_idx]);
		}

		// Commented out: print missing last length non-numerical
		//auto last_length = toc.parsed_length(toc.track_count());
		//if (last_length)
		//{
		//	this->update_cell(toc.track_count()-1, col_idx, last_length);
		//}// else
		//{
		//	this->update_cell(toc.track_count()-1, col_idx, "---");
		//}
		++col_idx;
	}
}


// AlbumChecksumsTableFormat


AlbumChecksumsTableFormat::AlbumChecksumsTableFormat(const int rows,
		const int columns, const bool track, const bool offset,
		const bool length, const bool filename)
	: WithMetadataFlagMethods(track, offset, length, filename)
	, StringTableBase(rows, columns)
	, AlbumTableBase(rows, columns, track, offset, length, filename)
	, lines_()
	, hexl_()
{
	hexl_.set_uppercase(true);
}


AlbumChecksumsTableFormat::~AlbumChecksumsTableFormat() = default;


void AlbumChecksumsTableFormat::format(const Checksums &checksums,
		const ARId &arid, const TOC &toc) // TODO prefix!
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
	int md_columns = this->track() + this->filename() + this->offset()
		+ this->length();

	this->validate_table_dimensions(checksums.size() /*rows*/,
			md_columns + checksums[0].keys().size()  /*columns*/);

	// TOC
	this->add_data(toc);

	// Checksums
	this->add_checksums(md_columns, checksums);

	auto table_lines = this->print();
	lines_->append(*table_lines);
}


void AlbumChecksumsTableFormat::format(const Checksums &checksums,
		const std::vector<std::string> &filenames)
{
	lines_ = std::make_unique<DefaultLines>();

	// Determine number of columns

	// Number of metadata columns
	int md_columns = this->filename() + this->length();

	this->validate_table_dimensions(checksums.size() /*rows*/,
			md_columns + checksums[0].keys().size()  /*columns*/);


	// Add filenames and lengths
	std::vector<uint32_t> actual_lengths{};
	actual_lengths.reserve(checksums.size());

	for (const auto& checksum : checksums)
	{
		actual_lengths.push_back(checksum.length());
	}
	this->add_data(filenames, std::vector<uint32_t>(), actual_lengths);


	// Checksums
	this->add_checksums(md_columns, checksums);

	auto table_lines = this->print();
	lines_->append(*table_lines);
}


void AlbumChecksumsTableFormat::add_checksums(const int start_col,
		const Checksums &checksums)
{
	int col_idx = start_col;
	for (const auto& type : checksums[0].keys()) // iterate checksum types
	{
		this->set_column_name(col_idx, arcstk::checksum::type_name(type));
		this->set_column_width(col_idx, 8);
		this->register_layout(col_idx, &hexl_);

		// produce rows (checksums)
		for (unsigned int row_idx = 0; row_idx < checksums.size(); ++row_idx)
		{
			this->update_cell(row_idx, col_idx,
					checksums[row_idx].get(type).value());
		}
		++col_idx;
	}
}


std::unique_ptr<Lines> AlbumChecksumsTableFormat::do_lines()
{
	return std::move(lines_);
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
	int md_columns = this->track() + this->filename() + this->offset()
		+ this->length();

	this->validate_table_dimensions(checksums.size() /*rows*/,
			md_columns + 2  /*columns*/);

	// Add table content
	this->add_data(*toc);
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
	int md_columns = this->track() + this->filename() + this->offset()
		+ this->length();

	this->validate_table_dimensions(checksums.size() /*rows*/,
			md_columns + 2  /*columns*/);

	// Add filenames and lengths
	std::vector<uint32_t> actual_lengths{};
	actual_lengths.reserve(checksums.size());

	for (const auto& checksum : checksums)
	{
		actual_lengths.push_back(checksum.length());
	}

	// Add table content
	this->add_data(filenames,
			std::vector<uint32_t>(checksums.size()) /* empty */,
			actual_lengths);

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

	//this->set_column_name(col_idx, arcstk::checksum::type_name(cstype));
	this->set_column_name(col_idx, "Local");
	this->set_column_width(col_idx, 8);
	this->register_layout(col_idx, &hexl_);

	this->set_column_name(col_idx + 1, "Accurate");
	this->set_column_width(col_idx + 1, 8);

	ARCS_LOG_DEBUG << "Table initialized";
	using type = arcstk::checksum::type;
	auto cstype = version ? type::ARCS2 : type::ARCS1;

	unsigned int row_idx { 0 };
	for (const auto& track : response[block])
	{
		if (row_idx < checksums.size())
		{
			this->update_cell(row_idx, col_idx,
				checksums[row_idx].get(cstype).value());
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
					Checksum(track.arcs()).value());
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

