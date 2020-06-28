#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#define __ARCSTOOLS_LAYOUTS_HPP__

/**
 * \file
 *
 * \brief Interface to output formats for printing.
 *
 * Provides OutputFormat, an abstract base class that encapsulates all actual
 * details for formatting the output. After different format() methods may have
 * been called, the actual output lines can be requested as an instance of
 * Lines, a class that represents a sequence of text lines to be printed to an
 * output stream from first to last.
 */

#include <cstdint>                // for uint32_t, uint8_t
#include <istream>                // for size_t, ostream
#include <memory>                 // for unique_ptr
#include <string>                 // for string
#include <type_traits>            // for underlying_type_t

#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>
#endif
#ifndef __LIBARCSTK_IDENTIFIER_HPP__
#include <arcstk/identifier.hpp>
#endif

namespace arcsapp
{

using arcstk::ARId;
using arcstk::Checksums;


/**
 * \brief Return underlying value of an integral enum
 */
template <typename E>
inline constexpr auto to_underlying(E e) noexcept
{
    return static_cast<std::underlying_type_t<E>>(e);
}


/**
 * \brief Minimal fitting width (= longest entry) of a string column
 */
template <class Container, typename = arcstk::IsFilenameContainer<Container>>
inline int optimal_width(Container&& list)
{
	std::size_t width = 0;

	for (const auto& entry : list)   // TODO Do this with STL, e.g. std::max!
	{
		if (entry.length() > width)
		{
			width = entry.length();
		}
	}

	return width;
}


/**
 * \brief Interface for formatting numbers
 */
class NumberLayout
{
public:

	/**
	 * \brief Virtual default destructor
	 */
	virtual ~NumberLayout() noexcept;

	/**
	 * \brief Layout an unsigned 32 bit integer
	 *
	 * \param[in] number  Number to format
	 * \param[in] width   Width to format
	 */
	std::string format(const uint32_t &number, const int width) const;

private:

	/**
	 * \brief Implements NumberLayout::(const uint32_t &, const int) const
	 *
	 * \param[in] number  Number to format
	 * \param[in] width   Width to format
	 */
	virtual std::string do_format(const uint32_t &number, const int width) const
	= 0;
};


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
	 * \brief Default Constructor
	 */
	WithInternalFlags() : WithInternalFlags(0) { /* empty */ };

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

	/**
	 * \brief Return true if no flags are set.
	 *
	 * \return TRUE if no flags are set
	 */
	bool no_flags() const;

	/**
	 * \brief Return TRUE if only one flag is set, otherwise FALSE
	 *
	 * \return TRUE if only one flag is set, otherwise FALSE
	 */
	bool only_one_flag() const;

	/**
	 * \brief Return TRUE if the specified flag is the only flag set
	 *
	 * \param[in] idx   Index to return
	 *
	 * \return TRUE iff the specified flag is set an all others are not
	 */
	bool only(const int idx) const;

private:

	/**
	 * \brief Implementation of the flags
	 */
	uint32_t flags_;
};


/**
 * \brief Format numbers in hexadecimal representation
 */
class HexLayout : public NumberLayout
				, private WithInternalFlags
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

	std::string do_format(const uint32_t &number, const int width) const
		override;
};


/**
 * \brief Property to set or use a layout for printing checksums.
 */
class WithChecksumLayout
{
public:

	/**
	 * \brief Default constructor
	 *
	 * Initializes a default HexLayout.
	 */
	WithChecksumLayout();

	virtual ~WithChecksumLayout() noexcept;

	/**
	 * \brief Set the layout for printing the checksums
	 *
	 * \param[in] layout Layout for printing the checksums
	 */
	void set_checksum_layout(std::unique_ptr<NumberLayout> &layout);

	/**
	 * \brief Return the layout for printing the checksums
	 *
	 * \return Layout for printing the checksums
	 */
	const NumberLayout& checksum_layout() const;

private:

	/**
	 * \brief Layout used for CHECKSUM columns to print the checksums
	 */
	std::unique_ptr<NumberLayout> checksum_layout_;
};


/**
 * \brief Abstract base class for output formats of ARId.
 */
class ARIdLayout : protected WithInternalFlags
				 , public WithChecksumLayout
{
public:

	/**
	 * \brief Show flags of the ARIdLayout
	 */
	enum class ARID_FLAG : int
	{
		URL      = 0,
		FILENAME = 1,
		TRACKS   = 2,
		ID1      = 3,
		ID2      = 4,
		CDDBID   = 5,
		COUNT    = 6
	};

	/**
	 * \brief Default constructor.
	 *
	 * Sets all formatting flags to TRUE
	 */
	ARIdLayout();

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
	ARIdLayout(const bool &url, const bool &filename,
			const bool &track_count, const bool &disc_id_1,
			const bool &disc_id_2, const bool &cddb_id);

	/**
	 * \brief Virtual default destructor
	 */
	virtual ~ARIdLayout() noexcept;

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
	 * \brief Return TRUE if \c flag is the only flag set, otherwise FALSE
	 *
	 * \param[in] flag Flag to check
	 *
	 * \return TRUE iff \c flag is the only flag set, otherwise FALSE
	 */
	bool has_only(const ARID_FLAG flag) const;

	/**
	 * \brief Format the ARId passed.
	 *
	 * The default URL prefix 'http://www.accuraterip.com/accuraterip/' can
	 * be overriden for output.
	 *
	 * \param[in] id         The ARId to format
	 * \param[in] alt_prefix Override the default URL prefix
	 */
	std::string format(const ARId &id, const std::string &alt_prefix) const;

private:

	/**
	 * \brief Implements format().
	 *
	 * \param[in] id The ARId to format
	 * \param[in] alt_prefix Override the default URL prefix
	 */
	virtual std::string do_format(const ARId &id, const std::string &alt_prefix)
		const
	= 0;
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
	 * \param[in] arid_layout The ARIdLayout to set
	 */
	explicit WithARId(std::unique_ptr<ARIdLayout> arid_layout);

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~WithARId() noexcept;

	/**
	 * \brief Set the format to use for formatting the ARId.
	 *
	 * \param[in] arid_layout The ARIdLayout to set
	 */
	void set_arid_layout(std::unique_ptr<ARIdLayout> arid_layout);

	/**
	 * \brief Read the format to use for formatting the ARId.
	 *
	 * \return The internal ARIdLayout
	 */
	ARIdLayout* arid_layout();

private:

	/**
	 * \brief Format for the ARId.
	 */
	std::unique_ptr<ARIdLayout> arid_layout_;
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
	 * \param[in] label    Set to TRUE for printing labels
	 * \param[in] track    Set to TRUE for printing track number (if any)
	 * \param[in] offset   Set to TRUE for printing offset (if any)
	 * \param[in] length   Set to TRUE for printing length (if any)
	 * \param[in] filename Set to TRUE for printing filename (if any)
	 */
	WithMetadataFlagMethods(const bool label, const bool track,
			const bool offset, const bool length, const bool filename);

	/**
	 * \brief Default constructor.
	 *
	 * Constructs an instance with all flags FALSE.
	 */
	WithMetadataFlagMethods() : WithMetadataFlagMethods(
			false, false, false, false, false) {}

	/**
	 * \brief Virtual default destructor
	 */
	virtual ~WithMetadataFlagMethods() noexcept;

	/**
	 * \brief Returns TRUE iff instance is configured to format the label.
	 *
	 * Intended to control the printing of column titles and row labels.
	 *
	 * \return Flag for printing the label
	 */
	bool label() const;

	/**
	 * \brief Activate or deactivate the printing of labels.
	 *
	 * Intended to control the printing of column titles and row labels.
	 *
	 * \param[in] filename Flag to set for printing the filename
	 */
	void set_label(const bool &label);

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
};


