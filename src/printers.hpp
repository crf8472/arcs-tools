#ifndef __ARCSTOOLS_PRINTERS_HPP__
#define __ARCSTOOLS_PRINTERS_HPP__

/**
 * \file
 *
 * \brief Output formats for printing.
 *
 * Provides different output formats for ARId, ARTriplet and formatted lists
 * of TrackARCSs. The convenience formats for ARProfile and ARBlock are composed
 * from these formats. There is also a format for offsets.
 */

#include <array>                  // for array
#include <cstdint>                // for int32_t
#include <iosfwd>                 // for ostream, size_t
#include <string>                 // for string
#include <vector>                 // for vector

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
						, virtual public StringTableStructure
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
template <typename... Args>
class Print
{
public:

	/**
	 * \brief Constructor
	 */
	Print(Args&&... args)
		: objects_ { std::forward<Args>(args)... }
	{
		// empty
	}

	/**
	 * \brief Virtual default destructor
	 */
	virtual ~Print() = default;

	/**
	 * \brief Add the objects to print
	 */
	void use(Args&&... args)
	{
		objects_ = std::make_tuple(std::forward<Args>(args)...);
	}

	/**
	 * \brief Called by overloaded operator <<.
	 *
	 * \param[in] out Stream to print to
	 */
	void out(std::ostream &outstream)
	{
		this->do_out(outstream, this->objects_);
	}

private:

	std::tuple<Args...> objects_;

	virtual void do_out(std::ostream &o, const std::tuple<Args...> &t)
	= 0;
};


/**
 * \brief Overload << to make each concrete Printer be usable with a stream.
 */
template <typename... Args>
std::ostream& operator << (std::ostream &stream, Print<Args...> &p)
{
	p.out(stream);
	return stream;
}


/**
 * \brief Abstract base class for output formats of ARTriplet.
 */
class ARTripletFormat final : virtual protected WithInternalFlags
							, public Print<int, ARTriplet>
{
public:

	/**
	 * \brief Constructor.
	 */
	ARTripletFormat();

private:

	void do_out(std::ostream &out, const std::tuple<int, ARTriplet> &t)
	override;
};


/**
 * \brief Simple table format for ARId.
 */
class ARIdTableFormat final : public ARIdLayout
							, public StringTableStructure
							, public Print<ARId, std::string>
{
public:

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
	ARIdTableFormat(const ARId &id, const std::string &alt_prefix,
			const bool &url, const bool &filename,
			const bool &track_count, const bool &disc_id_1,
			const bool &disc_id_2, const bool &cddb_id);

	ARIdTableFormat(const bool &url, const bool &filename,
			const bool &track_count, const bool &disc_id_1,
			const bool &disc_id_2, const bool &cddb_id) :
		ARIdTableFormat (arcstk::EmptyARId, std::string(), url, filename,
			track_count, disc_id_1, disc_id_2, cddb_id ) { /* empty */ };

	/**
	 * \brief Default constructor.
	 *
	 * Sets all formatting flags to TRUE
	 */
	ARIdTableFormat() : ARIdTableFormat(arcstk::EmptyARId, std::string(),
			true, true, false, false, false, false)
	{ /* empty */ };

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~ARIdTableFormat() noexcept;

private:

	void init(const int rows, const int cols) override;

	void do_out(std::ostream &o, const std::tuple<ARId, std::string> &t)
		override;

	std::string do_format(const ARId &id, const std::string &alt_prefix) const
		override;

	void print_label(std::ostream &out, const ARIdLayout::ARID_FLAG flag) const;

	/**
	 * \brief Row labels
	 */
	const std::array<std::string, to_underlying(ARID_FLAG::COUNT)> row_labels_;

	/**
	 * \brief Iterable array of show flags
	 */
	const std::array<ARID_FLAG,   to_underlying(ARID_FLAG::COUNT)> show_flags_;
};


/**
 * \brief Simple table format for album-based Checksums.
 */
class AlbumChecksumsTableFormat final   : public AlbumTableBase
										, public Print<Checksums*, std::vector<std::string>, TOC*, ARId>
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

	void init(const int rows, const int cols) override;

	int columns_apply_cs_settings(
			const std::vector<arcstk::checksum::type> &types) override;

	void do_out(std::ostream &o,
		const std::tuple<Checksums*, std::vector<std::string>, TOC*, ARId> &t)
		override;

	/**
	 * \brief Hexadecimal layout used for Checksums columns
	 */
	HexLayout hexlayout_;
};


/**
 * \brief Simple table format for album-based @link Match Matches @endlink.
 */
class AlbumMatchTableFormat final   : public AlbumTableBase
									, public Print<Checksums*, std::vector<std::string>, ARResponse, Match*, int, bool, TOC*, ARId>
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

	void init(const int rows, const int cols) override;

	int columns_apply_cs_settings(
			const std::vector<arcstk::checksum::type> &types) override;

	void do_out(std::ostream &out,
			const std::tuple<Checksums*, std::vector<std::string>, ARResponse,
			Match*, int, bool, TOC*, ARId> &t) override;

	/**
	 * \brief Hexadecimal layout used for Checksums columns
	 */
	HexLayout hexlayout_;
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
	 * \param[in] fmt_name Name of the format
	 * \param[in] lib_name Name of the lib
	 * \param[in] desc     Short description
	 * \param[in] version  Version
	 */
	void append_line(const std::string &fmt_name, const std::string &lib_name,
			const std::string &desc, const std::string &version);

private:

	void init(const int rows, const int cols) override;

	int curr_row_;
};

#endif
