#ifndef __ARCSTOOLS_TOOLS_CALC_HPP__
#define __ARCSTOOLS_TOOLS_CALC_HPP__

/**
 * \file
 *
 * \brief Helper tools for ARCS calculation.
 */

#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#ifndef __LIBARCSTK_IDENTIFIER_HPP__
#include <arcstk/identifier.hpp>
#endif
#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>
#endif

#ifndef __LIBARCSDEC_SELECTION_HPP__
#include <arcsdec/selection.hpp>
#endif

namespace arcsapp
{
namespace calc
{

using arcstk::ARId;
using arcstk::TOC;
using arcstk::Checksums;

using arcsdec::FileReaderSelection;


/**
 * \brief Returns the audiofile layout of a TOC.
 *
 * The first value of the returned tuple is TRUE iff \c toc references either
 * a single audio file or no audio files at all, otherwise FALSE.
 *
 * The second value of the returned tuple is TRUE iff \c toc references a
 * non-empty, pairwise distinct list of audio files, otherwise FALSE.
 *
 * <table>
 *   <tr><td><tt><TRUE, TRUE></tt></td>
 *        <td>only one file</td></tr>
 *   <tr><td><tt><FALSE, TRUE></tt></td>
 *       <td>multiple files, one per track</td></tr>
 *   <tr><td><tt><FALSE, FALSE></tt></td>
 *       <td>multiple files, but some files contain more than 1 track</td></tr>
 *    <tr><td><tt><TRUE, FALSE></tt></td>
 *       <td>no audio files at all</td></tr>
 * </table>
 *
 * The third value is the list of filenames itself. If the TOC contains no
 * filenames, the list is empty. If the TOC contains multiple occurrences of
 * exactly one filename, the list will only contain one entry.
 *
 * \param[in] toc The TOC to analyze
 *
 * \return Flags for audiolayout, list of audio files
 */
std::tuple<bool,bool,std::vector<std::string>> audiofile_layout(const TOC &toc);


/**
 * \brief Wrapper for ARCSCalculator to handle input with multiple audio files.
 */
class ARCSMultifileAlbumCalculator final
{
public:

	/**
	 * \brief Default Constructor
	 *
	 * \param[in] type The default type to request
	 */
	ARCSMultifileAlbumCalculator(const arcstk::checksum::type type);

	/**
	 * \brief Constructor.
	 *
	 * Uses ARCS2 as the default type to request.
	 */
	ARCSMultifileAlbumCalculator() : ARCSMultifileAlbumCalculator(
			arcstk::checksum::type::ARCS2) { /* empty */ };

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~ARCSMultifileAlbumCalculator() noexcept;

	/**
	 * \brief Calculate ARCS values of the CD image represented by the specified
	 * audio files.
	 *
	 * If the metadata file contains any names of audiofiles, they are ignored
	 * in favor of \c audiofilenames. If the list of audiofiles is empty, the
	 * path of the metafilename will be searched for audiofile names specified
	 * within the metafilename.
	 *
	 * \param[in] audiofilenames Name of the audio files
	 * \param[in] metafilename   Name of the metadata file
	 *
	 * \return Checksums, Id and TOC of the image represented by the input files
	 */
	std::tuple<Checksums, ARId, std::unique_ptr<TOC>> calculate(
			const std::vector<std::string> &audiofilenames,
			const std::string &metafilename) const;

	/**
	 * \brief Calculate ARCSs for the given audio files.
	 *
	 * It can be specified that the sequence of audiofiles forms an album by
	 * passing <tt>true</tt> for both boolean parameters.
	 *
	 * The ARCSs in the result will have the same order as the input files,
	 * so for any index i: 0 <= i < audiofilenames.size(), result[i] will be the
	 * result for audiofilenames[i]. The result will have the same size as
	 * audiofilenames.
	 *
	 * Note that in this use case, it is not offered to compute the ARId of the
	 * album since the exact offsets are missing. The ARId returned will be
	 * therefore be empty, the TOC pointer will be nullptr.
	 *
	 * \param[in] audiofilenames Names of the audiofiles
	 * \param[in] skip_front     Skip front samples of first track
	 * \param[in] skip_back      Skip back samples of last track
	 *
	 * \return The AccurateRip checksum of this track
	 */
	std::tuple<Checksums, ARId, std::unique_ptr<TOC>> calculate(
			const std::vector<std::string> &audiofilenames,
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

	// Forward declaration for private implementation.
	class Impl;

	/**
	 * \brief Internal implementation instance
	 */
	std::unique_ptr<Impl> impl_;
};

} // namespace calc
} // namespace arcsapp

#endif

