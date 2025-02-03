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
#include <iostream>

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
using arcsdec::ToCParser;
using arcsdec::FileReaderSelection;


std::tuple<bool,bool,std::vector<std::string>> audiofile_layout(const ToC& toc)
{
	auto list { toc.filenames() };

	if (list.empty())
	{
		return std::make_tuple(true, false, std::vector<std::string>{});
	}

	std::unordered_set<std::string> set(list.begin(), list.end());
	const bool is_single { set.size() == 1 };
	return std::make_tuple(is_single, is_single or set.size() == list.size(),
			is_single ? std::vector<std::string>{ *list.cbegin() } : list);
}


// IdSelection


std::unique_ptr<arcsdec::FileReaderSelection> IdSelection::operator()(
		const std::string& id) const
{
	using IdSelection_t = arcsdec::FileReaderPreferenceSelection<
		arcsdec::MinPreference, arcsdec::IdSelector>;

	return !id.empty() ? std::make_unique<IdSelection_t>(id) : nullptr;
}


/**
 * \brief Private implementation of a ARCSMultifileAlbumCalculator
 */
class ARCSMultifileAlbumCalculator::Impl final
{
public:

	/**
	 * \brief Calculate ARCS values for album with optional audiofile names.
	 *
	 * If \c audiofilenames are non-empty, they override the audiofilenames in
	 * the metafile. If \c audiofilenames are empty, the audiofilenames from the
	 * metafile will be searched for in the path of the metafile.
	 *
	 * \param[in] metafilename   Metadata file
	 * \param[in] audiofilenames List of audiofile names
	 *
	 * \return Checksums, ARId and ToC for the input
	 */
	std::tuple<Checksums, ARId, std::unique_ptr<ToC>> calculate(
			const std::vector<std::string> &audiofilenames,
			const std::string &metafilename) const;

	/**
	 * \brief Calculate ARCS values for audiofilenams without ToC.
	 *
	 * Mere wrapper for arcsdec::ARCSCalculator.
	 *
	 * \param[in] audiofilenames Names of the audiofile
	 * \param[in] skip_front     Skip front samples of first track
	 * \param[in] skip_back      Skip back samples of last track
	 *
	 * \return The AccurateRip checksum of this track
	 */
	Checksums calculate(const std::vector<std::string> &audiofilenames,
			const bool &skip_front, const bool &skip_back) const;

	/**
	 * \brief Set the checksum type to be calculated.
	 *
	 * \param[in] type Checksum type to be calculated
	 */
	void set_types(const ChecksumTypeset& type);

	/**
	 * \brief The checksum type to be calculated.
	 *
	 * \return Checksum type to be calculated by this instance
	 */
	ChecksumTypeset types() const;

	/**
	 * \brief Set the FileReaderSelection for this instance.
	 *
	 * \param[in] selection The selection to use
	 */
	void set_toc_selection(FileReaderSelection *selection);

	/**
	 * \brief Get the FileReaderSelection used by this instance.
	 *
	 * \return The selection used by this instance
	 */
	FileReaderSelection* toc_selection() const;

	/**
	 * \brief Set the FileReaderSelection for this instance.
	 *
	 * \param[in] selection The selection to use
	 */
	void set_audio_selection(FileReaderSelection *selection);

	/**
	 * \brief Get the FileReaderSelection used by this instance.
	 *
	 * \return The selection used by this instance
	 */
	FileReaderSelection* audio_selection() const;

private:

	/**
	 * \brief Calculate ARCS values for album with audiofilenames from metafile.
	 *
	 * If metafile does not specify any audiofilenames, result will be empty.
	 *
	 * \param[in] metafilename Metadata file
	 * \param[in] searchpath   Searchpath for audiofiles
	 *
	 * \return Checksums, ARId and ToC for the input
	 */
	std::tuple<Checksums, ARId, std::unique_ptr<ToC>> calculate(
			const std::string &metafilename,
			const std::string &searchpath) const;

