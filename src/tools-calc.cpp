/**
 * \file tools-calc.cpp Combine ARCS calculation with file handling
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
#include <utility>                  // for move
#include <vector>                   // for vector

#if __cplusplus >= 201703L
#include <filesystem>
#endif

#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>     // for Checksums, type
#endif
#ifndef __LIBARCSTK_IDENTIFIER_HPP__
#include <arcstk/identifier.hpp>    // for ARId, EmptyARId
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


std::tuple<bool,bool> ToCFiles::flags(const std::vector<std::string>& filenames)
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


std::tuple<Checksums, ARId, std::unique_ptr<ToC>>
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

		return calculate(std::move(toc), file::path(metafilename));
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

	auto calculator { setup_calculator() };

	// case: single-file album w ToC
	if (1 == filecount)
	{
		const auto [ checksums, arid ] =
			calculator.calculate(audiofilenames.front(), *toc);

		return { checksums, arid, std::move(toc) };
	}

	// case: multi-file album w ToC (== "EAC-styled layout")
	if (toc->total_tracks() == filecount)
	{
		const auto chksums { calculator.calculate(audiofilenames, true, true) };
		const auto arid    { make_arid(*toc) };

		return { chksums, *arid, std::move(toc) };
	}

	return { Checksums{ 0 }, arcstk::EmptyARId, nullptr };
}


std::tuple<Checksums, ARId, std::unique_ptr<ToC>>
	ChecksumCalculator::calculate(
		const std::vector<std::string>& audiofilenames,
		const bool first_is_first_track, const bool last_is_last_track) const
{
	auto calculator { setup_calculator() };

	const auto checksums { calculator.calculate(audiofilenames,
			first_is_first_track, last_is_last_track) };

	return { checksums, arcstk::EmptyARId, nullptr };
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


std::tuple<Checksums, ARId, std::unique_ptr<ToC>>
	ChecksumCalculator::calculate(
		std::unique_ptr<ToC> toc, const std::string& filepath) const
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

	auto calculator { setup_calculator() };

	if (is_single_file)
	{
		const auto audiofile =
			ToCFiles::expand_path(filepath, audiofiles.front());

		// case: single-file album w ToC
		const auto [ checksums, arid ] = calculator.calculate(audiofile, *toc);

		return { checksums, arid, std::move(toc) };
	} else
	{
		for (auto& audiofile : audiofiles)
		{
			audiofile = ToCFiles::expand_path(filepath, audiofile);
		}

		// case: multi-file album w toc (== "EAC-styled layout")
		const auto checksums { calculator.calculate(audiofiles, true, true) };
		const auto arid      { make_arid(*toc) };

		return { checksums, *arid, std::move(toc) };
	}
}


ARCSCalculator ChecksumCalculator::setup_calculator() const
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
	: WithInternalFlags()
{
	set_show_base(false);
	set_uppercase(true);
}


void HexLayout::set_show_base(const bool base)
{
	flags().set_flag(0, base);
}


bool HexLayout::shows_base() const
{
	return flags().flag(0);
}


void HexLayout::set_uppercase(const bool uppercase)
{
	flags().set_flag(1, uppercase);
}


bool HexLayout::is_uppercase() const
{
	return flags().flag(1);
}


std::string HexLayout::do_format(InputTuple t) const
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


void validate(const Checksums& checksums, const ToC* toc,
		const ARId& arid, const std::vector<std::string>& filenames)
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

	if (!(arid.empty()
		|| static_cast<uint16_t>(arid.track_count()) == total_tracks))
	{
		throw std::invalid_argument("Mismatch: "
				"Checksums for " + to_string(total_tracks)
				+ " files/tracks, but AccurateRip id specifies "
				+ to_string(arid.track_count()) + " tracks.");
	}
}

} // namespace calc
} // namespace v_1_0_0
} // namespace arcsapp