/**
 * \brief A table with formatted columns
 */
class TableStructure
{
public:

	/**
	 * \brief Virtual destructor.
	 */
	virtual ~TableStructure() noexcept;

	/**
	 * \brief Returns the number of rows (without header)
	 *
	 * \return The number of rows
	 */
	std::size_t rows() const;

	/**
	 * \brief Returns the number of columns
	 *
	 * \return The number of columns
	 */
	std::size_t columns() const;

	/**
	 * \brief Set the title of this table.
	 *
	 * \param[in] title Title of this table
	 */
	void set_table_title(const std::string &title);

	/**
	 * \brief Get the title of this table.
	 *
	 * \return Title of this table
	 */
	const std::string& table_title() const;

	/**
	 * \brief Set the column width
	 *
	 * \param[in] col   Column index
	 * \param[in] width Width of the column
	 */
	void set_width(const int col, const int width);

	/**
	 * \brief Get width of a column
	 *
	 * \param[in] col The column to get the width of
	 *
	 * \return Width of the column
	 *
	 * \throws std::out_of_range If col > columns() or col < 0.
	 */
	int width(const int col) const;

	/**
	 * \brief Set alignment for the specified column
	 *
	 * \param[in] col   Column index
	 * \param[in] align Alignment
	 */
	void set_alignment(const int col, const bool align);

	/**
	 * \brief Alignment of the specified column
	 *
	 * \param[in] col   Column index
	 *
	 * \return Alignment of this column
	 */
	bool alignment(const int col) const;

	/**
	 * \brief Set the column type
	 *
	 * \param[in] col   Column index
	 * \param[in] type  The column type
	 */
	void set_type(const int col, const int type);

	/**
	 * \brief Get type of a column
	 *
	 * \param[in] col The column to get the type of
	 *
	 * \return Type of the column
	 *
	 * \throws std::out_of_range If col > columns() or col < 0.
	 */
	int type(const int col) const;

	/**
	 * \brief Set the column name
	 *
	 * \param[in] col   Column index
	 * \param[in] title The column title
	 */
	void set_title(const int col, const std::string &title);

	/**
	 * \brief Get name of a column
	 *
	 * \param[in] col The column to get the name of
	 *
	 * \return Name of the column
	 *
	 * \throws std::out_of_range If col > columns() or col < 0.
	 */
	std::string title(const int col) const;

	/**
	 * \brief Set the row label
	 *
	 * \param[in] row   Row index
	 * \param[in] label The row label
	 */
	void set_row_label(const int row, const std::string &label);

	/**
	 * \brief Get label of row
	 *
	 * \param[in] row The row to get the label of
	 *
	 * \return Label of the row
	 *
	 * \throws std::out_of_range If row > rows()
	 */
	std::string row_label(const int row) const;

	/**
	 * \brief Set the column delimiter symbol
	 *
	 * \param[in] delim The column delimiter symbol
	 */
	void set_column_delimiter(const std::string &delim);

	/**
	 * \brief Returns the column delimiter symbol
	 *
	 * \return The column delimiter
	 */
	std::string column_delimiter() const;

	/**
	 * \brief Reset the dimensions of the table
	 *
	 * \param[in] rows Number of rows
	 * \param[in] cols Number of columns
	 */
	void resize(const int rows, const int cols);

	/**
	 * \brief Perform a bounds check and throw std::out_of_range on illegal
	 * values.
	 *
	 * \param[in] row  Row index
	 * \param[in] col  Column index
	 *
	 * \throws std::out_of_range If col > columns() or col < 0.
	 * \throws std::out_of_range If row > rows() or row < 0.
	 */
	void bounds_check(const int row, const int col) const;

private:

	/**
	 * \brief Implements TableStructure::rows()
	 *
	 * \return The number of rows
	 */
	virtual std::size_t do_rows() const
	= 0;

