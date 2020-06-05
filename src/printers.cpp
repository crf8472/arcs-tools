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

#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#include "layouts.hpp"
#endif

namespace arcsapp
{

using arcstk::ARId;
using arcstk::Checksum;
using arcstk::TOC;

namespace
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

} // namespace


// ARTripletFormat


ARTripletFormat::ARTripletFormat()
	: Print<int, ARTriplet> { 0, ARTriplet() }
{
	// empty
}


void ARTripletFormat::do_out(std::ostream &out,
		const std::tuple<int, ARTriplet> &t)
{
	auto track   = std::get<0>(t);
	auto triplet = std::get<1>(t);

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


// ARIdTableFormat


ARIdTableFormat::ARIdTableFormat(const ARId &id, const std::string &alt_prefix,
		const bool &url, const bool &filename,
		const bool &track_count, const bool &disc_id_1, const bool &disc_id_2,
		const bool &cddb_id)
	: ARIdLayout(url, filename, track_count, disc_id_1, disc_id_2, cddb_id)
	, StringTableStructure(
			url + filename + track_count + disc_id_1 + disc_id_2 + cddb_id,
			only_one_flag() ? 1 : 2 // one or two columns
		)
	, Print<ARId, std::string>(ARId(id), std::string(alt_prefix)) // FIXME Copies to remove constness
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


void ARIdTableFormat::do_out(std::ostream &o,
		const std::tuple<ARId, std::string> &t)
{
	o << format(std::get<0>(t), std::get<1>(t)) << std::endl;
}


void ARIdTableFormat::print_label(std::ostream &out,
		const ARIdLayout::ARID_FLAG flag) const
{
	out << std::setw(width(0)) << row_labels_[to_underlying(flag)] + ":";
}


// AlbumChecksumsTableFormat


AlbumChecksumsTableFormat::AlbumChecksumsTableFormat(
		const bool show_labels,
		const bool show_track,
		const bool show_offset,
		const bool show_length,
		const bool show_filename,
		const std::string &coldelim)
	: TypedColsTableBase(
			0, 0, show_labels, show_track, show_offset, show_length,
			show_filename)
	, ChecksumsResultPrinter(                          /* TODO Use EmptyARId */
			nullptr, std::vector<std::string>{}, nullptr, ARId{0,0,0,0})
{
	this->init(0, 0); // do_out() does resize() anyway!
	this->set_column_delimiter(coldelim);
}


AlbumChecksumsTableFormat::~AlbumChecksumsTableFormat() noexcept
= default;


void AlbumChecksumsTableFormat::init(const int /* rows */, const int /* cols */)
{
	// empty
}


int AlbumChecksumsTableFormat::columns_apply_cs_settings(
		const std::vector<arcstk::checksum::type> &types)
{
	// Type each column on the "right" as 'CHECKSUM' and apply defaults
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


void AlbumChecksumsTableFormat::do_out(std::ostream &out,
		const std::tuple<Checksums*, std::vector<std::string>, TOC*, ARId> &t)
{
	auto checksums = std::get<0>(t);
	auto filenames = std::get<1>(t);
	auto toc = std::get<2>(t);

	auto types_to_print = ordered_typelist(*checksums);

	resize(1/*titles*/ + checksums->size(),
			total_metadata_columns() + types_to_print.size());

	// Apply column settings

	const auto md_offset = columns_apply_md_settings();
	columns_apply_cs_settings(types_to_print);
	set_widths(CELL_TYPE::FILENAME, optimal_width(filenames));

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
						{ cell = std::to_string((*checksums)[row].length()); }
					break;

				case CELL_TYPE::CHECKSUM :
					cstype = types_to_print[col - md_offset];
					cell = checksum_layout().format(
							(*checksums)[row].get(cstype).value(),
							width(col));
					break;

				case CELL_TYPE::MATCH    : break;
			}

			out << std::setw(width(col))
				<< (alignment(col) > 0 ? std::left : std::right)
				<< cell;

			if (col < columns() - 1) // No delimiter on rightmost column
			{
				out << column_delimiter();
			}
		}
		out << std::endl;
	}
}


// AlbumTracksTableFormat