	/**
	 * \brief Checksum type to request
	 */
	ChecksumTypeset types_ = {  arcstk::checksum::type::ARCS1,
								arcstk::checksum::type::ARCS2   };

	/**
	 * \brief Internal ToC parser selection.
	 */
	FileReaderSelection* toc_selection_ = nullptr;

	/**
	 * \brief Internal Audio reader selection.
	 */
	FileReaderSelection* audio_selection_ = nullptr;
};


std::tuple<Checksums, ARId, std::unique_ptr<ToC>>
	ARCSMultifileAlbumCalculator::Impl::calculate(
			const std::vector<std::string> &audiofilenames,
			const std::string &metafilename) const
{
	ARCS_LOG_DEBUG << "Calculate result from metafilename"
			" and one or more audiofiles";

	if (audiofilenames.empty())
	{
		ARCS_LOG_WARNING << "Requested multiple audiofile calculation but "
			"no audiofiles provided. "
			"Fall back to extracting audiofilenames from metafile.";

		return this->calculate(metafilename, file::path(metafilename));
	}

	if (metafilename.empty())
	{
		throw std::invalid_argument("No ToC file specified.");
	}

	ToCParser parser;
	if (toc_selection()) { parser.set_selection(toc_selection()); }
	auto toc { parser.parse(metafilename) };

	// Validate ToC information

	const int filecount = audiofilenames.size();

	if (toc->total_tracks() != filecount and filecount != 1) // case: illegal
	{
		std::ostringstream msg;
		msg << "Inconsistent input: Metafile " << metafilename
			<< " specifies " << toc->total_tracks() << " tracks"
			<< " but " << filecount << " audio files were passed to override.";

		throw std::invalid_argument(msg.str());
	}

	ARCS_LOG_INFO << "Specified audio filenames override ToC filenames."
			" Audiofiles from ToC are ignored.";

	ARCSCalculator c { types() };
	if (audio_selection()) { c.set_selection(audio_selection()); }

	// case: single-file album w ToC
	if (1 == filecount)
	{
		const auto [ checksums, arid ] = c.calculate(audiofilenames[0], *toc);

		return std::make_tuple(checksums, arid, std::move(toc));
	}

	// case: multi-file album w ToC
	if (toc->total_tracks() == filecount)
	{
		const auto checksums { c.calculate(audiofilenames, true, true) };
		const auto arid      { make_arid(*toc) };

		return std::make_tuple(checksums, *arid, std::move(toc));
	}

	return std::make_tuple(Checksums{ 0 }, arcstk::EmptyARId, nullptr);
}


std::tuple<Checksums, ARId, std::unique_ptr<ToC>>
	ARCSMultifileAlbumCalculator::Impl::calculate(
		const std::string &metafilename,
		const std::string &audiosearchpath) const
{
	ARCS_LOG_DEBUG << "Calculate result from metafilename"
			" and searchpath for audiofiles";

	if (metafilename.empty())
	{
		throw std::invalid_argument("No ToC file specified.");
	}

	ToCParser parser;
	if (toc_selection()) { parser.set_selection(toc_selection()); }
	auto toc { parser.parse(metafilename) };

	// Validate ToC information

	auto [ single_audio_file, pairwise_distinct, audiofiles ] =
		audiofile_layout(*toc);

	if (not single_audio_file and not pairwise_distinct)
	{
		throw std::invalid_argument(
			"ToC references a set of multiple audio files, but they are not "
			"pairwise distinct. This usecase is unsupported in the current "
			"version.");
	}

	// Calculate ARCSs

	ARCSCalculator calculator { types() };
	if (audio_selection()) { calculator.set_selection(audio_selection()); }

	if (single_audio_file)
	{
		auto audiofile { toc->filenames().front() };

		if (not audiosearchpath.empty())
		{
			// XXX This assumes that the metafile does not specify any paths
			file::prepend_path(audiosearchpath, audiofile);
		}

		// case: single-file album w ToC
		auto [ checksums, arid ] = calculator.calculate(audiofile, *toc);

		return std::make_tuple(checksums, arid, std::move(toc));
	} else
	{
		if (not audiosearchpath.empty())
		{
			for (auto& audiofile : audiofiles)
			{
				// XXX This assumes that the metafile does not specify any paths
				file::prepend_path(audiosearchpath, audiofile);
			}
		}

		// case: multi-file album w toc
		auto checksums { calculator.calculate(audiofiles, true, true) };
		auto arid      { make_arid(*toc) };

		return std::make_tuple(checksums, *arid, std::move(toc));
	}
}


