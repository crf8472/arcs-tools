#include <arcstk/identifier.hpp>
#include <sstream>
#include <tuple>
#ifndef __ARCSTOOLS_PRINTERS_HPP__
#include "printers.hpp"
#endif

#include <algorithm>  // for max, copy_if
#include <iomanip>    // for operator<<, setw, setfill
#include <iostream>   // for ostream, operator<<, basic_ostream, endl, size_t
#include <iterator>   // for back_insert_iterator, back_inserter
#include <set>        // for operator!=, set, set<>::const_iterator
#include <stdexcept>  // for invalid_argument
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


//ARTripletFormat::ARTripletFormat()
//	: Print<int, ARTriplet> { std::make_tuple(nullptr, nullptr) }
//{
//	// empty
//}
//
//
//void ARTripletFormat::assertions(const std::tuple<const int*,
//		const ARTriplet*> &t) const
//{
//	const auto track   = std::get<0>(t);
//
//	if (!track)
//	{
//		throw std::invalid_argument("Cannot print a nullptr as track");
//	}
//
//	const auto triplet = std::get<1>(t);
//
//	if (!triplet)
//	{
//		throw std::invalid_argument("Cannot print a nullptr as triplet");
//	}
//}


// ARIdFormat


//ARIdFormat::ARIdFormat(const bool id, const bool url,
//		const bool filename, const bool track_count, const bool disc_id_1,
//		const bool disc_id_2, const bool cddb_id)
//	: WithARIdLayout(std::make_unique<ARIdTableLayout>(
//				id, url, filename, track_count, disc_id_1, disc_id_2, cddb_id))
//	, ARIdPrinter(std::make_tuple(&arcstk::EmptyARId, nullptr))
//{
//	// empty
//}
//
//
//ARIdFormat::~ARIdFormat() noexcept = default;
//
//
//void ARIdFormat::assertions(const std::tuple<const ARId*,
//		const std::string*> &t) const
//{
//	const auto arid = std::get<0>(t);
//
//	if (!arid)
//	{
//		throw std::invalid_argument("Cannot print a null pointer for an ARId");
//	}
//
//	// alt_prefix is allowed to be nullptr
//}
//
//
//void ARIdFormat::do_out(std::ostream &out,
//		const std::tuple<const ARId*, const std::string*> &t)
//{
//	assertions(t);
//
//	const auto id         = std::get<0>(t);
//	const auto alt_prefix = std::get<1>(t);
//
//	if (!alt_prefix)
//	{
//		out << arid_layout()->format(*id, std::string{}) << std::endl;
//	} else
//	{
//		out << arid_layout()->format(*id, *alt_prefix) << std::endl;
//	}
//}


// ChecksumsResultPrinter


ChecksumsResultPrinter::ChecksumsResultPrinter()
	: ChecksumsResultPrinterBase(
			std::make_tuple(nullptr, nullptr, nullptr, nullptr, nullptr))
{
	// empty
}


void ChecksumsResultPrinter::assertions(
		const std::tuple<const Checksums*, const std::vector<std::string>*,
		const TOC*, const ARId*, const bool*> &t) const
{
	const auto checksums = std::get<0>(t);
	const auto filenames = std::get<1>(t);
	const auto toc       = std::get<2>(t);
	const auto arid      = std::get<3>(t);
	//const auto is_album  = std::get<4>(t); // unused

	const auto total_tracks = checksums->size();

	if (!checksums or total_tracks == 0)
	{
		throw std::invalid_argument("Missing value: "
				"Need some Checksums to print");
	}

	if (auto track0 = checksums->at(0);
		track0.types().empty() or track0.empty())
	{
		throw std::invalid_argument("Missing value: "
				"Checksums seem to hold no checksums");
	}

	if (!toc and (!filenames or filenames->empty()))
	{
		throw std::invalid_argument("Missing value: "
				"Need either TOC data or filenames to print results");
	}

	if (toc && static_cast<uint16_t>(toc->track_count()) != total_tracks)
	{
		throw std::invalid_argument("Mismatch: "
				"Checksums for " + std::to_string(total_tracks)
				+ " files/tracks, but TOC specifies "
				+ std::to_string(toc->track_count()) + " tracks.");
	}

	if (filenames and not (filenames->empty()
				or filenames->size() == total_tracks or filenames->size() == 1))
	{
		throw std::invalid_argument("Mismatch: "
				"Checksums for " + std::to_string(total_tracks)
				+ " files/tracks, but " + std::to_string(filenames->size())
				+ " files.");
	}

	if (arid and not (arid->empty()
		or static_cast<uint16_t>(arid->track_count()) == total_tracks))
	{
		throw std::invalid_argument("Mismatch: "
				"Checksums for " + std::to_string(total_tracks)
				+ " files/tracks, but AccurateRip id specifies "
				+ std::to_string(arid->track_count()) + " tracks.");
	}
}


