#ifndef __ARCSTOOLS_OUTPUTFORMATS_HPP__
#define __ARCSTOOLS_OUTPUTFORMATS_HPP__

/**
 * \file
 *
 * \brief Output formats for printing.
 *
 * Provides different output formats for ARId, ARTriplet and formatted lists
 * of TrackARCSs. The convenience formats for ARProfile and ARBlock are composed
 * from these formats. There is also a format for offsets.
 */

#include <arcstk/identifier.hpp>
#include <array>
#include <utility>
#include <vector>

#ifndef __LIBARCSTK_PARSE_HPP__
#include <arcstk/parse.hpp>
#endif
#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>
#endif
#ifndef __LIBARCSTK_MATCH_HPP__
#include <arcstk/match.hpp>
#endif

#ifndef __ARCSTOOLS_FORMAT_HPP__
#include "format.hpp"
#endif


using arcstk::ARId;
using arcstk::ARTriplet;
using arcstk::ARBlock;
using arcstk::ARResponse;
using arcstk::Checksums;
using arcstk::Match;
using arcstk::TOC;


/**
 * \brief A table based format for album data
 */
class AlbumTableBase	: public WithMetadataFlagMethods
						, virtual public WithARId
						, virtual public StringTableLayout
{
public:

	/**
	 * \brief Column type
	 */
	enum class COL_TYPE : int
	{
		TRACK    = 1,
		FILENAME = 2,
		OFFSET   = 3,
		LENGTH   = 4,
		CHECKSUM = 5,
		MATCH    = 6
	};

	/**
	 * \brief Constructor.
	 *
	 * \param[in] rows    Number of rows
	 * \param[in] columns Number of columns
	 */
	AlbumTableBase(const int rows, const int columns);

	/**
	 * \brief Constructor.
	 *
	 * \param[in] rows     Number of rows
	 * \param[in] columns  Number of columns
	 * \param[in] track    Set to TRUE for printing track number (if any)
	 * \param[in] offset   Set to TRUE for printing offset (if any)
	 * \param[in] length   Set to TRUE for printing length (if any)
	 * \param[in] filename Set to TRUE for printing filename (if any)
	 */
	AlbumTableBase(const int rows, const int columns,
			const bool track, const bool offset, const bool length,
			const bool filename);

	/**
	 * \brief Return number of declared metadata columns.
	 *
	 * \return Number of declared metadata columns.
	 */
	int total_metadata_columns() const;

	/**
	 * \brief Set column type for specified column
	 */
	void assign_type(const int col, const COL_TYPE type);

	/**
	 * \brief Return type of specified column
	 *
	 * \param[in] col
	 *
	 * \return Type of specified column
	 */
	COL_TYPE type_of(const int col) const;

	/**
	 * \brief Return current default width for columns of the given type
	 */
	int default_width(const COL_TYPE type) const;

	/**
	 * \brief Return current default title for columns of the given type
	 */
	std::string default_title(const COL_TYPE type) const;

	/**
	 * \brief Set widths of all columns with given type
	 */
	void set_widths(const COL_TYPE type, const int width);

protected:

	/**
	 * \brief Apply types and standard settings to columns
	 */
	int columns_apply_settings();

	/**
	 * \brief Create ordered list of types to print columns for
	 */
	std::vector<arcstk::checksum::type> typelist(const Checksums &checksums)
		const;

	/**
	 * \brief Print column titles
	 */
	void print_column_titles(std::ostream &out) const;

	/**
	 * \brief Convert from COL_TYPE to int
	 */
	int convert_from(const COL_TYPE type) const;

	/**
	 * \brief Convert to COL_TYPE from int
	 */
	COL_TYPE convert_to(const int type) const;

	/**
	 * \brief Returns the track lengths of \c checksums in track order
	 *
	 * \param[in] checksums Checksums to get the track lengths from
	 *
	 * \return The track lengths of \c checksums
	 */
	std::vector<int32_t> get_lengths(const Checksums &checksums) const;

private:

	/**
	 * \brief Called by columns_apply_settings after the metadata columns are
	 * initialized
	 *
	 * It is expected that this function initializes the columns that are
	 * typed CHECKSUM and MATCH.
	 */
	virtual int columns_apply_cs_settings(
			const std::vector<arcstk::checksum::type> &types)
	= 0;
};


/**
 * \brief Common base class for printers
 */
class Printer
{
public:

	virtual ~Printer() = default;
};


/**
 * \brief Print an ARId
 */
class ARIdPrinter : public Printer
{
public:

	/**
	 * \brief Print the results to the specified stream
	 *
	 * Ignores the filenames of TOC.
	 *
	 * \param[in] out    Output stream
	 * \param[in] arid   ARId
	 * \param[in] prefix URL prefix
	 */
	void out(std::ostream &out, const ARId &arid, const std::string &prefix);

private:

	virtual void do_out(std::ostream &out, const ARId &arid,
			const std::string &prefix)
	= 0;
};


/**
 * \brief Print an ARTriplet
 */
class ARTripletPrinter : public Printer
{
public:

	/**
	 * \brief Print the results to the specified stream
	 *
	 * Ignores the filenames of TOC.
	 *
	 * \param[in] out     Output stream
	 * \param[in] track   The track represented by \c triplet
	 * \param[in] triplet Triplet to print
	 */
	void out(std::ostream &out, const int track, const ARTriplet &triplet);

private:

	virtual void do_out(std::ostream &out, const int track,
			const ARTriplet &triplet)
	= 0;
};


/**
 * \brief Print an ARBlock
 */
class ARBlockPrinter : public Printer
{
public:

	/**
	 * \brief Print the results to the specifiec stream
	 *
	 * \param[in] out   Output stream
	 * \param[in] block Block to print
	 */
	void out(std::ostream &out, const ARBlock &block);

private:

	virtual void do_out(std::ostream &out, const ARBlock &block)
	= 0;
};


/**
 * \brief Print the results of a Checksums calculation
 */
class ChecksumsResultPrinter : public Printer
{
public:

	/**
	 * \brief Print the results to the specified stream
	 *
	 * Ignores the filenames of TOC.
	 *
	 * \param[in] checksums checksums
	 * \param[in] filenames filenames
	 * \param[in] toc       TOC
	 * \param[in] arid      ARId
	 */
	void out(std::ostream &out, const Checksums &checksums,
			const std::vector<std::string> &filenames,
			const TOC *toc, const ARId &arid);

private:

	virtual void do_out(std::ostream &out, const Checksums &checksums,
			const std::vector<std::string> &filenames,
			const TOC *toc, const ARId &arid)
	= 0;
};


/**
 * \brief Print the results of a Verification
 */
class MatchResultPrinter : public Printer
{
public:

	/**
	 * \brief Print the results to the specified stream
	 *
	 * \param[in] out       Output stream
	 * \param[in] checksums Checksums
	 * \param[in] filenames Filenames
	 * \param[in] response  Response
	 * \param[in] match     Match to print
	 * \param[in] version   Version
	 * \param[in] toc       TOC (whose filenames are ignored)
	 * \param[in] arid      ARId
	 */
	void out(std::ostream &out, const Checksums &checksums,
			const std::vector<std::string> &filenames,
			const ARResponse &response,
			const Match &match, const int block, const bool version,
			const TOC *toc, const ARId &arid);

private:

	virtual void do_out(std::ostream &out, const Checksums &checksums,
			const std::vector<std::string> &filenames,
			const ARResponse &response,
			const Match &match, const int block, const bool version,
			const TOC *toc, const ARId &arid)
	= 0;
};


/**
 * \brief Simple table format for ARId.
 */
class ARIdTableFormat final : public ARIdLayout
							, public StringTableLayout
							, public ARIdPrinter
{
public:

	/**
	 * \brief Default constructor.
	 *
	 * Sets all formatting flags to TRUE
	 */
	ARIdTableFormat();

	/**
	 * \brief Constructor setting all flags.
	 *
	 * \param[in] url         Set to TRUE for printing the URL
	 * \param[in] filename    Set to TRUE for printing the filename
	 * \param[in] track_count Set to TRUE for printing the track_count
	 * \param[in] disc_id_1   Set to TRUE for printing the disc id1
	 * \param[in] disc_id_2   Set to TRUE for printing the disc id2
	 * \param[in] cddb_id     Set to TRUE for printing the cddb id
	 */
	ARIdTableFormat(const bool &url, const bool &filename,
			const bool &track_count, const bool &disc_id_1,
			const bool &disc_id_2, const bool &cddb_id);

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~ARIdTableFormat() noexcept;

private:

	void do_out(std::ostream &out, const ARId &arid,
			const std::string &prefix) override;

	std::string do_format(const ARId &id, const std::string &alt_prefix) const
		override;
};


/**
 * \brief Simple table format for album-based Checksums.
 */