	/**
	 * \brief Implements TableStructure::columns()
	 *
	 * \return The number of columns
	 */
	virtual std::size_t do_columns() const
	= 0;

	/**
	 * \brief Description.
	 *
	 * \param[in] title Title of the table
	 */
	virtual void do_set_table_title(const std::string &title)
	= 0;

	/**
	 * \brief Return the title of the table.
	 *
	 * \return Title of the table
	 */
	virtual const std::string& do_table_title() const
	= 0;

	/**
	 * \brief Implements TableStructure::set_width(const int, const int)
	 *
	 * \param[in] col  The column id
	 * \param[in] width Widht of the column
	 */
	virtual void do_set_width(const int col, const int width)
	= 0;

	/**
	 * \brief Implements TableStructure::width(const int)
	 *
	 * \param[in] col The column to get the width of
	 *
	 * \return Width of the column
	 *
	 * \throws std::out_of_range If col > columns() or col < 0.
	 */
	virtual int do_width(const int col) const
	= 0;

	/**
	 * \brief Implements TableStructure::set_alignment(const int, const bool)
	 *
	 * \param[in] col   Column index
	 * \param[in] align Alignment
	 */
	virtual void do_set_alignment(const int col, const bool align)
	= 0;

	/**
	 * \brief Implements TableStructure::alignment(const int)
	 *
	 * \param[in] col   Column index
	 *
	 * \return Alignment of this column
	 */
	virtual bool do_alignment(const int col) const
	= 0;

	/**
	 * \brief Implements TableStructure::set_type(const int, const int)
	 *
	 * \param[in] col  Column index
	 * \param[in] name Column type
	 */
	virtual void do_set_type(const int col, const int type)
	= 0;

	/**
	 * \brief Implements TableStructure::type(const int)
	 *
	 * \param[in] col The column to get the type of
	 *
	 * \return Type of the column
	 *
	 * \throws std::out_of_range If col > columns or col < 0.
	 */
	virtual int do_type(const int col) const
	= 0;

	/**
	 * \brief Implements TableStructure::set_title(const int, const std::string&)
	 *
	 * \param[in] col   Column index
	 * \param[in] title Column title
	 */
	virtual void do_set_title(const int col, const std::string &title)
	= 0;

	/**
	 * \brief Implements TableStructure::title(const int)
	 *
	 * \param[in] col The column to get the title of
	 *
	 * \return Title of the column
	 *
	 * \throws std::out_of_range If col > columns or col < 0.
	 */
	virtual std::string do_title(const int col) const
	= 0;

	/**
	 * \brief Implements TableStructure::set_row_label(const int, const std::string&)
	 *
	 * \param[in] row   Row index
	 * \param[in] label Row label
	 */
	virtual void do_set_row_label(const int row, const std::string &label)
	= 0;

	/**
	 * \brief Implements TableStructure::label(const int)
	 *
	 * \param[in] row The row to get the label of
	 *
	 * \return Label of the row
	 *
	 * \throws std::out_of_range If row > rows()
	 */
	virtual std::string do_row_label(const int row) const
	= 0;

	/**
	 * \brief Implements TableStructure::set_column_delimiter(const std::string&)
	 *
	 * \param[in] delim The column delimiter symbol
	 */
	virtual void do_set_column_delimiter(const std::string &delim)
	= 0;

	/**
	 * \brief Implements TableStructure::column_delimiter()
	 *
	 * \return The column delimiter
	 */
	virtual std::string do_column_delimiter() const
	= 0;

	/**
	 * \brief Implements TableStructure::resize(const int, const int)
	 *
	 * \param[in] rows  Number of rows
	 * \param[in] cols  Number of columns
	 */
	virtual void do_resize(const int rows, const int cols)
	= 0;

	/**
	 * \brief Perform a bounds check
	 *
	 * \param[in] row  Row index
	 * \param[in] col  Column index
	 *
	 * \throws std::out_of_range If col > columns() or col < 0.
	 * \throws std::out_of_range If row > rows() or row < 0.
	 */
	virtual void do_bounds_check(const int row, const int col) const
	= 0;
};