// AlbumChecksumsTableFormat


AlbumChecksumsTableFormat::AlbumChecksumsTableFormat(
		const bool show_labels,
		const bool show_track,
		const bool show_offset,
		const bool show_length,
		const bool show_filename,
		const std::string &coldelim)
	: TypedColsTableBase(0, 0,
			show_labels, show_track, show_offset, show_length, show_filename)
	, ChecksumsResultPrinter(std::make_tuple(nullptr, nullptr, nullptr, nullptr,
				nullptr))
{
	this->set_column_delimiter(coldelim);
}


AlbumChecksumsTableFormat::~AlbumChecksumsTableFormat() noexcept
= default;


void AlbumChecksumsTableFormat::do_init(const int /* rows */,
		const int /* cols */)
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
		const std::tuple<const Checksums*, const std::vector<std::string>*,
		const TOC*, const ARId*, const bool*> &t)
{
	assertions(t);

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


	// Configure table

	if (!toc) { set_offset(false); }
	if (filenames->empty()) { set_filename(false); }
	// assertion is fulfilled that either not filenames.empty() or non-null toc

	// TODO Use init() instead
	resize(checksums->size() + label(),
			total_metadata_columns() + types_to_print.size());
	// assertion is fulfilled that rows >= 1 as well as columns >= 1

	const auto md_offset = columns_apply_md_settings();
	columns_apply_cs_settings(types_to_print);
	set_widths(CELL_TYPE::FILENAME, optimal_width(*filenames));


	// Print table

	if (label()) { print_column_titles(out); }

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
					cell = checksum_layout().format(
							(*checksums)[row].get(cstype), width(col));
					break;

				case CELL_TYPE::MATCH    : break;
			}

			print_cell(out, col, cell, col < columns() - 1);
		}
		out << std::endl;
	}
}


// AlbumTracksTableFormat


AlbumTracksTableFormat::AlbumTracksTableFormat(const bool label,
		const bool track, const bool offset, const bool length,
		const bool filename, const std::string &coldelim)
	: TypedRowsTableBase(0, 0, label, track, offset, length, filename)
	, ChecksumsResultPrinter(std::make_tuple(nullptr, nullptr, nullptr, nullptr,
				nullptr))
{
	this->set_column_delimiter(coldelim);
}


void AlbumTracksTableFormat::do_init(const int /* rows */, const int /* cols */)
{
	// empty
}


