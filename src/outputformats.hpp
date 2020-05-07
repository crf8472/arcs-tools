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

#ifndef __LIBARCSDEC_DESCRIPTORS_HPP__
#include <arcsdec/descriptors.hpp>
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

using arcsdec::FileReaderDescriptor;


/**
 * \brief Internal flag API
 *
 * Provides 32 boolean states with accessors.
 */
class WithInternalFlags
{

public:

	/**
	 * \brief Constructor
	 *
	 * \param[in] flags Initial internal state
	 */
	WithInternalFlags(const uint32_t flags);

	/**
	 * \brief Set the specified flag to the specified value.
	 *
	 * \param[in] idx   Index to set
	 * \param[in] value Value to set
	 */
	void set_flag(const int idx, const bool value);

	/**
	 * \brief Return the specified flag.
	 *
	 * \param[in] idx   Index to return
	 *
	 * \return The value of the specified flag
	 */
	bool flag(const int idx) const;


private:

	/**
	 * \brief Implementation of the flags
	 */
	uint32_t flags_;
};


/**
 * \brief Abstract base class for output formats of ARId.
 */
class ARIdFormat	: virtual public OutputFormat
					, protected WithInternalFlags
{

public:

	/**
	 * \brief Default constructor.
	 *
	 * Sets all formatting flags to TRUE
	 */
	ARIdFormat();

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
	ARIdFormat(const bool &url, const bool &filename,
			const bool &track_count, const bool &disc_id_1,
			const bool &disc_id_2, const bool &cddb_id);

	/**
	 * \brief Virtual default destructor
	 */
	virtual ~ARIdFormat() noexcept;

	/**
	 * \brief Returns TRUE iff instance is configured to format the URL.
	 *
	 * \return URL flag
	 */
	bool url() const;

	/**
	 * \brief Set to TRUE to print the URL.
	 *
	 * \param[in] url Flag to indicate that the URL has to be printed
	 */
	void set_url(const bool url);

	/**
	 * \brief Returns TRUE iff instance is configured to format the filename.
	 *
	 * \return Filename flag
	 */
	bool filename() const;

	/**
	 * \brief Set to TRUE to print the filename.
	 *
	 * \param[in] filename Flag to indicate that the filename has to be printed
	 */
	void set_filename(const bool filename);

	/**
	 * \brief Returns TRUE iff instance is configured to format the track_count.
	 *
	 * \return Track count flag
	 */
	bool track_count() const;

	/**
	 * \brief Set to TRUE to print the track count.
	 *
	 * \param[in] trackcount Flag to indicate that the track count has to be printed
	 */
	void set_trackcount(const bool trackcount);

	/**
	 * \brief Returns TRUE iff instance is configured to format the disc id 1.
	 *
	 * \return Disc id 1 flag
	 */
	bool disc_id_1() const;

	/**
	 * \brief Set to TRUE to print the first disc id.
	 *
	 * \param[in] disc_id_1 Flag to indicate that the first disc id has to be printed
	 */
	void set_disc_id_1(const bool disc_id_1);

	/**
	 * \brief Returns TRUE iff instance is configured to format the disc id 2.
	 *
	 * \return Disc id 2 flag
	 */
	bool disc_id_2() const;

	/**
	 * \brief Set to TRUE to print the second disc id.
	 *
	 * \param[in] disc_id_2 Flag to indicate that the first disc id has to be printed
	 */
	void set_disc_id_2(const bool disc_id_2);

	/**
	 * \brief Returns TRUE iff instance is configured to format the cddb id.
	 *
	 * \return CDDB id flag
	 */
	bool cddb_id() const;

	/**
	 * \brief Set to TRUE to print the cddb id.
	 *
	 * \param[in] cddb_id Flag to indicate that the cddb id has to be printed
	 */
	void set_cddb_id(const bool cddb_id);

	/**
	 * \brief Format the ARId passed.
	 *
	 * The default URL prefix 'http://www.accuraterip.com/accuraterip/' can
	 * be overriden for output.
	 *
	 * \param[in] id         The ARId to format
	 * \param[in] alt_prefix Override the default URL prefix
	 */
	void format(const ARId &id, const std::string &alt_prefix);


private:

	std::unique_ptr<Lines> do_lines() override;

	/**
	 * \brief Implements format().
	 *
	 * \param[in] id The ARId to format
	 * \param[in] alt_prefix Override the default URL prefix
	 */
	virtual std::unique_ptr<Lines> do_format(const ARId &id,
			const std::string &alt_prefix) const
	= 0;

	/**
	 * \brief Internal representation of the lines
	 */
	std::unique_ptr<Lines> lines_;
};