/**
 * \brief Base class for a table of strings layout.
 *
 * Implement function \c do_init() in a subclass to get a concrete layout.
 * The default implementation of do_init() is empty in StringTable.
 *
 * \see StringTable
 */
class StringTableStructure : public TableStructure
{
public:

	/**
	 * \brief Constructor
	 *
	 * \param[in] rows Number of rows (including header, if any)
	 * \param[in] cols Number of columns (including label column, if any)
	 */
	StringTableStructure(const int rows, const int cols);

	/**
	 * \brief Default constructor constructs a table with dimensions 0,0
	 */
	StringTableStructure() : StringTableStructure(0, 0) { /* empty */ }

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~StringTableStructure() noexcept;

	/**
	 * \brief Initialize the instance with the specified number of
	 * rows and columns.
	 *
	 * \param[in] rows Number of rows
	 * \param[in] cols Number of columns
	 */
	void init(const int rows, const int cols);

protected:

	/**
	 * \brief Prints the title of the table.
	 *
	 * \param[in] out The stream to print to
	 */
	void print_title(std::ostream &out) const;

	/**
	 * \brief Worker to print column titles.
	 *
	 * Default implementation prints title(column) for each column
	 * with alignment to the right side.
	 *
	 * \param[in] out The stream to print to
	 */
	void print_column_titles(std::ostream &out) const;

	/**
	 * \brief Worker to print a row label
	 *
	 * Default implementation prints row_label(row) with alignment to the right
	 * side.
	 *
	 * \param[in] out The stream to print to
	 * \param[in] row Current row
	 */
	void print_label(std::ostream &out, const int row) const;

	/**
	 * \brief Worker to print a cell.
	 *
	 * Default implementation prints \c text using \c width(col) and
	 * \c alignment(col).
	 *
	 * \param[in] out        The stream to print to
	 * \param[in] col        Current column
	 * \param[in] text       Text to print
	 * \param[in] with_delim TRUE prints a column delimiter, FALSE skips it
	 */
	void print_cell(std::ostream &out, const int col, const std::string &text,
		const bool with_delim) const;

	/**
	 * \brief Return the optimal width for the label column.
	 *
	 * The optimal width is based only on the labels that would be printed.
	 *
	 * \return Optimal width for label column
	 */
	int optimal_label_width() const;

	/**
	 * \brief Resize the dimensions of the layout
	 *
	 * \param[in] rows New number of rows
	 * \param[in] cols New number of columns
	 */
	void update_dimensions(const int rows, const int cols) const;

	/**
	 * \brief TRUE if \c row is within the dimensions
	 *
	 * \param[in] row Row index to check
	 *
	 * \return TRUE if \c row is within the dimensions, otherwise FALSE
	 */
	bool legal_row(const int row) const;

	/**
	 * \brief TRUE if \c col is within the dimensions
	 *
	 * \param[in] col Column index to check
	 *
	 * \return TRUE if \c col is within the dimensions, otherwise FALSE
	 */
	bool legal_col(const int col) const;

	/**
	 * \brief Throws if \c legal_row(row) is FALSE
	 *
	 * \param[in] row Row index to check
	 *
	 * \throws std::out_of_range If not \c legal_row(row)
	 */
	void bounds_check_row(const int row) const;

	/**
	 * \brief Throws if \c legal_col(col) is FALSE
	 *
	 * \param[in] col Column index to check
	 *
	 * \thcols std::out_of_range If not \c legal_col(col)
	 */
	void bounds_check_col(const int col) const;


	StringTableStructure(const StringTableStructure &rhs);

	StringTableStructure(StringTableStructure &&rhs) noexcept;

private:

	virtual void do_init(const int rows, const int cols)
	= 0;

