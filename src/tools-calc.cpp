/**
 * \internal
 *
 * \file
 *
 * \brief Implements symbols from tools-calc.hpp.
 */

#ifndef __ARCSTOOLS_TOOLS_CALC_HPP__
#include "tools-calc.hpp"
#endif

#include <cstdint>                  // for uint16_t
#include <iomanip>                  // for setw, setfill
#include <memory>                   // for unique_ptr, make_unique
#include <sstream>                  // for ostringstream
#include <stdexcept>                // for invalid_argument
#include <string>                   // for string
#include <tuple>                    // for make_tuple, tuple
#include <unordered_set>            // for unordered_set
#include <utility>                  // for move, make_pair, pair
#include <vector>                   // for vector

#if __cplusplus >= 201703L
#include <filesystem>
#endif

#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>     // for Checksums, type
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#ifndef __LIBARCSDEC_CALCULATORS_HPP__
#include <arcsdec/calculators.hpp>  // for ToCParser, ARCSCalculator
#endif
#ifndef __LIBARCSDEC_SELECTION_HPP__
#include <arcsdec/selection.hpp>    // for FileReaderPreferenceSelection
#endif

#ifndef __ARCSTOOLS_TOOLS_FS_HPP__
#include "tools-fs.hpp"             // for path, prepend_path
#endif