/**
 * \brief Simple table format for ARId.
 */
class ARIdTableFormat : public ARIdFormat
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

	std::unique_ptr<Lines> do_format(const ARId &id,
			const std::string &alt_prefix) const override;
};


/**
 * \brief Abstract base class for output formats that conatin an ARId.
 *
 * \todo Make a template from this (also see WithChecksums)
 */
class WithARId
{

public:

	/**
	 * \brief Constructor.
	 */
	WithARId();

	/**
	 * \brief Constructor.
	 *
	 * \param[in] arid_format The ARIdFormat to set
	 */
	explicit WithARId(std::unique_ptr<ARIdFormat> arid_format);

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~WithARId() noexcept;

	/**
	 * \brief Set the format to use for formatting the ARId.
	 *
	 * \param[in] format The ARIdFormat to set
	 */
	void set_arid_format(std::unique_ptr<ARIdFormat> arid_format);

	/**
	 * \brief Read the format to use for formatting the ARId.
	 *
	 * \return The internal ARIdFormat
	 */
	ARIdFormat* arid_format();


private:

	/**
	 * \brief Format for the ARId.
	 */
	std::unique_ptr<ARIdFormat> arid_format_;
};


/**
 * \brief Adds flags for showing track, offset, length or filename.
 */
class WithMetadataFlagMethods : protected WithInternalFlags
{

public:

	/**
	 * \brief Constructor.
	 *
	 * \param[in] track    Set to TRUE for printing track number (if any)
	 * \param[in] offset   Set to TRUE for printing offset (if any)
	 * \param[in] length   Set to TRUE for printing length (if any)
	 * \param[in] filename Set to TRUE for printing filename (if any)
	 */
	WithMetadataFlagMethods(const bool track, const bool offset,
			const bool length, const bool filename);

	/**
	 * \brief Virtual default destructor
	 */
	virtual ~WithMetadataFlagMethods() noexcept;

	/**
	 * \brief Returns TRUE iff instance is configured to format the track
	 * number.
	 *
	 * \return Flag for printing the track number
	 */
	bool track() const;

	/**
	 * \brief Activate or deactivate the printing of the track number.
	 *
	 * \param[in] track Flag to set for printing the track number
	 */
	void set_track(const bool &track);

	/**
	 * \brief Returns TRUE iff instance is configured to format the offset.
	 *
	 * \return Flag for printing the offset
	 */
	bool offset() const;

	/**
	 * \brief Activate or deactivate the printing of the offsets.
	 *
	 * \param[in] offset Flag to set for printing the offset
	 */
	void set_offset(const bool &offset);

	/**
	 * \brief Returns TRUE iff instance is configured to format the length.
	 *
	 * \return Flag for printing the length
	 */
	bool length() const;

	/**
	 * \brief Activate or deactivate the printing of the lengths.
	 *
	 * \param[in] length Flag to set for printing the length
	 */
	void set_length(const bool &length);

	/**
	 * \brief Returns TRUE iff instance is configured to format the filename.
	 *
	 * \return Flag for printing the filename
	 */
	bool filename() const;

	/**
	 * \brief Activate or deactivate the printing of the filenames.
	 *
	 * \param[in] filename Flag to set for printing the filename
	 */
	void set_filename(const bool &filename);


protected:

	/**
	 * \brief Returns the track lengths of \c checksums in track order
	 *
	 * \param[in] checksums Checksums to get the track lengths from
	 *
	 * \return The track lengths of \c checksums
	 */
	std::vector<int32_t> get_lengths(const Checksums &checksums) const;

	/**
	 * \brief Format the toc data.
	 *
	 * \param[in] filenames Filenames
	 * \param[in] offsets   Offsets
	 * \param[in] lengths   Lengths
	 *
	 * \return Number of currently used metadata columns
	 */
	virtual int add_metadata(const std::vector<std::string> &filenames,
		const std::vector<int32_t> &offsets,
		const std::vector<int32_t> &lengths)
	= 0;
};


/**
 * \brief A table based format for album data
 */
