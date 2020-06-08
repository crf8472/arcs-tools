/**
 * \file tools_calc.cpp Combine ARCS calculation with file handling
 */

#ifndef __ARCSTOOLS_TOOLS_CALC_HPP__
#include "tools-calc.hpp"
#endif

#include <memory>                   // for unique_ptr, make_unique
#include <ostream>                  // for operator<<, basic_ostream, basic_...
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


std::pair<bool,bool> audiofile_layout(const TOC &toc)
{
	auto list { arcstk::toc::get_filenames(toc) };
	std::unordered_set<std::string> set;

	for (const auto& file : list) { set.insert(file); }

	const bool is_single { set.size() == 1 };

	return std::make_pair(is_single, is_single or set.size() == list.size());
}


/**
 * \brief Private implementation of a ARCSMultifileAlbumCalculator
 */
class ARCSMultifileAlbumCalculator::Impl final
{

public:

	/**
	 * \brief Implements ARCSMultifileAlbumCalculator::
	 *
	 * \param[in] metafilename   Metadata file
	 * \param[in] audiofilenames List of audiofile names
	 */
	std::tuple<Checksums, ARId, std::unique_ptr<TOC>> calculate(
			const std::string &metafilename,
			const std::vector<std::string> &audiofilenames) const;

	/**
	 * \brief Implements ARCSMultifileAlbumCalculator::
	 *
	 * \param[in] metafilename Metadata file
	 * \param[in] searchpath   Searchpath for audiofiles
	 */
	std::tuple<Checksums, ARId, std::unique_ptr<TOC>> calculate(
			const std::string &metafilename,
			const std::string &searchpath) const;

	/**
	 * \brief Implements ARCSMultifileAlbumCalculator::
	 *
	 * \param[in] audiofilenames Names of the audiofile
	 * \param[in] skip_front     Skip front samples of first track
	 * \param[in] skip_back      Skip back samples of last track
	 *
	 * \return The AccurateRip checksum of this track
	 */
	Checksums calculate(const std::vector<std::string> &audiofilenames,
			const bool &skip_front, const bool &skip_back) const;

	void set_type(const arcstk::checksum::type &type);

	arcstk::checksum::type type() const;

private:

	arcstk::checksum::type type_;
};


std::tuple<Checksums, ARId, std::unique_ptr<TOC>>
	ARCSMultifileAlbumCalculator::Impl::calculate(
			const std::string &metafilename,
			const std::vector<std::string> &audiofilenames) const
{
	if (audiofilenames.empty())
	{
		if (not metafilename.empty())
		{
			ARCS_LOG_WARNING << "Called multiple audiofile calculation but "
				"no audiofiles provided. "
				"Ignore and proceed with only metafile.";

			return this->calculate(metafilename, std::string());
		}

		const auto msg =
			"Inconsistent input: neither audio files nor a metadata file.";

		ARCS_LOG_ERROR << msg;
		throw std::logic_error(msg);
	}

	TOCParser parser;
	auto toc { parser.parse(metafilename) };

	const int filecount = audiofilenames.size();

	if (toc->track_count() != filecount and filecount != 1) // case: illegal
	{
		std::stringstream ss;
		ss << "Inconsistent input: TOC specifies "
			<< toc->track_count() << " tracks"
			<< " but "
			<< filecount << " audio files were passed.";
		const auto msg = ss.str();

		ARCS_LOG_ERROR << msg;
		throw std::logic_error(msg);
	}

	ARCSCalculator c { type() };

	if (1 == filecount) // case: single-file album with toc
	{
		const auto& [ arcss, arid ] = c.calculate(audiofilenames[0], *toc);

		return std::make_tuple(arcss, arid, std::move(toc));
	}

	if (toc->track_count() == filecount) // case: multi-file album without toc
	{
		ARCS_LOG_INFO << "Specified audio filenames override TOC filenames.";
		ARCS_LOG_INFO << "TOC information is ignored.";

		auto arcss { c.calculate(audiofilenames, true, true) };
		auto arid  { make_arid(*toc) };

		return std::make_tuple(arcss, *arid, std::move(toc));
	}

	return std::make_tuple(Checksums(0), arcstk::EmptyARId, nullptr);
}


std::tuple<Checksums, ARId, std::unique_ptr<TOC>>
	ARCSMultifileAlbumCalculator::Impl::calculate(
		const std::string &metafilename,
		const std::string &audiosearchpath) const
{
	if (metafilename.empty())
	{
		ARCS_LOG_ERROR << "No metafile specified, bail out.";

		return std::make_tuple(Checksums(0), arcstk::EmptyARId, nullptr);
	}

	// TODO Ensure that audiosearchpath ends with the plattform specific
	// file separator, e.g. '/'

	TOCParser parser;
	auto toc { parser.parse(metafilename) };

	// Validate audiofile set

	const auto& [ single_audio_file, pw_distinct ] = audiofile_layout(*toc);

	if (not single_audio_file and not pw_distinct)
	{
		ARCS_LOG_ERROR <<
			"TOC references a set of multiple audio files, but they are not "
			"pairwise distinct. This usecase is unsupported in the current "
			"version ";

		return std::make_tuple(Checksums(0), arcstk::EmptyARId, std::move(toc));
	}

	// Calculate ARCSs

	ARCSCalculator calculator { type() };

	if (single_audio_file)
	{
		ARCS_LOG_DEBUG << "Calculate result from metafilename and audiofile";

		ARCS_LOG_DEBUG << "TOC references single audio file: "
			<< toc->filename(1);

		auto audiofile { toc->filename(1) };

		if (not audiosearchpath.empty())
		{
			// NOTE: there MUST be a platform-specific file separator at the
			// end of audiosearchpath or this will fail
			audiofile.insert(0, audiosearchpath);
		}

		const auto& [ arcss, arid ] = calculator.calculate(audiofile, *toc);
		// case: single-file album with toc

		return std::make_tuple(arcss, arid, std::move(toc));
	} else
	{
		ARCS_LOG_DEBUG << "Calculate result from metafilename (and searchpath)";

		ARCS_LOG_DEBUG << "TOC references multiple files";

		auto audiofiles { arcstk::toc::get_filenames(*toc) };

		if (not audiosearchpath.empty())
		{
			for (auto& filename : audiofiles)
			{
				// NOTE: there MUST be a platform-specific file separator at the
				// end of audiosearchpath or this will fail
				filename.insert(0, audiosearchpath);
			}
		}

		auto arcss { calculator.calculate(audiofiles, true, true) };
		auto arid  { make_arid(*toc) };
		// case: multi-file album with toc

		return std::make_tuple(arcss, *arid, std::move(toc));
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
		const std::string &metafilename,
		const std::vector<std::string> &audiofilenames) const
{
	if (metafilename.empty())
	{
		throw std::invalid_argument("No TOC file specified.");
	}

	if (audiofilenames.empty())
	{
		return impl_->calculate(metafilename, file::path(metafilename));
	}

	return impl_->calculate(metafilename, audiofilenames);
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