namespace arcsapp
{
inline namespace v_1_0_0
{
namespace calc
{

using arcsdec::ARCSCalculator;
using arcsdec::FileReaderSelection;
using arcsdec::ToCParser;


std::tuple<bool,bool,std::vector<std::string>> ToCFiles::get(const ToC& toc)
{
	const auto toc_list { toc.filenames() };

	const auto& [ is_single, pairwise_distinct ] = ToCFiles::flags(toc_list);

	if (!pairwise_distinct)
	{
		return { is_single, pairwise_distinct, {/* no filenames */} };
	}

	// Since the filenames are pairwise distinct,
	// return either only the first (if single) or the entire list

	if (is_single && !toc_list.empty())
	{
		return { is_single, pairwise_distinct,
			std::vector<std::string>{ toc_list.front() } };
	}

	return { is_single, pairwise_distinct, toc_list };
}


std::pair<bool,bool> ToCFiles::flags(const std::vector<std::string>& filenames)
{
	if (filenames.empty())
	{
		return { true, false };
	}

	using std::cbegin;
	using std::cend;

	const auto set = std::unordered_set<std::string>(cbegin(filenames),
			cend(filenames));

	const bool is_single { set.size() == 1 };

	return {
		is_single, /*single file ?*/
		is_single or set.size() == filenames.size() /*pairwise distinct ?*/
	};
}


std::string ToCFiles::expand_path(const std::string& metafilename,
		const std::string& audiofile)
{
	namespace fs = std::filesystem;

	auto filepath  = fs::path { metafilename };

	filepath.remove_filename();
	filepath += fs::path { audiofile };

	return filepath.generic_string();
}


// IdSelection


std::unique_ptr<arcsdec::FileReaderSelection> IdSelection::operator()(
		const std::string& id) const
{
	using IdSelection_t = arcsdec::FileReaderPreferenceSelection<
		arcsdec::MinPreference, arcsdec::IdSelector>;

	return !id.empty() ? std::make_unique<IdSelection_t>(id) : nullptr;
}


// ChecksumCalculator


ChecksumCalculator::ChecksumCalculator()
	: ChecksumCalculator(
			{ arcstk::checksum::type::ARCS1, arcstk::checksum::type::ARCS2 })
{
	// empty
};


ChecksumCalculator::ChecksumCalculator(
		const ChecksumTypeset& types)
	: types_           { types }
	, audio_selection_ { nullptr }
	, toc_selection_   { nullptr }
{
	// empty
}


ChecksumCalculator::~ChecksumCalculator() noexcept
= default;


std::pair<Checksums, std::unique_ptr<ToC>>
	ChecksumCalculator::calculate(
			const std::vector<std::string>& audiofilenames,
			const std::string& metafilename) const
{
	ARCS_LOG_DEBUG << "Calculate result from metafilename"
			" and one or more audiofiles";

	if (metafilename.empty())
	{
		throw std::invalid_argument("No ToC file specified.");
	}

	auto toc { setup_parser().parse(metafilename) };

	if (audiofilenames.empty())
	{
		// No audio files passed? => Use from ToC

		return calculate(toc, file::path(metafilename));
	}

	// Validate track number

	const int filecount = audiofilenames.size();

	if (filecount != toc->total_tracks() && filecount != 1) // case: illegal
	{
		std::ostringstream msg;
		msg << "Inconsistent input: Metafile " << metafilename
			<< " specifies " << toc->total_tracks() << " tracks"
			<< " but " << filecount << " audio files were passed to override.";

		throw std::invalid_argument(msg.str());
	}

	ARCS_LOG_INFO << "Specified audio filenames override ToC filenames."
			" Audiofiles from ToC are ignored.";

	// Run

	auto calculator { setup_arcs_calculator() };

	// case: single-file album w ToC
	if (1 == filecount)
	{
		const auto [ checksums, toc2 ] =
			calculator.calculate(audiofilenames.front(), *toc);

		return { checksums, std::make_unique<ToC>(toc2) };
	}

	// case: multi-file album w ToC (== "EAC-styled layout")
	if (toc->total_tracks() == filecount)
	{
		const auto chksums { calculator.calculate(audiofilenames, true, true) };

		return { chksums, std::move(toc) };
	}

	return { Checksums{ 0 }, nullptr }; // TODO should throw instead
}


std::pair<Checksums, std::unique_ptr<ToC>>
	ChecksumCalculator::calculate(
		const std::vector<std::string>& audiofilenames,
		const bool first_is_first_track, const bool last_is_last_track) const
{
	auto calculator { setup_arcs_calculator() };

	const auto checksums { calculator.calculate(audiofilenames,
			first_is_first_track, last_is_last_track) };

	return { checksums, nullptr };
}


void ChecksumCalculator::set_types(const ChecksumTypeset& types)
{
	types_ = types;
}


ChecksumTypeset ChecksumCalculator::types() const
{
	return types_;
}


void ChecksumCalculator::set_toc_selection(FileReaderSelection* selection)
{
	toc_selection_ = selection;
}


FileReaderSelection* ChecksumCalculator::toc_selection() const
{
	return toc_selection_;
}


void ChecksumCalculator::set_audio_selection(FileReaderSelection* selection)
{
	audio_selection_ = selection;
}


FileReaderSelection* ChecksumCalculator::audio_selection() const
{
	return audio_selection_;
}


std::pair<Checksums, std::unique_ptr<ToC>>
	ChecksumCalculator::calculate(
		const std::unique_ptr<ToC>& toc, const std::string& filepath) const
{
	ARCS_LOG_DEBUG << "Calculate result from ToC"
			" and searchpath for audiofiles";

	// Validate audio file set in ToC

	auto [ is_single_file, pairwise_dist, audiofiles ] = ToCFiles::get(*toc);

	if (!is_single_file && !pairwise_dist)
	{
		throw std::invalid_argument(
			"ToC references a set of multiple audio files, but they are not "
			"pairwise distinct. This usecase is unsupported in the current "
			"version.");
	}

	// Calculate ARCSs

	auto calculator { setup_arcs_calculator() };

	if (is_single_file)
	{
		const auto audiofile =
			ToCFiles::expand_path(filepath, audiofiles.front());

		// case: single-file album w ToC
		const auto [ checksums, toc2 ] = calculator.calculate(audiofile, *toc);

		return { checksums, std::make_unique<ToC>(toc2) };
	} else
	{
		for (auto& audiofile : audiofiles)
		{
			audiofile = ToCFiles::expand_path(filepath, audiofile);
		}

		// case: multi-file album w toc (== "EAC-styled layout")
		const auto checksums { calculator.calculate(audiofiles, true, true) };

		return { checksums, nullptr };
	}
}


ARCSCalculator ChecksumCalculator::setup_arcs_calculator() const
{
	auto calculator { ARCSCalculator { types() } };

	if (audio_selection())
	{
		calculator.set_selection(audio_selection());
	}

	return calculator;
}


ToCParser ChecksumCalculator::setup_parser() const
{
	auto parser { ToCParser{} };

	if (toc_selection())
	{
		parser.set_selection(toc_selection());
	}

	return parser;
}


// HexLayout


HexLayout::HexLayout()
	: PropertyStore { 0x00000002 }
{
	// means: SHOW_BASE is OFF, UPPERCASE is ON
}


std::string HexLayout::do_format(InputTuple t) const
{
	auto checksum = std::get<0>(t);
	auto width    = std::get<1>(t);

	std::ostringstream ss;

	if (has_property(HEX_FLAG::SHOW_BASE))
	{
		ss << std::showbase;
	}

	if (has_property(HEX_FLAG::UPPERCASE))
	{
		ss << std::uppercase;
	}

	ss << std::hex << std::setw(width) << std::setfill('0') << checksum.value();

	return ss.str();
}


// validate


void validate(const Checksums& checksums, const ToC* toc,
		const std::vector<std::string>& filenames)
{
	using std::to_string;

	const auto total_tracks = checksums.size();

	if (total_tracks == 0)
	{
		throw std::invalid_argument("Missing value: "
				"Need some Checksums to print");
	}

	if (!(filenames.empty()
				|| filenames.size() == total_tracks || filenames.size() == 1))
	{
		throw std::invalid_argument("Mismatch: "
				"Checksums for " + to_string(total_tracks)
				+ " files/tracks, but " + to_string(filenames.size())
				+ " files.");
	}

	if (checksums.at(0).empty() || checksums.at(0).types().empty())
	{
		throw std::invalid_argument("Missing value: "
				"Checksums seem to hold no checksums");
	}

	if (!toc && filenames.empty())
	{
		throw std::invalid_argument("Missing value: "
				"Need either ToC data or filenames to print results");
	}

	if (toc && static_cast<uint16_t>(toc->total_tracks()) != total_tracks)
	{
		throw std::invalid_argument("Mismatch: "
				"Checksums for " + to_string(total_tracks)
				+ " files/tracks, but ToC specifies "
				+ to_string(toc->total_tracks()) + " tracks.");
	}
}

} // namespace calc
} // namespace v_1_0_0
} // namespace arcsapp