	std::size_t do_rows() const override;
	std::size_t do_columns() const override;

	void do_set_table_title(const std::string &title) override;
	const std::string& do_table_title() const override;

	void do_set_width(const int col, const int width) override;
	int do_width(const int col) const override;

	void do_set_alignment(const int col, const bool align) override;
	bool do_alignment(const int col) const override;

	void do_set_type(const int col, const int type) override;
	int do_type(const int col) const override;

	void do_set_title(const int col, const std::string &name) override;
	std::string do_title(const int col) const override;

	void do_set_row_label(const int row, const std::string &label) override;
	std::string do_row_label(const int row) const override;

	void do_set_column_delimiter(const std::string &delim) override;
	std::string do_column_delimiter() const override;

	void do_resize(const int rows, const int cols) override;
	void do_bounds_check(const int row, const int col) const override;

	// forward declaration
	class Impl;

	/**
	 * \brief Private implementation
	 */
	std::unique_ptr<Impl> impl_;
};


class StringTableBase;

std::ostream& operator << (std::ostream &o, const StringTableBase &table);

/**
 * \brief Base class for tables of strings.
 *
 * This class is not intended for polymorphic use.
 */
class StringTableBase : public StringTableStructure
{
public:

	friend std::ostream& operator<< (std::ostream &o,
			const StringTableBase &table);

	/**
	 * \brief Constructor
	 *
	 * \param[in] rows    Number of rows (including header, if any)
	 * \param[in] columns Number of columns (including label column, if any)
	 * \param[in] dyn_column_width  If 'FALSE' cells are truncated on col width
	 * \param[in] allow_append_rows If 'FALSE' appending rows is forbidden
	 */
	StringTableBase(const int rows, const int columns,
			const bool dyn_column_width, const bool allow_append_rows);

	/**
	 * \brief Constructor
	 *
	 * Activates dynamic optimal table width and appending of rows.
	 *
	 * \param[in] rows    Number of rows (including header, if any)
	 * \param[in] columns Number of columns (including label column, if any)
	 */
	StringTableBase(const int rows, const int columns)
		: StringTableBase(rows, columns, true, true) { /* empty */ };

	/**
	 * \brief Default constructor
	 *
	 * Constructs a table with dimensions 0,0, activates appending rows and
	 * dynamic column widths.
	 */
	StringTableBase() : StringTableBase(0, 0) { /* empty */ }

	/**
	 * \brief Default destructor
	 */
	virtual ~StringTableBase() noexcept;

	/**
	 * \brief TRUE if the table has dynamic column widths activated.
	 *
	 * \return TRUE if dynamic column widths are activated, otherwise FALSE
	 */
	bool has_dynamic_widths() const;

	/**
	 * \brief TRUE if the table has row appending activated.
	 *
	 * \return TRUE if row appending is activated, otherwise FALSE.
	 */
	bool has_appending_rows() const;

	/**
	 * \brief Updates an existing cell with specified text.
	 *
	 * Performs bounds checking and throws on illegal values.
	 *
	 * \param[in] row  Row index to access
	 * \param[in] col  Column index to access
	 * \param[in] text New text for specified cell
	 *
	 * \throws std::out_of_range If col > columns() or col < 0.
	 */
	void update_cell(const int row, const int col, const std::string &text);

	/**
	 * \brief Returns the content of the specified cell.
	 *
	 * The result is equivalent to \c (row, col) if \c row and \c col
	 * have legal values, but \c cell() performs bounds checking and
	 * throws on illegal values.
	 *
	 * \param[in] row Row index to access
	 * \param[in] col Column index to access
	 *
	 * \return Content of the specified cell
	 *
	 * \throws std::out_of_range If col > columns() or col < 0.
	 */
	std::string cell(const int row, const int col) const;