Checksums ARCSMultifileAlbumCalculator::Impl::calculate(
		const std::vector<std::string> &audiofilenames,
		const bool &skip_front, const bool &skip_back) const
{
	ARCSCalculator calculator { types() };
	if (audio_selection()) { calculator.set_selection(audio_selection()); }

	return calculator.calculate(audiofilenames, skip_front, skip_back);
}


void ARCSMultifileAlbumCalculator::Impl::set_types(
		const ChecksumTypeset& types)
{
	types_ = types;
}


ChecksumTypeset ARCSMultifileAlbumCalculator::Impl::types() const
{
	return types_;
}


void ARCSMultifileAlbumCalculator::Impl::set_toc_selection(
		FileReaderSelection *selection)
{
	toc_selection_ = selection;
}


FileReaderSelection* ARCSMultifileAlbumCalculator::Impl::toc_selection() const
{
	return toc_selection_;
}


void ARCSMultifileAlbumCalculator::Impl::set_audio_selection(
		FileReaderSelection *selection)
{
	audio_selection_ = selection;
}


FileReaderSelection* ARCSMultifileAlbumCalculator::Impl::audio_selection() const
{
	return audio_selection_;
}


// ARCSMultifileAlbumCalculator


ARCSMultifileAlbumCalculator::ARCSMultifileAlbumCalculator(
		const ChecksumTypeset& types)
	: impl_(std::make_unique<ARCSMultifileAlbumCalculator::Impl>())
{
	impl_->set_types(types);
}


ARCSMultifileAlbumCalculator::~ARCSMultifileAlbumCalculator() noexcept
= default;


std::tuple<Checksums, ARId, std::unique_ptr<ToC>>
	ARCSMultifileAlbumCalculator::calculate(
		const std::vector<std::string> &audiofilenames,
		const std::string &metafilename) const
{
	ARCS_LOG_DEBUG << "Calculate result from audiofilenames and metafilename";

	return impl_->calculate(audiofilenames, metafilename);
}


std::tuple<Checksums, ARId, std::unique_ptr<ToC>>
	ARCSMultifileAlbumCalculator::calculate(
			const std::vector<std::string> &audiofilenames,
			const bool &skip_front, const bool &skip_back) const
{
	ARCS_LOG_DEBUG << "Calculate result from audiofilenames "
			" and flags for first and last track";

	auto checksums { impl_->calculate(audiofilenames, skip_front, skip_back) };

	return std::make_tuple(checksums, arcstk::EmptyARId, nullptr);
}


void ARCSMultifileAlbumCalculator::set_types(const ChecksumTypeset& types)
{
	impl_->set_types(types);
}


ChecksumTypeset ARCSMultifileAlbumCalculator::types() const
{
	return impl_->types();
}


void ARCSMultifileAlbumCalculator::set_toc_selection(
		FileReaderSelection *selection)
{
	impl_->set_toc_selection(selection);
}


FileReaderSelection* ARCSMultifileAlbumCalculator::toc_selection() const
{
	return impl_->toc_selection();
}


void ARCSMultifileAlbumCalculator::set_audio_selection(
		FileReaderSelection *selection)
{
	impl_->set_audio_selection(selection);
}


FileReaderSelection* ARCSMultifileAlbumCalculator::audio_selection() const
{
	return impl_->audio_selection();
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