class AlbumTableBase	: virtual public WithMetadataFlagMethods
						, virtual public WithARId
						, virtual public StringTableBase
{

public:

	enum class COL_TYPE : int
	{
		TRACK    = 1,
		FILENAME = 2,
		OFFSET   = 3,
		LENGTH   = 4,
		CHECKSUM = 5
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
	 * \brief Set widths of all columns with given type
	 */
	void set_widths(const COL_TYPE type, const int width);

	/**
	 * \brief Return TRUE iff the table contains at least 1 column of the
	 * specified type.
	 *
	 * \param[in] type
	 *
	 * \return TRUE iff at least 1 column in the table has the specified type
	 */
	//bool has(const COL_TYPE type) const;


protected:

	/**
	 * \brief Apply types and standard settings to columns
	 */
	int setup_columns();

	/**
	 * \brief Create ordered list of types to print columns for
	 */
	std::vector<arcstk::checksum::type> types_f(const Checksums &checksums)
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
	 * \brief Return number of declared metadata columns.
	 */
	int add_metadata(const std::vector<std::string> &filenames,
		const std::vector<int32_t> &offsets,
		const std::vector<int32_t> &lengths) override; // TODO Remove
};


/**
 * \brief Print the results of a Checksums calculation
 */
class ChecksumsResultPrinter
{
public:

	virtual ~ChecksumsResultPrinter() = default;

	/**
	 * \brief Print the results to the specified stream
	 *
	 * \param[in] checksums checksums
	 * \param[in] filenames filenames
	 * \param[in] toc       TOC (whose filenames are ignored)
	 * \param[in] arid      ARId
	 */
	void out(std::ostream &out, const Checksums &checksums,
			const std::vector<std::string> &filenames,
			const TOC &toc, const ARId &arid);

private:

	virtual void do_out(std::ostream &out, const Checksums &checksums,
			const std::vector<std::string> &filenames,
			const TOC &toc, const ARId &arid)
	= 0;
};


/**
 * \brief Print the results of a Verification
 */
class MatchResultPrinter
{
public:

	virtual ~MatchResultPrinter() = default;

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
			const TOC &toc, const ARId &arid);

private:

	virtual void do_out(std::ostream &out, const Checksums &checksums,
			const std::vector<std::string> &filenames,
			const ARResponse &response,
			const Match &match, const int block, const bool version,
			const TOC &toc, const ARId &arid)
	= 0;
};


// Concrete classes ----


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

	void do_out(std::ostream &out, const Checksums &checksums,
			const std::vector<std::string> &filenames,
			const TOC &toc, const ARId &arid) override;

	/**
	 * \brief Hexadecimal layout used for Checksums columns
	 */
	HexLayout hexlayout_;
};


/**
 * \brief Simple table format for album-based @link Match Matches @endlink.
 */
class AlbumMatchTableFormat final   : public OutputFormat
									, public AlbumTableBase
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

	/**
	 * \brief Format the match information
	 *
	 * \param[in] checksums Checksums to print
	 * \param[in] response  Response to match
	 * \param[in] match     Match information
	 * \param[in] block     The block to print
	 * \param[in] v2        Indicate whether to use v2
	 */
	void format(const Checksums &checksums, const ARResponse &response,
			const Match &match, const int block, const bool v2,
			const ARId &id, const std::unique_ptr<TOC> toc); // TODO Remove

	/**
	 * \brief Formats the match information
	 *
	 * \param[in] checksums Checksums to print
	 * \param[in] response  Response to match
	 * \param[in] match     Match information
	 * \param[in] block     The block to print
	 * \param[in] v2        Indicate whether to use v2
	 * \param[in] filenames Filenames
	 */
	void format(const Checksums &checksums, const ARResponse &response,
			const Match &match, const int block, const bool v2,
			const std::vector<std::string> &filenames); // TODO Remove

private:

	std::unique_ptr<Lines> do_lines() override; // TODO Remove

	void do_out(std::ostream &out, const Checksums &checksums,
			const std::vector<std::string> &filenames,
			const ARResponse &response,
			const Match &match, const int block, const bool version,
			const TOC &toc, const ARId &arid) override;

	/**
	 * \brief Add the checkums match to the table starting in the specified
	 * column.
	 *
	 * If one activates other columns except 'filename' and 'length'
	 * they are printed empty. If the number of checksum types is bigger than
	 * the <tt>columns() - start_col</tt> a crash is likely to happen.
	 *
	 * \param[in] start_col Column to start
	 * \param[in] checksums The Checksums to add
	 * \param[in] response  The Response to add
	 * \param[in] match     The Match to a block from
	 * \param[in] block     The matched block to add
	 * \param[in] version   The ARCS version to add
	 */
	void add_checksums_match(const int start_col,
		const Checksums &checksums, const ARResponse &response,
		const Match &match, const int block, const bool version); // TODO Remove

	/**
	 * \brief Internal line buffer
	 */
	std::unique_ptr<DefaultLines> lines_; // TODO Remove

	/**
	 * \brief Hexadecimal layout used for Checksums columns
	 */
	HexLayout hexlayout_;
};