void AlbumTracksTableFormat::do_out(std::ostream &o,
		const std::tuple<const Checksums*, const std::vector<std::string>*,
		const TOC*, const ARId*, const bool*> &t)
{
	assertions(t);

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

	if (is_album) { set_track(true); }
	if (!toc) { set_offset(false); }
	if (!filenames or filenames->empty()) { set_filename(false); }
	// assertion is fulfilled that either not filenames.empty() or non-null toc

	const bool show_input = filename(); // Overrides --no-track-nos

	// TODO Use init() instead
	resize(show_input + offset() + length() + types_to_print.size(),
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

			set_row_label(row, input_label);
			++row;
		}
		for (const auto& type : types_to_print)
		{
			set_row_label(row, arcstk::checksum::type_name(type));
			++row;
		}
		if (offset())
			{ set_row_label(row, defaults::label(CELL_TYPE::OFFSET)); ++row; }
		if (length())
			{ set_row_label(row, defaults::label(CELL_TYPE::LENGTH)); ++row; }
		// ... skip filename ...
	}

	// Assign column widths
	for (std::size_t col = 0; col < columns(); ++col) // print cell
	{
		set_width(col, 8); // TODO Magic number, only ok while no filenames
		set_alignment(col, false);
	}

	// Print table rows

	int row = 0;

	if (show_input)
	{
		if (label()) { print_label(o, row); }

		int trackno = 1;
		for (std::size_t col = 0; col < columns() - 1; ++col)
		{
			print_cell(o, col, std::to_string(trackno), true);
			++trackno;
		}
		print_cell(o, columns() - 1, std::to_string(trackno), false);

		o << std::endl;
		++row;
	}

	for (const auto& type : types_to_print)
	{
		if (label()) { print_label(o, row); }

		int index = 0;
		for (std::size_t col = 0; col < columns() - 1; ++col, ++index)
		{
			print_cell(o, col,
				checksum_layout().format(checksums->at(index).get(type), 8),
				true);
		}

		print_cell(o, columns() - 1,
				checksum_layout().format(checksums->at(index).get(type), 8),
				false);

		o << std::endl;
		++row;
	}

	if (offset())
	{
		if (label()) { print_label(o, row); }

		int trackno = 1;
		for (std::size_t col = 0; col < columns() - 1; ++col)
		{
			print_cell(o, col, std::to_string(toc->offset(trackno)), true);
			++trackno;
		}
		print_cell(o, columns() - 1, std::to_string(toc->offset(trackno)),
				false);

		o << std::endl;
		++row;
	}

	if (length())
	{
		if (label()) { print_label(o, row); }

		int index = 0;
		for (std::size_t col = 0; col < columns() - 1; ++col, ++index)
		{
			print_cell(o, col,
				std::to_string(checksums->at(index).length()),
				true);
		}
		print_cell(o, columns() - 1,
				std::to_string(checksums->at(index).length()),
				false);

		o << std::endl;
		++row;
	}
}


// MatchResultPrinter


MatchResultPrinter::MatchResultPrinter()
	: MatchResultPrinterBase(std::make_tuple(nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr))
	, match_symbol_ { "   ==   " }
{
	// empty
}


void MatchResultPrinter::set_match_symbol(const std::string &match_symbol)
{
	match_symbol_ = match_symbol;
}


const std::string& MatchResultPrinter::match_symbol() const
{
	return match_symbol_;
}


