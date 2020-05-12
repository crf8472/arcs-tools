#ifndef __ARCSTOOLS_FORMAT_HPP__
#define __ARCSTOOLS_FORMAT_HPP__

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

#include <cstdint>
#include <deque>
#include <istream>
#include <memory>
#include <string>
#include <vector>

#ifndef __LIBARCSTK_IDENTIFIER_HPP__
#include <arcstk/identifier.hpp>
#endif


using arcstk::ARId;


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

	for (const auto& entry : list)   // TODO Do this with STL!
	{
		if (entry.length() > width)
		{
			width = entry.length();
		}
	}

	return width;
}


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
 * \brief Abstract base class for output formats of ARId.
 */
class ARIdLayout : protected WithInternalFlags
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
	 * \param[in] format The ARIdLayout to set
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
	 * \param[in] track    Set to TRUE for printing track number (if any)
	 * \param[in] offset   Set to TRUE for printing offset (if any)
	 * \param[in] length   Set to TRUE for printing length (if any)
	 * \param[in] filename Set to TRUE for printing filename (if any)
	 */
	WithMetadataFlagMethods(const bool track, const bool offset,
			const bool length, const bool filename);

	/**
	 * \brief Default constructor.
	 *
	 * Constructs an instance with all flags FALSE.
	 */
	WithMetadataFlagMethods() : WithMetadataFlagMethods(
			false, false, false, false) {}

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
};


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
 * \brief Format numbers in hexadecimal representation
 */
class HexLayout : public NumberLayout
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

	/**
	 * Internal layout flags
	 */
	uint8_t flags_;
};


/**
 * \brief A table with formatted columns
 */
class TableLayout
{
public:

	/**
	 * \brief Virtual destructor.
	 */
	virtual ~TableLayout() noexcept;

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
	 * \brief Implements StringTable::rows()
	 *
	 * \return The number of rows
	 */
	virtual std::size_t do_rows() const
	= 0;

	/**
	 * \brief Implements StringTable::columns()
	 *
	 * \return The number of columns
	 */
	virtual std::size_t do_columns() const
	= 0;

	/**
	 * \brief Implements StringTable::set_width(const int, const int)
	 *
	 * \param[in] col  The column id
	 * \param[in] width Widht of the column
	 */
	virtual void do_set_width(const int col, const int width)
	= 0;

	/**
	 * \brief Implements StringTable::width(const int)
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
	 * \brief Implements StringTable::set_alignment(const int, const bool)
	 *
	 * \param[in] col   Column index
	 * \param[in] align Alignment
	 */
	virtual void do_set_alignment(const int col, const bool align)
	= 0;

	/**
	 * \brief Implements StringTable::alignment(const int)
	 *
	 * \param[in] col   Column index
	 *
	 * \return Alignment of this column
	 */
	virtual bool do_alignment(const int col) const
	= 0;

	/**
	 * \brief Implements StringTable::set_title(const int, const std::string&)
	 *
	 * \param[in] col  Column index
	 * \param[in] name Column title
	 */
	virtual void do_set_title(const int col, const std::string &name)
	= 0;

	/**
	 * \brief Implements StringTable::title(const int)
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
	 * \brief Implements StringTable::set_type(const int, const int)
	 *
	 * \param[in] col  Column index
	 * \param[in] name Column type
	 */
	virtual void do_set_type(const int col, const int type)
	= 0;

	/**
	 * \brief Implements StringTable::type(const int)
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
	 * \brief Implements StringTable::set_column_delimiter(const std::string&)
	 *
	 * \param[in] delim The column delimiter symbol
	 */
	virtual void do_set_column_delimiter(const std::string &delim)
	= 0;

	/**
	 * \brief Implements StringTable::column_delimiter()
	 *
	 * \return The column delimiter
	 */
	virtual std::string do_column_delimiter() const
	= 0;

	/**
	 * \brief Implements StringTable::resize(const int, const int)
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
 * Implement function \c init() in a subclass to get a concrete layout.
 */
class StringTableLayout : public TableLayout
{
public:

	/**
	 * \brief Constructor
	 *
	 * \param[in] rows    Number of rows (including header, if any)
	 * \param[in] columns Number of columns (including label column, if any)
	 */
	StringTableLayout(const int rows, const int cols);

	/**
	 * \brief Default constructor constructs a table with dimensions 0,0
	 */
	StringTableLayout() : StringTableLayout(0, 0) { /* empty */ }

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~StringTableLayout() noexcept;

private:

	virtual void init(const int rows, const int cols)
	= 0;

	std::size_t do_rows() const override;
	std::size_t do_columns() const override;

	void do_set_width(const int col, const int width) override;
	int do_width(const int col) const override;

	void do_set_alignment(const int col, const bool align) override;
	bool do_alignment(const int col) const override;

	void do_set_title(const int col, const std::string &name) override;
	std::string do_title(const int col) const override;

	void do_set_type(const int col, const int type) override;
	int do_type(const int col) const override;

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


class StringTable;

std::ostream& operator << (std::ostream &o, const StringTable &table);

/**
 * \brief A table of strings.
 *
 * Implement function \c init() in a subclass to get a concrete table.
 */
class StringTable : public StringTableLayout
{
public:

	friend std::ostream& operator<< (std::ostream &o,
			const StringTable &table);

	/**
	 * \brief Constructor
	 *
	 * \param[in] rows    Number of rows (including header, if any)
	 * \param[in] columns Number of columns (including label column, if any)
	 */
	StringTable(const int rows, const int columns);

	/**
	 * \brief Default constructor constructs a table with dimensions 0,0
	 */
	StringTable() : StringTable(0, 0) { /* empty */ }

	/**
	 * \brief Non-virtual default destructor
	 */
	~StringTable() noexcept;

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

private:

	/**
	 * \brief Implements StringTable::update_cell(const int, const int)
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
	 * \brief Implements StringTable::cell(const int, const int)
	 *
	 * \param[in] row  Row index
	 * \param[in] col  Column index
	 *
	 * \throws std::out_of_range If col > columns() or col < 0.
	 *
	 * \return Content of cell(row, col)
	 */
	virtual std::string do_cell(const int row, const int col) const;

	// forward declaration
	class Impl;

	/**
	 * \brief Private implementation
	 */
	std::unique_ptr<Impl> impl_;
};

#endif