/**
 * \brief Abstract base class for output formats of ARTriplet.
 */
class ARTripletFormat : virtual public OutputFormat
{

public:

	/**
	 * \brief Empty constructor.
	 */
	ARTripletFormat();

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~ARTripletFormat() noexcept;

	/**
	 * \brief Format the ARTriplet passed.
	 *
	 * \param[in] triplet The ARTriplet to be formatted
	 */
	void format(const unsigned int &track, const ARTriplet &triplet);


private:

	std::unique_ptr<Lines> do_lines() override;

	/**
	 * \brief Implements format().
	 *
	 * \param[in] triplet The ARTriplet to be formatted
	 */
	virtual std::unique_ptr<Lines> do_format(const unsigned int &track,
			const ARTriplet &triplet) const;

	/**
	 * \brief Internal representation of lines
	 */
	std::unique_ptr<Lines> lines_;
};


/**
 * \brief Abstract base class for output formats of ARBlock.
 */
class ARBlockFormat : virtual public OutputFormat, public WithARId
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

	/**
	 * \brief Format the ARBlock passed to lines.
	 *
	 * \param[in] block The ARBlock to format
	 */
	void format(const ARBlock &block);


protected:

	/**
	 * \brief Read the format to use for formatting the ARTriplets of the block.
	 *
	 * \return The internal ARTripletFormat
	 */
	ARTripletFormat* triplet_fmt();


private:

	std::unique_ptr<Lines> do_lines() override;

	/**
	 * \brief Implements format().
	 *
	 * \param[in] block The ARBlock to format
	 */
	virtual std::unique_ptr<Lines> do_format(const ARBlock &block)
	= 0;

	/**
	 * \brief Internal representation of lines.
	 */
	std::unique_ptr<Lines> lines_;

	/**
	 * \brief Internal ARTripletFormat to use.
	 */
	std::unique_ptr<ARTripletFormat> triplet_format_;
};


/**
 * \brief Table format for an ARBlock.
 */
class ARBlockTableFormat : public ARBlockFormat
{

public:

	/**
	 * \brief Constructor.
	 */
	ARBlockTableFormat();

	/**
	 * \brief Destructor.
	 */
	virtual ~ARBlockTableFormat() noexcept;


private:

	/**
	 * \brief Implements format().
	 *
	 * \param[in] block The ARBlock to format
	 */
	virtual std::unique_ptr<Lines> do_format(const ARBlock &block);
};


/**
 * \brief Output format for offsets.
 *
 * For logging and debugging
 */
class OffsetsFormat : virtual public OutputFormat
{

public:

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~OffsetsFormat() noexcept;

	/**
	 * \brief Format the Offsets passed.
	 *
	 * \param[in] offsets The offsets to be formatted
	 */
	void format(const std::vector<uint32_t> &offsets);


private:

	std::unique_ptr<Lines> do_lines() override;

	/**
	 * \brief Implements format().
	 *
	 * \param[in] offsets The offsets to be formatted
	 *
	 * \return The formatted Lines
	 */
	virtual void do_format(const std::vector<uint32_t> &offsets);

	/**
	 * \brief Internal representation of the offsets.
	 */
	std::unique_ptr<Lines> lines_;
};


/**
 * \brief Collect descriptor infos
 */
class FormatCollector
{

public:

	FormatCollector();

	void add(const FileReaderDescriptor &descriptor);

	std::vector<std::array<std::string, 4>> info() const;


private:

	std::vector<std::array<std::string, 4>> info_;
};


/**
 * \brief Print supported formats.
 */
class FormatList :  virtual public OutputFormat,
					virtual public StringTableBase
{

public:

	/**
	 * \brief Constructor
	 */
	FormatList(std::size_t entry_count);

	/**
	 * \brief Inspect the specified descriptor for printable information.
	 */
	void inspect(const FileReaderDescriptor &descriptor);

	/**
	 * \brief Format the metadata/audio format info.
	 *
	 * \param[in] fmt_name
	 * \param[in] lib_name
	 * \param[in] version
	 */
	void format(const std::string &fmt_name, const std::string &lib_name,
			const std::string &desc, const std::string &version);


protected:

	/**
	 * \brief Add a format description
	 */
	void add_data(const std::string &fmt_name, const std::string &lib_name,
			const std::string &desc, const std::string &version);


private:

	int curr_row_;

	std::unique_ptr<Lines> do_lines() override;
};

#endif
