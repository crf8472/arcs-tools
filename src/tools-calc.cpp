/**
 * \file tools-calc.cpp Combine ARCS calculation with file handling
 */

#ifndef __ARCSTOOLS_TOOLS_CALC_HPP__
#include "tools-calc.hpp"
#endif

#include <memory>                   // for unique_ptr, make_unique
#include <sstream>                  // for ostream
#include <stdexcept>                // for logic_error
#include <string>                   // for string, char_traits, operator<<
#include <tuple>                    // for make_tuple, tuple
#include <unordered_set>            // for unordered_set
#include <utility>                  // for move, make_pair, pair
#include <vector>                   // for vector

#ifndef __LIBARCSTK_IDENTIFIER_HPP__
#include <arcstk/identifier.hpp>
#endif
#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>
#endif
#ifndef __LIBARCSTK_LOGGING_HPP__
#include <arcstk/logging.hpp>
#endif

#ifndef __LIBARCSDEC_CALCULATORS_HPP__
#include <arcsdec/calculators.hpp>
#endif

#ifndef __ARCSTOOLS_TOOLS_FS_HPP__
#include "tools-fs.hpp"             // for path
#endif

namespace arcsapp
{
namespace calc
{

using arcstk::make_empty_arid;

using arcsdec::ARCSCalculator;
using arcsdec::TOCParser;


std::tuple<bool,bool,std::vector<std::string>> audiofile_layout(const TOC &toc)
{
	auto list { arcstk::toc::get_filenames(toc) };

	if (list.empty())
	{
		return std::make_tuple(true, false, std::vector<std::string>{});
	}

	std::unordered_set<std::string> set(list.begin(), list.end());
	const bool is_single { set.size() == 1 };
	return std::make_tuple(is_single, is_single or set.size() == list.size(),
			is_single ? std::vector<std::string>{ *list.cbegin() } : list);
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
	 * \return Checksums, ARId and TOC for the input
	 */
	std::tuple<Checksums, ARId, std::unique_ptr<TOC>> calculate(
			const std::vector<std::string> &audiofilenames,
			const std::string &metafilename) const;

	/**
	 * \brief Calculate ARCS values for audiofilenams without TOC.
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
	void set_type(const arcstk::checksum::type &type);

	/**
	 * \brief The checksum type to be calculated.
	 *
	 * \return Checksum type to be calculated by this instance
	 */
	arcstk::checksum::type type() const;

private:

	/**
	 * \brief Calculate ARCS values for album with audiofilenames from metafile.
	 *
	 * If metafile does not specify any audiofilenames, result will be empty.
	 *
	 * \param[in] metafilename Metadata file
	 * \param[in] searchpath   Searchpath for audiofiles
	 *
	 * \return Checksums, ARId and TOC for the input
	 */
	std::tuple<Checksums, ARId, std::unique_ptr<TOC>> calculate(
			const std::string &metafilename,
			const std::string &searchpath) const;

	/**
	 * \brief Checksum type to request
	 */
	arcstk::checksum::type type_;
};


std::tuple<Checksums, ARId, std::unique_ptr<TOC>>
	ARCSMultifileAlbumCalculator::Impl::calculate(
			const std::vector<std::string> &audiofilenames,
			const std::string &metafilename) const
{
	if (audiofilenames.empty())
	{
		ARCS_LOG_WARNING << "Called multiple audiofile calculation but "
			"no audiofiles provided. "
			"Fall back to extracting audiofilenames from metafile.";

		return this->calculate(metafilename, file::path(metafilename));
	}

	if (metafilename.empty())
	{
		throw std::invalid_argument("No TOC file specified.");
	}

	TOCParser parser;
	auto toc { parser.parse(metafilename) };

	// Validate TOC information

	const int filecount = audiofilenames.size();

	if (toc->track_count() != filecount and filecount != 1) // case: illegal
	{
		std::ostringstream msg;
		msg << "Inconsistent input: Metafile " << metafilename
			<< " specifies " << toc->track_count() << " tracks"
			<< " but " << filecount << " audio files were passed to override.";

		throw std::invalid_argument(msg.str());
	}

	ARCS_LOG_INFO << "Specified audio filenames override TOC filenames."
			" Audiofiles from TOC are ignored.";

	ARCSCalculator c { type() };

	// case: single-file album w TOC
	if (1 == filecount)
	{
		const auto [ checksums, arid ] = c.calculate(audiofilenames[0], *toc);

		return std::make_tuple(checksums, arid, std::move(toc));
	}

	// case: multi-file album w TOC
	if (toc->track_count() == filecount)
	{
		const auto checksums { c.calculate(audiofilenames, true, true) };
		const auto arid      { make_arid(*toc) };

		return std::make_tuple(checksums, *arid, std::move(toc));
	}

	return std::make_tuple(Checksums{ 0 }, arcstk::EmptyARId, nullptr);
}


std::tuple<Checksums, ARId, std::unique_ptr<TOC>>
	ARCSMultifileAlbumCalculator::Impl::calculate(
		const std::string &metafilename,
		const std::string &audiosearchpath) const
{
	if (metafilename.empty())
	{
		throw std::invalid_argument("No TOC file specified.");
	}

	TOCParser parser;
	auto toc { parser.parse(metafilename) };

	// Validate TOC information

	auto [ single_audio_file, pairwise_distinct, audiofiles ] =
		audiofile_layout(*toc);

	if (not single_audio_file and not pairwise_distinct)
	{
		throw std::invalid_argument(
			"TOC references a set of multiple audio files, but they are not "
			"pairwise distinct. This usecase is unsupported in the current "
			"version.");
	}

	ARCS_LOG_DEBUG << "Calculate result from metafilename"
			" and searchpath for audiofiles";

	// Calculate ARCSs

	ARCSCalculator calculator { type() };

	if (single_audio_file)
	{
		auto audiofile { toc->filename(1) };

		if (not audiosearchpath.empty())
		{
			// XXX This assumes that the metafile does not specify any paths
			file::prepend_path(audiosearchpath, audiofile);
		}

		// case: single-file album w TOC
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
	ARCSCalculator calculator { type() };

	return calculator.calculate(audiofilenames, skip_front, skip_back);
}


void ARCSMultifileAlbumCalculator::Impl::set_type(
		const arcstk::checksum::type &type)
{
	type_ = type;
}


arcstk::checksum::type ARCSMultifileAlbumCalculator::Impl::type() const
{
	return type_;
}


// ARCSMultifileAlbumCalculator


ARCSMultifileAlbumCalculator::ARCSMultifileAlbumCalculator(
		const arcstk::checksum::type type)
	: impl_(std::make_unique<ARCSMultifileAlbumCalculator::Impl>())
{
	impl_->set_type(type);
}


ARCSMultifileAlbumCalculator::~ARCSMultifileAlbumCalculator() noexcept
= default;


std::tuple<Checksums, ARId, std::unique_ptr<TOC>>
	ARCSMultifileAlbumCalculator::calculate(
		const std::vector<std::string> &audiofilenames,
		const std::string &metafilename) const
{
	return impl_->calculate(audiofilenames, metafilename);
}


std::tuple<Checksums, ARId, std::unique_ptr<TOC>>
	ARCSMultifileAlbumCalculator::calculate(
			const std::vector<std::string> &audiofilenames,
			const bool &skip_front, const bool &skip_back) const
{
	auto Checksums = impl_->calculate(audiofilenames, skip_front, skip_back);

	return std::make_tuple(Checksums, arcstk::EmptyARId, nullptr);
}


void ARCSMultifileAlbumCalculator::set_type(const arcstk::checksum::type &type)
{
	impl_->set_type(type);
}


arcstk::checksum::type ARCSMultifileAlbumCalculator::type() const
{
	return impl_->type();
}

} // namespace calc
} // namespace arcsapp