void MatchResultPrinter::assertions(
		const std::tuple<const Checksums*, const std::vector<std::string>*,
			const std::vector<Checksum>*, const Match*, const int*, const bool*,
			const TOC*, const ARId*> &t) const
{
	const auto  checksums = std::get<0>(t);
	const auto& filenames = std::get<1>(t);
	const auto  toc       = std::get<6>(t);
	const auto& arid      = std::get<7>(t);

	const auto total_tracks = checksums->size();

	if (!checksums or total_tracks == 0)
	{
		throw std::invalid_argument("Missing value: "
				"Need some Checksums to print");
	}

	if (auto track0 = checksums->at(0);
		track0.types().empty() or track0.empty())
	{
		throw std::invalid_argument("Missing value: "
				"Checksums seem to hold no checksums");
	}

	if (!toc and (!filenames or filenames->empty()))
	{
		throw std::invalid_argument("Missing value: "
				"Need either TOC data or filenames to print results");
	}

	if (toc && static_cast<uint16_t>(toc->track_count()) != total_tracks)
	{
		throw std::invalid_argument("Mismatch: "
				"Checksums for " + std::to_string(total_tracks)
				+ " files/tracks, but TOC specifies "
				+ std::to_string(toc->track_count()) + " tracks.");
	}

	if (not (!filenames or filenames->empty()
				or filenames->size() == total_tracks or filenames->size() == 1))
	{
		throw std::invalid_argument("Mismatch: "
				"Checksums for " + std::to_string(total_tracks)
				+ " files/tracks, but " + std::to_string(filenames->size())
				+ " files.");
	}

	if (arid and not (arid->empty()
		or static_cast<uint16_t>(arid->track_count()) == total_tracks))
	{
		throw std::invalid_argument("Mismatch: "
				"Checksums for " + std::to_string(total_tracks)
				+ " files/tracks, but AccurateRip id specifies "
				+ std::to_string(arid->track_count()) + " tracks.");
	}

	// Specific for verify

	const auto  refsums   = std::get<2>(t);
	const auto  match     = std::get<3>(t);
	const auto  block     = std::get<4>(t);
	//const auto  version   = std::get<5>(t); // unused

	if (!refsums)
	{
		throw std::invalid_argument("Missing reference checksums, "
				"nothing to print.");
	}

	if (refsums->size() != total_tracks)
	{
		throw std::invalid_argument("Mismatch: "
				"Reference for " + std::to_string(refsums->size())
				+ " tracks, but Checksums specify "
				+ std::to_string(total_tracks) + " tracks.");
	}

	if (!match)
	{
		throw std::invalid_argument("Missing match information, "
				"nothing to print.");
	}

	if (!block)
	{
		throw std::invalid_argument(
			"Index of matching checksum block is missing, "
				"nothing to print.");
	}

	if (*block > match->total_blocks())
	{
		throw std::invalid_argument("Mismatch: "
				"Match contains no block " + std::to_string(*block)
				+ " but contains only "
				+ std::to_string(match->total_blocks()) + " blocks.");
	}
}


// AlbumMatchTableFormat


AlbumMatchTableFormat::AlbumMatchTableFormat(
			const bool show_label,
			const bool show_track,
			const bool show_offset,
			const bool show_length,
			const bool show_filename,
			const std::string &coldelim)
	: TypedColsTableBase(0, 0,
			show_label, show_track, show_offset, show_length, show_filename)
	, MatchResultPrinter()
{
	this->set_column_delimiter(coldelim);
}


AlbumMatchTableFormat::~AlbumMatchTableFormat() noexcept = default;


void AlbumMatchTableFormat::do_init(const int /* rows */, const int /* cols */)
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


void AlbumMatchTableFormat::do_out(std::ostream &out,
		const std::tuple<const Checksums*, const std::vector<std::string>*,
			const std::vector<Checksum>*, const Match*, const int*, const bool*,
			const TOC*, const ARId*> &t)
{
	assertions(t);

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
		if (*version)
		{
			types_to_print.push_back(TYPE::ARCS2);
		} else
		{
			types_to_print.push_back(TYPE::ARCS1);
		}
	}
	types_to_print.shrink_to_fit();

	// Configure table

	// Determine number of rows
	const int total_entries = 1 /* col titles */
		+ (toc ? checksums->size()
			   : std::max(checksums->size(), refsums->size()));

	if (!toc) { set_offset(false); }

	// TODO Use init() instead
	resize(total_entries, total_metadata_columns() + types_to_print.size() + 1);

	const auto md_offset = columns_apply_md_settings();
	set_widths(CELL_TYPE::FILENAME, optimal_width(*filenames));

	columns_apply_cs_settings(types_to_print);

	// Print contents

	if (label()) { print_column_titles(out); }

	TYPE cstype = TYPE::ARCS2; // default
	std::string cell{};
	int trackno = 1;

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
					cell = checksum_layout().format((*refsums)[row],
							width(col));
					break;

				case CELL_TYPE::MATCH    : // "Mine" columns (may be one or two)
					cstype = types_to_print[col - md_offset - 1];
					if (match->track(*block, row, cstype == TYPE::ARCS2))
					{
						cell = match_symbol();
					} else
					{
						cell = checksum_layout().format(
							checksums->at(row).get(cstype),
							width(col));
					}
					break;
			}

			print_cell(out, col, cell, col < columns() - 1);
		}

		out << std::endl;
	}
}

} // namespace arcsapp