AlbumTracksTableFormat::AlbumTracksTableFormat(const bool label,
		const bool track, const bool offset, const bool length,
		const bool filename, const std::string &coldelim)
	: TypedRowsTableBase(0, 0, label, track, offset, length, filename)
	, ChecksumsResultPrinter(                          /* TODO Use EmptyARId */
			nullptr, std::vector<std::string>{}, nullptr, ARId{0,0,0,0})
{
	this->init(0, 0); // do_out() does resize() anyway!
	this->set_column_delimiter(coldelim);
}


void AlbumTracksTableFormat::init(const int /* rows */, const int /* cols */)
{
	// empty
}


void AlbumTracksTableFormat::do_out(std::ostream &o,
		const std::tuple<Checksums*, std::vector<std::string>, TOC*, ARId> &t)
{
	auto checksums = std::get<0>(t);
	auto toc = std::get<2>(t);
	auto filenames = std::get<1>(t);

	if (!toc)
	{
		set_offset(false);

		if (filenames.size() < checksums->size())
		{
			throw std::runtime_error("Too less filenames: "
					+ std::to_string(filenames.size())
					+ " but checksums result suggests "
					+ std::to_string(checksums->size()));
		}
	}

	// Configure table

	const auto types_to_print = ordered_typelist(*checksums);

	resize(track() + filename() + offset() + length() + types_to_print.size(),
				label() + checksums->size());

	if (label())
	{
		// Use column 0 as label column
		set_width(0, defaults::max_label_length);
		set_alignment(0, true);
		set_title(0, "Label");
	}

	int trackno = 1;
	const std::size_t start_col = label() ? 1 : 0;
	const std::string col_label = toc ? "Track " : "File ";

	for (std::size_t col = start_col; col < columns(); ++col)
	{
		set_width(col, 8);
		set_title(col, col_label + std::to_string(trackno));

		++trackno;
	}

	// Print table

// Commented out: Works technically ok, but adds redundancy
//	print_column_titles(o);

// Commented out: printing filenames makes any col width to explode
// Options:
// - Do not print filenames, just assume the right order
// - Make cols fixed width
//	if (filename())
//	{
//		if (label())
//		{
//			o   << std::setw(width(0))
//				<< (alignment(0) > 0 ? std::left : std::right)
//				<< defaults::label(CELL_TYPE::FILENAME)
//				<< column_delimiter();
//		}
//
//		int trackno = 0;
//		for (std::size_t col = start_col; col < columns(); ++col) // print cell
//		{
//			o   << std::setw(width(col))
//				<< (alignment(col) > 0 ? std::left : std::right)
//				<< filenames[trackno]
//				<< column_delimiter();
//			++trackno;
//		}
//		o   << std::endl;
//	}

	if (track())
	{
		if (label())
		{
			o   << std::setw(width(0))
				<< (alignment(0) > 0 ? std::left : std::right)
				<< "Track"
				<< column_delimiter();
		}

		int trackno = 1;
		for (std::size_t col = start_col; col < columns(); ++col) // print cell
		{
			o   << std::setw(width(col))
				<< (alignment(col) > 0 ? std::left : std::right)
				<< std::to_string(trackno)
				<< column_delimiter();
			++trackno;
		}
		o   << std::endl;
	}

	if (offset())
	{
		if (label())
		{
			o   << std::setw(width(0))
				<< (alignment(0) > 0 ? std::left : std::right)
				<< defaults::label(CELL_TYPE::OFFSET)
				<< column_delimiter();
		}

		int trackno = 1;
		for (std::size_t col = start_col; col < columns(); ++col) // print cell
		{
			o   << std::setw(width(col))
				<< (alignment(col) > 0 ? std::left : std::right)
				<< std::to_string(toc->offset(trackno))
				<< column_delimiter();
			++trackno;
		}
		o   << std::endl;
	}

	if (length())
	{
		if (label())
		{
			o   << std::setw(width(0))
				<< (alignment(0) > 0 ? std::left : std::right)
				<< defaults::label(CELL_TYPE::LENGTH)
				<< column_delimiter();
		}

		int trackno = 0;
		for (std::size_t col = start_col; col < columns(); ++col) // print cell
		{
			o   << std::setw(width(col))
				<< (alignment(col) > 0 ? std::left : std::right)
				<< checksums->at(trackno).length()
				<< column_delimiter();
			++trackno;
		}
		o   << std::endl;
	}

	for (const auto& type : types_to_print)
	{
		if (label())
		{
			o   << std::setw(width(0))
				<< (alignment(0) > 0 ? std::left : std::right)
				<< arcstk::checksum::type_name(type)
				<< column_delimiter();
		}

		int trackno = 0;
		for (std::size_t col = start_col; col < columns(); ++col) // print cell
		{
			o   << std::setw(width(col))
				<< (alignment(col) > 0 ? std::left : std::right)
				<< checksum_layout().format(
						checksums->at(trackno).get(type).value(), 8)
				<< column_delimiter();
			++trackno;
		}
		o   << std::endl;
	}
}


