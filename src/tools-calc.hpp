#ifndef __ARCSTOOLS_TOOLS_CALC_HPP__
#define __ARCSTOOLS_TOOLS_CALC_HPP__

/**
 * \file
 *
 * \brief Helper tools for ARCS calculation.
 */

#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#include "layouts.hpp"                 // for Layout
#endif

#ifndef __LIBARCSDEC_SELECTION_HPP__
#include <arcsdec/selection.hpp>       // FileReaderSelection
#endif

#ifndef __LIBARCSTK_IDENTIFIER_HPP__
#include <arcstk/identifier.hpp>       // for ARId
#endif
#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>        // for Checksums, checksum::type
#endif

#include <memory>      // for unique_ptr
#include <string>      // for string
#include <tuple>       // for tuple
#include <vector>      // for vector


// forward declarations
namespace arcsdec
{
inline namespace v_1_0_0
{
class ARCSCalculator;
class ToCParser;
} // namespace v_1_0_0
} // namespace arcsdec


namespace arcsapp
{
inline namespace v_1_0_0
{

/**
 * \brief Tools and helpers for managing AccurateRip checksums.
 */
namespace calc
{

using arcstk::ARId;
using arcstk::ToC;
using arcstk::Checksums;

using arcsdec::ARCSCalculator;
using arcsdec::FileReaderSelection;
using arcsdec::ToCParser;

// FIXME This has to be made available from arcsdec/calculators.hpp
using ChecksumTypeset = std::unordered_set<arcstk::checksum::type>;


/**
 * \brief Analyze ToC for filenames and adjust file paths.
 */
struct ToCFiles final
{
	/**
	 * \brief Returns whether the list of names represent the same file.
	 *
	 * The first value of the returned tuple is TRUE iff \p toc references
	 * either a single audio file or no audio files at all, otherwise FALSE.
	 *
	 * The second value of the returned tuple is TRUE iff \p toc references a
	 * non-empty, pairwise distinct list of audio files, otherwise FALSE.
	 *
	 * <table>
	 *   <tr><td><tt><TRUE, TRUE></tt></td>
	 *        <td>only one file</td></tr>
	 *   <tr><td><tt><FALSE, TRUE></tt></td>
	 *       <td>multiple files, one per track</td></tr>
	 *   <tr><td><tt><FALSE, FALSE></tt></td>
	 *       <td>multiple files, but some files contain more than 1 track</td>
	 *       </tr>
	 *    <tr><td><tt><TRUE, FALSE></tt></td>
	 *       <td>no audio files at all</td></tr>
	 * </table>
	 *
	 * \param[in] names List of filenames
	 *
	 * \return Flags for audiolayout
	 */
	static std::tuple<bool,bool> flags(const std::vector<std::string>& names);

	/**
	 * \brief Returns the audiofile layout of a ToC.
	 *
	 * The first and second value are identical to flags().
	 *
	 * The third value is the list of filenames itself. If the ToC contains no
	 * filenames, the list is empty. If the ToC contains multiple occurrences of
	 * exactly one filename, the list will only contain one entry. In all other
	 * cases, the original list will be returned.
	 *
	 * \param[in] toc The ToC to analyze
	 *
	 * \return Flags for audiolayout, list of audio files
	 */
	static std::tuple<bool,bool,std::vector<std::string>> get(const ToC& toc);

	/**
	 * \brief Prepends path of argument 2 with path of argument 1.
	 *
	 * \param[in] metafilename  Name of the metadata file
	 * \param[in] audiofilename Name of the audio file
	 *
	 * \return Expanded file path
	 */
	static std::string expand_path(const std::string& metafilename,
		const std::string& audiofilename);
};


/**
 * \brief Create a selection for a specific FileReader Id.
 */
struct IdSelection final
{
	/**
	 * \brief Create a selection for the specific FileReader id.
	 */
	std::unique_ptr<arcsdec::FileReaderSelection> operator()(
			const std::string& id) const;
};


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"

/**
 * \brief Wrapper for ARCSCalculator to handle input with multiple audio files.
 */
class ChecksumCalculator final
{
public:

	/**
	 * \brief Constructor.
	 *
	 * Uses ARCS2 as the default type to request.
	 */
	ChecksumCalculator();

	/**
	 * \brief Default Constructor
	 *
	 * \param[in] types The checksum types to calculate
	 */
	explicit ChecksumCalculator(const ChecksumTypeset& types);

	/**
	 * \brief Virtual default destructor.
	 */
	~ChecksumCalculator() noexcept;

	/**
	 * \brief Calculate ARCS values of the CD image represented by the specified
	 * audio files.
	 *
	 * If the metadata file contains any names of audiofiles, they are ignored
	 * in favor of \p audiofilenames. If the list of audiofiles is empty, the
	 * path of the metafilename will be searched for audiofile names specified
	 * within the metafilename.
	 *
	 * \param[in] audiofilenames Name of the audio files
	 * \param[in] metafilename   Name of the metadata file
	 *
	 * \return Checksums, Id and ToC of the image represented by the input files
	 */
	std::tuple<Checksums, ARId, std::unique_ptr<ToC>> calculate(
			const std::vector<std::string>& audiofilenames,
			const std::string& metafilename) const;

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
	 * album since the exact offsets are missing. The ARId returned will
	 * therefore be empty, the ToC pointer will be nullptr.
	 *
	 * \param[in] audiofilenames       Names of the audiofiles
	 * \param[in] first_is_first_track Declare first file as first track
	 * \param[in] last_is_last_track   Declare last file as last track
	 *
	 * \return The AccurateRip checksums of these tracks
	 */
	std::tuple<Checksums, ARId, std::unique_ptr<ToC>> calculate(
			const std::vector<std::string>& audiofilenames,
			const bool first_is_first_track, const bool last_is_last_track)
		const;

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
	 * \brief Get the FileReaderSelection used by this instance.
	 *
	 * \return The selection used by this instance
	 */
	FileReaderSelection* audio_selection() const;

	/**
	 * \brief Set the FileReaderSelection for this instance.
	 *
	 * \param[in] selection The selection to use
	 */
	void set_audio_selection(FileReaderSelection* selection);

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
	void set_toc_selection(FileReaderSelection* selection);

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
			std::unique_ptr<ToC> toc, const std::string& searchpath) const;

	/**
	 * \brief Setup internal ARCSCalculator instance.
	 *
	 * \return ARCSCalculator to perform the calculations
	 */
	ARCSCalculator setup_calculator() const;

	/**
	 * \brief Setup internal ToCParser instance.
	 *
	 * \return ToCParser to perform the metadata parsing
	 */
	ToCParser setup_parser() const;

	/**
	 * \brief Checksum type to request
	 */
	ChecksumTypeset types_;

	/**
	 * \brief Internal Audio reader selection.
	 */
	FileReaderSelection* audio_selection_;

	/**
	 * \brief Internal ToC parser selection.
	 */
	FileReaderSelection* toc_selection_;
};

#pragma GCC diagnostic pop


/**
 * \brief Interface for formatting Checksums.
 */
using ChecksumLayout = Layout<std::string, arcstk::Checksum, int>;


/**
 * \brief Format Checksums in hexadecimal representation.
 */
class HexLayout final : protected WithInternalFlags
					  , public    ChecksumLayout
{
public:

	/**
	 * \brief Constructor
	 */
	HexLayout();

	/**
	 * \brief Make the base '0x' visible
	 *
	 * \param[in] base Flag for showing the base
	 */
	void set_show_base(const bool base);

	/**
	 * \brief Return TRUE if the base is shown, otherwise FALSE
	 *
	 * \return TRUE if the base is shown, otherwise FALSE
	 */
	bool shows_base() const;

	/**
	 * \brief Make the hex digits A-F uppercase
	 *
	 * \param[in] base Flag for making hex digits A-F uppercase
	 */
	void set_uppercase(const bool base);

	/**
	 * \brief Return TRUE if A-F are uppercase, otherwise FALSE
	 *
	 * \return TRUE if A-F are uppercase, otherwise FALSE
	 */
	bool is_uppercase() const;

private:

	std::string do_format(InputTuple t) const override;
};


/**
 * \brief Validate the input objects common to every result.
 *
 * Throws if validation fails.
 *
 * \param[in] checksums  Checksums as resulted
 * \param[in] toc        ToC as resulted
 * \param[in] arid       ARId as resulted
 * \param[in] filenames  Filenames as resulted
 *
 * \throws invalid_argument If validation fails
 */
void validate(const Checksums& checksums, const ToC* toc,
	const ARId& arid, const std::vector<std::string>& filenames);


} // namespace calc
} // namespace v_1_0_0
} // namespace arcsapp

#endif