class AlbumChecksumsTableFormat final   : public AlbumTableBase
										, public ChecksumsResultPrinter
{
public:

	/**
	 * \brief Constructor.
	 *
	 * \param[in] rows     Number of tracks
	 * \param[in] columns  Number of types
	 * \param[in] track    Set to TRUE for printing track number (if any)
	 * \param[in] offset   Set to TRUE for printing offset (if any)
	 * \param[in] length   Set to TRUE for printing length (if any)
	 * \param[in] filename Set to TRUE for printing filename (if any)
	 */
	AlbumChecksumsTableFormat(const int rows, const int columns,
			const bool track, const bool offset, const bool length,
			const bool filename);

	/**
	 * \brief Virtual default destructor.
	 */
	~AlbumChecksumsTableFormat() noexcept override;

private:

	int columns_apply_cs_settings(
			const std::vector<arcstk::checksum::type> &types) override;

	void do_out(std::ostream &out, const Checksums &checksums,
			const std::vector<std::string> &filenames,
			const TOC *toc, const ARId &arid) override;

	/**
	 * \brief Hexadecimal layout used for Checksums columns
	 */
	HexLayout hexlayout_;
};


/**
 * \brief Simple table format for album-based @link Match Matches @endlink.
 */
class AlbumMatchTableFormat final   : public AlbumTableBase
									, public MatchResultPrinter
{
public:

	/**
	 * \brief Constructor.
	 *
	 * \param[in] rows     Number of tracks
	 * \param[in] track    Set to TRUE for printing track number (if any)
	 * \param[in] offset   Set to TRUE for printing offset (if any)
	 * \param[in] length   Set to TRUE for printing length (if any)
	 * \param[in] filename Set to TRUE for printing filename (if any)
	 */
	AlbumMatchTableFormat(const int rows, const bool track, const bool offset,
			const bool length, const bool filename);

	/**
	 * \brief Virtual default destructor.
	 */
	~AlbumMatchTableFormat() noexcept override;

private:

	int columns_apply_cs_settings(
			const std::vector<arcstk::checksum::type> &types) override;

	void do_out(std::ostream &out, const Checksums &checksums,
			const std::vector<std::string> &filenames,
			const ARResponse &response,
			const Match &match, const int block, const bool version,
			const TOC *toc, const ARId &arid) override;

	/**
	 * \brief Hexadecimal layout used for Checksums columns
	 */
	HexLayout hexlayout_;
};


/**
 * \brief Abstract base class for output formats of ARTriplet.
 */
class ARTripletFormat final : virtual protected WithInternalFlags
							, public ARTripletPrinter
{
private:

	/**
	 * \brief Implements out().
	 *
	 * \param[in] out     The stream to print
	 * \param[in] track   The track represented by \c triplet
	 * \param[in] triplet The ARTriplet to be formatted
	 */
	void do_out(std::ostream &out, const int track, const ARTriplet &triplet)
		override;
};


/**
 * \brief Abstract base class for output formats of ARBlock.
 */
class ARBlockFormat : virtual public WithARId
{
public:

	/**
	 * \brief Constructor.
	 */
	ARBlockFormat();

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~ARBlockFormat() noexcept;

	/**
	 * \brief Set the format to use for formatting the ARTriplets of the block.
	 *
	 * \param[in] format The ARTripletFormat to set
	 */
	void set_triplet_format(std::unique_ptr<ARTripletFormat> format);

	/**
	 * \brief Read the format to use for formatting the ARId.
	 *
	 * \return The internal ARTripletormat
	 */
	const ARTripletFormat& triplet_format() const;

protected:

	/**
	 * \brief Read the format to use for formatting the ARTriplets of the block.
	 *
	 * \return The internal ARTripletFormat
	 */
	ARTripletFormat* triplet_fmt();

private:

	/**
	 * \brief Internal ARTripletFormat to use.
	 */
	std::unique_ptr<ARTripletFormat> triplet_format_;
};


/**
 * \brief Table format for an ARBlock.
 */
class ARBlockTableFormat : public ARBlockFormat
						 , public ARBlockPrinter
{
private:

	void do_out(std::ostream &out, const ARBlock &block) override;
};


/**
 * \brief Print supported formats.
 */
class FormatList : public StringTable
{
public:

	/**
	 * \brief Constructor
	 */
	FormatList(std::size_t rows);

	/**
	 * \brief Format the metadata/audio format info.
	 *
	 * \param[in] fmt_name
	 * \param[in] lib_name
	 * \param[in] version
	 */
	void append_line(const std::string &fmt_name, const std::string &lib_name,
			const std::string &desc, const std::string &version);

private:

	int curr_row_;
};

#endif