// AlbumMatchTableFormat


AlbumMatchTableFormat::AlbumMatchTableFormat(
			const int rows,
			const bool show_label,
			const bool show_track,
			const bool show_offset,
			const bool show_length,
			const bool show_filename)
	: TypedColsTableBase(
			rows, /* rows == tracks */
			(show_track + show_offset + show_length + show_filename + 2),
			/* columns = metadata columns + matching checksums + match info */
			show_label, show_track, show_offset, show_length, show_filename)
	, MatchResultPrinter(
			nullptr, std::vector<std::string>{}, ARResponse{}, nullptr, 0,
				false, nullptr, ARId{0,0,0,0} /* TODO Use EmptyARId */ )
{
	this->init(rows, this->columns());
}


AlbumMatchTableFormat::~AlbumMatchTableFormat() noexcept = default;


void AlbumMatchTableFormat::init(const int /* rows */, const int /* cols */)
{
	// empty
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
		assign_type(col_idx, CELL_TYPE::CHECKSUM);
		set_title(col_idx, arcstk::checksum::type_name(*type_to_print));
		set_width(col_idx, defaults::width(CELL_TYPE::CHECKSUM));

		++col_idx;

		assign_type(col_idx, CELL_TYPE::MATCH);
		set_title(col_idx, defaults::label(CELL_TYPE::MATCH));
		set_width(col_idx, defaults::width(CELL_TYPE::MATCH));

		++col_idx;

		++type_to_print;
	}

	return col_idx - total_metadata_columns();
}


void AlbumMatchTableFormat::do_out(std::ostream &out,
		const std::tuple<Checksums*, std::vector<std::string>, ARResponse,
				Match*, int, bool, TOC*, ARId> &t)
{
	auto checksums = *std::get<0>(t);
	auto filenames = std::get<1>(t);
	auto response = std::get<2>(t);
	auto match = std::get<3>(t);
	auto best = std::get<4>(t);
	auto version = std::get<5>(t);
	auto toc = std::get<6>(t);
	//auto arid = std::get<7>(t);

	using TYPE = arcstk::checksum::type;

	std::vector<TYPE> types_to_print = { version ? TYPE::ARCS2 : TYPE::ARCS1 };

	// Determine number of rows
	const int total_entries = 1 /* col titles */ +
		(toc ? checksums.size()
			 : std::max(static_cast<int>(checksums.size()),
					response.tracks_per_block()));

	// Configure table

	resize(total_entries, total_metadata_columns() + types_to_print.size() * 2);

	const auto md_offset = columns_apply_md_settings();
	set_widths(CELL_TYPE::FILENAME, optimal_width(filenames));

	columns_apply_cs_settings(types_to_print);

	if (!toc) { set_offset(false); }

	// Print contents

	print_column_titles(out);

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
						{ cell = std::to_string(checksums[row].length()); }
					break;

				case CELL_TYPE::CHECKSUM :
					cstype = types_to_print[col - md_offset - m_columns];
					cell = checksum_layout().format(
							checksums[row].get(cstype).value(),
							width(col));
					break;

				case CELL_TYPE::MATCH    :
					cstype = types_to_print[col - md_offset - m_columns];
					++ m_columns;
					if (match->track(best, row, version))
					{
						cell = "   ==   ";
					} else
					{
						cell = checksum_layout().format(
								response[best].at(row).arcs(), width(col));
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

} // namespace arcsapp

