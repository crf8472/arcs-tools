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

namespace arcsapp
{
namespace calc
{

using arcstk::ARId;
using arcstk::TOC;
using arcstk::Checksums;


/**
 * \brief Returns the audiofile layout of a TOC.
 *
 * The first value of the returned pair is TRUE iff \c toc references a single
 * audio file, otherwise FALSE.
 *
 * The second value of the returned pair is TRUE iff \c toc references a
 * pairwise distinct list of audio files, otherwise FALSE.
 *
 * <tt><TRUE, TRUE></tt> : only one file
 *
 * <tt><FALSE, TRUE></tt> : multiple files, one per track
 *
 * <tt><FALSE, FALSE></tt> : multiple files, but some files contain more than
 *							one track
 *
 * <tt><TRUE, FALSE></tt> : impossible
 *
 * \param[in] toc The TOC to analyze
 *
 * \return TRUE iff \c toc references a single audio file.
 */
std::pair<bool,bool> audiofile_layout(const TOC &toc);


/**
 * \brief Wrapper for ARCSCalculator to handle input with multiple audio files.
 *
 *
 */
class ARCSMultifileAlbumCalculator final
{
public:

	/**
	 * \brief Constructor.
	 */
	ARCSMultifileAlbumCalculator();

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
	 * function will return an empty result.
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
	 * \brief Calculate ARCS values of the CD image represented by the specified
	 * metadata file and search audiofiles in the searchpath.
	 *
	 * The searchpath must end with a file separator.
	 *
	 * \param[in] metafilename   Name of the metadata file
	 * \param[in] searchpath     Name of the searchpath for audio files
	 *
	 * \return Checksums, Id and TOC of the image represented by the input files
	 */
	std::tuple<Checksums, ARId, std::unique_ptr<TOC>> calculate(
			const std::string &metafilename, const std::string &searchpath)
			const;

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