	/**
	 * \brief Access operator.
	 *
	 * The result is equivalent to \c cell(row, col) if \c row and \c col
	 * have legal values, but there is no bounds check performed.
	 *
	 * \param[in] row Row index to access
	 * \param[in] col Column index to access
	 *
	 * \return Content of the specified cell
	 */
	std::string operator() (const int row, const int col) const;

	/**
	 * \brief Row index of the first row that is not filled
	 *
	 * \return Current 0-based row index
	 */
	int current_row() const;

	/**
	 * \brief Return maximum number of rows
	 *
	 * \return Maximum number of rows
	 */
	int max_rows() const;

	/**
	 * \brief Return TRUE if the table contains no content.
	 *
	 * \return TRUE if the table has no content, otherwise FALSE
	 */
	bool empty() const;

protected:

	StringTableBase(const StringTableBase &rhs);

	StringTableBase(StringTableBase &&rhs) noexcept;

	StringTableBase& operator = (const StringTableBase &rhs);

	StringTableBase& operator = (StringTableBase &&rhs) noexcept;

	// TODO swap

	// TODO ==

private:

	void do_resize(const int rows, const int cols) override;
	// from TableStructure

	/**
	 * \brief Implements StringTableBase::update_cell(const int, const int)
	 *
	 * \param[in] row  Row index
	 * \param[in] col  Column index
	 * \param[in] text New cell text
	 *
	 * \throws std::out_of_range If col > columns() or col < 0.
	 */
	virtual void do_update_cell(const int row, const int col,
			const std::string &text);

	/**
	 * \brief Implements StringTableBase::cell(const int, const int)
	 *
	 * \param[in] row  Row index
	 * \param[in] col  Column index
	 *
	 * \throws std::out_of_range If col > columns() or col < 0.
	 *
	 * \return Content of cell(row, col)
	 */
	virtual std::string do_cell(const int row, const int col) const;

	/**
	 * \brief Holds the configuration flags.
	 */
	std::vector<bool> flags_;

	// forward declaration
	class Impl;

	/**
	 * \brief Private implementation
	 */
	std::unique_ptr<Impl> impl_;
};


/**
 * \brief A table of strings.
 *
 * The columns are by default configured to have always optimal width.
 * The table is by default allowed to have rows appended.
 */
class StringTable : public StringTableBase
{
public:

	using StringTableBase::StringTableBase;
	using StringTableBase::operator=;

private:

	void do_init(const int rows, const int cols) override;
	// from StringTableStructure
};


/**
 * \brief Cell types for album information
 */
enum class CELL_TYPE : int
{
	TRACK    = 1,
	FILENAME = 2,
	OFFSET   = 3,
	LENGTH   = 4,
	CHECKSUM = 5,
	MATCH    = 6
};


/**
 * \brief Default values
 */
namespace defaults
{

/**
 * \brief Return default title for columns or rows of the given type
 *
 * \param[in] type The type to get the default title for
 *
 * \return Default title for columns of this \c type
 */
std::string label(const CELL_TYPE type);

/**
 * \brief Return default width for columns of the given type
 *
 * \param[in] type The type to get the default width for
 *
 * \return Default width for columns of this \c type
 */
int width(const CELL_TYPE type);

} //namespace defaults


/**
 * \brief Convert from CELL_TYPE to int
 *
 * \param[in] type Type to convert
 *
 * \return Integer representing this type
 */
int convert_from(const CELL_TYPE type);


/**
 * \brief Convert to CELL_TYPE from int
 *
 * \param[in] type Integer representing the type
 *
 * \return Column type
 */
CELL_TYPE convert_to(const int type);


/**
 * \brief A table based format for album data with untyped columns.
 *
 * The tracks are columns and the rows are TOC data or checksums.
 *
 * A layout can be set to format the cells in columns holding checksums.
 */
class TypedRowsTableBase	: public WithMetadataFlagMethods
							, virtual public WithARId
							, virtual public WithChecksumLayout
							, virtual public StringTableStructure
{
public:

	/**
	 * \brief Constructor.
	 *
	 * \param[in] rows     Number of rows
	 * \param[in] columns  Number of columns
	 * \param[in] label    Set to TRUE for printing column titles
	 * \param[in] track    Set to TRUE for printing track number (if any)
	 * \param[in] offset   Set to TRUE for printing offset (if any)
	 * \param[in] length   Set to TRUE for printing length (if any)
	 * \param[in] filename Set to TRUE for printing filename (if any)
	 */
	TypedRowsTableBase(const int rows, const int columns,
			const bool label, const bool track, const bool offset,
			const bool length, const bool filename);

	/**
	 * \brief Constructor.
	 *
	 * \param[in] rows    Number of rows
	 * \param[in] columns Number of columns
	 */
	TypedRowsTableBase(const int rows, const int columns)
		: TypedRowsTableBase(rows, columns, true, true, true, true, true)
	{ /* empty */ };

	// FIXME Make abstract!
};


/**
 * \brief A table based format for album data with typed columns.
 *
 * The tracks are rows and the columns are TOC data or checksums.
 *
 * A layout can be set to format the cells in columns holding checksums.
 *
 * Concrete subclasses define how to preconfigure the columns for checksums or
 * match flags.
 */
class TypedColsTableBase	: public WithMetadataFlagMethods
							, virtual public WithARId
							, virtual public WithChecksumLayout
							, virtual public StringTableStructure
{
public:

	/**
	 * \brief Constructor.
	 *
	 * \param[in] rows     Number of rows
	 * \param[in] columns  Number of columns
	 * \param[in] label    Set to TRUE for printing column titles
	 * \param[in] track    Set to TRUE for printing track number (if any)
	 * \param[in] offset   Set to TRUE for printing offset (if any)
	 * \param[in] length   Set to TRUE for printing length (if any)
	 * \param[in] filename Set to TRUE for printing filename (if any)
	 */
	TypedColsTableBase(const int rows, const int columns,
			const bool label, const bool track, const bool offset,
			const bool length, const bool filename);

	/**
	 * \brief Constructor.
	 *
	 * \param[in] rows    Number of rows
	 * \param[in] columns Number of columns
	 */
	TypedColsTableBase(const int rows, const int columns)
		: TypedColsTableBase(rows, columns, true, true, true, true, true)
	{ /* empty */ };

	/**
	 * \brief Set column type for specified column
	 *
	 * \param[in] col  The column to set the type for
	 * \param[in] type The type to set
	 */
	void assign_type(const int col, const CELL_TYPE type);

	/**
	 * \brief Return type of specified column
	 *
	 * \param[in] col Column index
	 *
	 * \return Type of specified column
	 */
	CELL_TYPE type_of(const int col) const;

	/**
	 * \brief Set widths of all columns with given type
	 *
	 * \param[in] type  The type to get the default title for
	 * \param[in] width The width to set for columns of this \c type
	 */
	void set_widths(const CELL_TYPE type, const int width);

protected:

	/**
	 * \brief Return number of declared metadata columns.
	 *
	 * \return Number of declared metadata columns.
	 */
	int total_metadata_columns() const;

	/**
	 * \brief Apply types and standard settings to metadata columns.
	 *
	 * The metadata columns are: 'Tracks', 'Filenames', 'Offsets' and 'Lengths'.
	 * Those are equal for many types of tables.
	 *
	 * \return Number of metadata columns
	 */
	int columns_apply_md_settings();

private:

	/**
	 * \brief Apply default settings to the CHECKSUM and MATCH columns
	 *
	 * To be called after the metadata columns are initialized, i.e. after
	 * columns_apply_settings() has been called.
	 *
	 * It is expected that this function initializes the columns that are
	 * typed CHECKSUM and MATCH.
	 *
	 * \param[in] types List of types
	 */
	virtual int columns_apply_cs_settings(
			const std::vector<arcstk::checksum::type> &types)
	= 0;
};

} // namespace arcsapp

#endif

