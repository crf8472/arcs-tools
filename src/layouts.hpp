#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#define __ARCSTOOLS_LAYOUTS_HPP__

/**
 * \file
 *
 * \brief Output layouts.
 *
 * The application generates output that is to be formatted as table containing
 * numbers and strings. This module contains table layouts for ARIds,
 * ARTriplets and for the results of the CALC and VERIFY applications. It can
 * also be used to construct new layouts.
 */

#include <cstdint>                // for uint32_t, uint8_t
#include <istream>                // for size_t, ostream
#include <memory>                 // for unique_ptr
#include <string>                 // for string
#include <type_traits>            // for underlying_type_t

#ifndef __LIBARCSTK_IDENTIFIER_HPP__
#include <arcstk/identifier.hpp>
#endif
#ifndef __LIBARCSTK_CALCULATE_HPP__
#include <arcstk/calculate.hpp>
#endif
#ifndef __LIBARCSTK_MATCH_HPP__
#include <arcstk/match.hpp>
#endif
#ifndef __LIBARCSTK_PARSE_HPP__
#include <arcstk/parse.hpp>
#endif

#ifndef __ARCSTOOLS_TABLE_HPP__
#include "table.hpp"
#endif

namespace arcsapp
{

using arcstk::ARId;
using arcstk::ARTriplet;
using arcstk::Checksum;
using arcstk::Checksums;
using arcstk::Match;
using arcstk::TOC;


/**
 * \brief Abstract templatized base class for layouts.
 *
 * Each concrete subclass will provide a function \c format() that accepts
 * exactly the parameters from the template parameter pack as const references.
 */
template <typename ...Args>
class Layout
{
public:

	using ArgsTuple     = std::tuple<Args...>;
	using ArgsRefTuple  = std::tuple<const Args&...>;

	/**
	 * \brief Virtual default destructor
	 */
	inline virtual ~Layout() = default;

	/**
	 * \brief Format objects.
	 *
	 * \param[in] t Tuple of the objects to format
	 */
	inline std::string format(ArgsRefTuple t) const
	{
		this->assertions(t);
		return this->do_format(t);
	}

	/**
	 * \brief Format objects.
	 *
	 * Convenience for not having to turn anything explicitly into tuples.
	 *
	 * \param[in] args The objects to format
	 */
	inline std::string format(const Args&... args) const
	{
		return this->format(std::make_tuple(args...));
	}

protected:

	virtual void assertions(ArgsRefTuple) const { /* empty */ };

private:

	virtual std::string do_format(ArgsRefTuple args) const
	= 0;
};


// Commented out, but left as a note:
// One could now implement a specialization with two parameter packs:
//
//template <typename ...Settings, typename ...Args>
//class Layout<std::tuple<Settings...>, std::tuple<Args...>>
//{
//public:
//
//	using SettingsTuple = std::tuple<Settings...>;
//	using ArgsTuple     = std::tuple<Args...>;
//	using ArgsRefTuple  = std::tuple<const Args*...>;
//
//	...
//};
//
// It could then be used like:
// Layout<std::tuple<int, bool>, std::<Checksum, const char*>)


/**
 * \brief Worker: return underlying value of an integral enum.
 *
 * \tparam E Enum to get the underlying type for
 */
template <typename E>
inline constexpr auto to_underlying(E e) noexcept
{
    return static_cast<std::underlying_type_t<E>>(e);
}


/**
 * \brief Interface for formatting Checksums.
 */
class ChecksumLayout : public Layout<Checksum, int>
{
	/* empty */
};


/**
 * \brief Internal flag API
 *
 * Provides 32 boolean states with accessors.
 */
class InternalFlags
{
public:

	/**
	 * \brief Constructor.
	 *
	 * \param[in] flags Initial internal state
	 */
	explicit InternalFlags(const uint32_t flags);

	/**
	 * \brief Default Constructor.
	 *
	 * Initializes every flag to FALSE.
	 */
	InternalFlags() : InternalFlags(0) { /* empty */ };

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
 * \brief Provides internal settings as member.
 */
class WithInternalFlags
{
public:

	/**
	 * \brief Construct with individual flags.
	 */
	explicit WithInternalFlags(const uint32_t flags) : settings_ { flags }
		{ /* empty */ }

	/**
	 * \brief Default constructor.
	 *
	 * Initializes any internal setting with FALSE.
	 */
	WithInternalFlags() : WithInternalFlags(0) { /* empty */ }

protected:

	/**
	 * \brief Access internal settings.
	 *
	 * \return Settings.
	 */
	InternalFlags& settings() { return settings_; }

	/**
	 * \brief Access internal settings.
	 *
	 * \return Settings.
	 */
	const InternalFlags& settings() const { return settings_; }

private:

	InternalFlags settings_;
};


/**
 * \brief Format numbers in hexadecimal representation
 */
class HexLayout : protected WithInternalFlags
				, public ChecksumLayout
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

	std::string do_format(ArgsRefTuple t) const override;
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

	/**
	 * \brief Virtual destructor.
	 */
	virtual ~WithChecksumLayout() noexcept;

	/**
	 * \brief Set the layout for printing the checksums
	 *
	 * \param[in] layout Layout for printing the checksums
	 */
	void set_checksum_layout(std::unique_ptr<ChecksumLayout> &layout);

	/**
	 * \brief Return the layout for printing the checksums
	 *
	 * \return Layout for printing the checksums
	 */
	const ChecksumLayout& checksum_layout() const;

private:

	std::unique_ptr<ChecksumLayout> checksum_layout_;
};


/**
 * \brief Abstract base class for output formats of ARTriplet.
 */
class ARTripletLayout : protected WithInternalFlags
					  , public Layout<int, ARTriplet>
{
public:

	using Layout<int, ARTriplet>::Layout;

private:

	// no assertions()

	std::string do_format(ArgsRefTuple t) const override;
};


/**
 * \brief Abstract base class for output formats of ARId.
 */
class ARIdLayout : protected WithInternalFlags
				 , public WithChecksumLayout
				 , public Layout<ARId, std::string> // TODO Do also Settings!
{
public:

	/**
	 * \brief Show flags of the ARIdLayout
	 */
	enum class ARID_FLAG : int
	{
		ID       = 0,
		URL      = 1,
		FILENAME = 2,
		TRACKS   = 3,
		ID1      = 4,
		ID2      = 5,
		CDDBID   = 6,
		COUNT    = 7
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
	 * \param[in] labels      Set to TRUE for printing the field labels
	 * \param[in] id          Set to TRUE for printing the ID
	 * \param[in] url         Set to TRUE for printing the URL
	 * \param[in] filename    Set to TRUE for printing the filename
	 * \param[in] track_count Set to TRUE for printing the track_count
	 * \param[in] disc_id_1   Set to TRUE for printing the disc id1
	 * \param[in] disc_id_2   Set to TRUE for printing the disc id2
	 * \param[in] cddb_id     Set to TRUE for printing the cddb id
	 */
	ARIdLayout(const bool labels, const bool id, const bool url,
			const bool filename, const bool track_count, const bool disc_id_1,
			const bool disc_id_2, const bool cddb_id);

	/**
	 * \brief Virtual default destructor
	 */
	virtual ~ARIdLayout() noexcept;

	/**
	 * \brief Returns TRUE iff instance is configured to print field labels.
	 *
	 * \return TRUE iff instance is configured to print field labels
	 */
	bool fieldlabels() const;

	/**
	 * \brief Set to TRUE to print field labels.
	 *
	 * \param[in] labels Flag to indicate that field labels have to be printed
	 */
	void set_fieldlabels(const bool labels);

	/**
	 * \brief Returns TRUE iff instance is configured to format the ID.
	 *
	 * \return ID flag
	 */
	bool id() const;

	/**
	 * \brief Set to TRUE to print the ID.
	 *
	 * \param[in] id Flag to indicate that the ID has to be printed
	 */
	void set_id(const bool id);

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
	 * \param[in] trackcount TRUE indicates to print track count
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
	 * \param[in] disc_id_1 TRUE indicates to print disc id 1
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
	 * \param[in] disc_id_2 TRUE indicates to print disc id 2
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

private:

	/**
	 * \brief Iterable aggregate of the defined flags.
	 *
	 * Order matches definition order in ARID_FLAG.
	 */
	const std::array<ARID_FLAG, to_underlying(ARID_FLAG::COUNT)> show_flags_
	{
		ARID_FLAG::ID,
		ARID_FLAG::URL,
		ARID_FLAG::FILENAME,
		ARID_FLAG::TRACKS,
		ARID_FLAG::ID1,
		ARID_FLAG::ID2,
		ARID_FLAG::CDDBID
	};

	/**
	 * \brief Row label for the defined flags.
	 *
	 * Order matches definition order in ARID_FLAG.
	 */
	const std::array<std::string, to_underlying(ARID_FLAG::COUNT)> labels_
	{
		"ID",
		"URL",
		"Filename",
		"Tracks",
		"ID1",
		"ID2",
		"CDDB ID"
	};

	//std::string do_format(ArgsRefTuple t) const

	bool field_labels_;

protected:

	/**
	 * \brief Worker: print the the sub-ids as part of an ARId.
	 *
	 * \param[in] id Id to print as part of an ARId
	 *
	 * \return Hexadecimal ARId-conforming representation of a 32bit unsigned.
	 */
	std::string hex_id(const uint32_t id) const;

	/**
	 * \brief Getter for the show flags.
	 */
	auto show_flags() const -> decltype( show_flags_ );

	/**
	 * \brief Getter for the flag labels.
	 */
	auto labels() const -> decltype( labels_ );
};


/**
 * \brief Table-style layout for ARId information.
 */
class ARIdTableLayout : public ARIdLayout
{
public:

	using ARIdLayout::ARIdLayout;

private:

	std::string do_format(ArgsRefTuple t) const override;
};


/**
 * \brief Abstract base class for output formats that conatin an ARId.
 */
class WithARIdLayout
{
public:

	/**
	 * \brief Constructor.
	 */
	WithARIdLayout();

	/**
	 * \brief Constructor.
	 *
	 * \param[in] arid_layout The ARIdLayout to set
	 */
	explicit WithARIdLayout(std::unique_ptr<ARIdLayout> arid_layout);

	/**
	 * \brief Virtual default destructor.
	 */
	virtual ~WithARIdLayout() noexcept;

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
	 * \param[in] label Flag to set for printing the labels
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
							, virtual public WithARIdLayout
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

	/**
	 * \brief Constructor.
	 */
	TypedRowsTableBase()
		: TypedRowsTableBase(0, 0, true, true, true, true, true)
	{ /* empty */ };

	// FIXME Make abstract!

protected:

	/**
	 * \brief Apply labels to rows.
	 *
	 * The metadata rows are: 'Tracks', 'Filenames', 'Offsets' and 'Lengths'.
	 * Those are equal for many types of tables.
	 *
	 * \param[in] is_album Flag to indicate whether the layout is for an album
	 *
	 * \return Number of metadata rows
	 */
	int rows_apply_md_settings(const bool is_album);
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
							, virtual public WithARIdLayout
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
	 * \brief Constructor.
	 */
	TypedColsTableBase()
		: TypedColsTableBase(0, 0, true, true, true, true, true)
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


/**
 * \brief Base class for users of a StringTableStructure.
 */
class TableUser : public WithMetadataFlagMethods
{
public:

	/**
	 * \brief Common constructor.
	 *
	 * \param[in] label    Set to TRUE for printing column titles
	 * \param[in] track    Set to TRUE for printing track number (if any)
	 * \param[in] offset   Set to TRUE for printing offset (if any)
	 * \param[in] length   Set to TRUE for printing length (if any)
	 * \param[in] filename Set to TRUE for printing filename (if any)
	 * \param[in] coldelim Set column delimiter
	 */
	TableUser(const bool label, const bool track,
			const bool offset, const bool length, const bool filename,
			const std::string &coldelim);

	/**
	 * \brief Returns the column delimiter.
	 *
	 * \return Delimiter for columns
	 */
	std::string column_delimiter() const;

	/**
	 * \brief Set the column delimiter.
	 *
	 * \param[in] coldelim New column delimiter
	 */
	void set_column_delimiter(const std::string &coldelim);

private:

	std::string coldelim_;
};



/**
 * \brief Interface for formatting the results of the CALC application.
 */
class CalcResultLayout : public TableUser
					   , public Layout < Checksums,
										 std::vector<std::string>,
										 const TOC*,
										 ARId,
										 bool >
{
public:

	using TableUser::TableUser;

protected:

	void assertions(ArgsRefTuple t) const override;
};


/**
 * \brief Formatting the result of an album-mode calculation as a table.
 */
class CalcAlbumTableLayout final : public CalcResultLayout
{
public:

	using CalcResultLayout::CalcResultLayout;

private:

	/**
	 * \brief Internal table class.
	 */
	class TableLayout : public TypedColsTableBase
	{
	public:

		friend CalcAlbumTableLayout;

		using TypedColsTableBase::TypedColsTableBase;

	private:

		void do_init(const int, const int) override { /* empty */ };

		int columns_apply_cs_settings(
			const std::vector<arcstk::checksum::type> &types) override;
	};

	std::string do_format(ArgsRefTuple t) const override;
};


/**
 * \brief Formatting the result of a tracks-mode calculation as a table.
 */
class CalcTracksTableLayout final : public CalcResultLayout // TODO Redundant?
{
public:

	using CalcResultLayout::CalcResultLayout;

private:

	/**
	 * \brief Internal table class.
	 */
	class TableLayout : public TypedRowsTableBase
	{
	public:

		friend CalcTracksTableLayout;

		using TypedRowsTableBase::TypedRowsTableBase;

	private:

		void do_init(const int, const int) override { /* empty */ };
	};

	std::string do_format(ArgsRefTuple t) const override;
};


/**
 * \brief Interface for formatting the results of the VERIFY application.
 */
class VerifyResultLayout : public TableUser
						 , public Layout < Checksums,
										   std::vector<std::string>,
										   std::vector<Checksum>,
										   const Match*,
										   int,
										   bool,
										   const TOC*,
										   ARId >
{
public:

	/**
	 * \brief Constructor.
	 *
	 * \param[in] label    Set to TRUE for printing column titles
	 * \param[in] track    Set to TRUE for printing track number (if any)
	 * \param[in] offset   Set to TRUE for printing offset (if any)
	 * \param[in] length   Set to TRUE for printing length (if any)
	 * \param[in] filename Set to TRUE for printing filename (if any)
	 * \param[in] coldelim Set column delimiter
	 */
	VerifyResultLayout(const bool label, const bool track,
			const bool offset, const bool length, const bool filename,
			const std::string &coldelim)
		: TableUser(label, track, offset, length, filename, coldelim)
		, match_symbol_ { "   ==   " }
	{ /* empty */ }

	/**
	 * \brief Set the symbol to be printed on identity of two checksum values.
	 *
	 * When some values do not match, the respective values are printed.
	 *
	 * \param[in] match_symbol The symbol to represent a match
	 */
	void set_match_symbol(const std::string &match_symbol);

	/**
	 * \brief The symbol to be printed on identity of two checksum values.
	 *
	 * When some values do not match, the respective values are printed.
	 *
	 * \return Match symbol
	 */
	const std::string& match_symbol() const;

protected:

	void assertions(ArgsRefTuple t) const override;

private:

	//std::string do_format(ArgsRefTuple t) const

	/**
	 * \brief The symbol to be printed on identity of two checksum values.
	 */
	std::string match_symbol_;
};


/**
 * \brief Formatting the result of a verification as a table.
 */
class VerifyTableLayout final : public VerifyResultLayout
{
public:

	using VerifyResultLayout::VerifyResultLayout;

private:

	/**
	 * \brief Internal table class.
	 */
	class TableLayout : public TypedColsTableBase
	{
	public:

		friend VerifyTableLayout;

		using TypedColsTableBase::TypedColsTableBase;

	private:

		void do_init(const int, const int) override { /* empty */ };

		int columns_apply_cs_settings(
			const std::vector<arcstk::checksum::type> &types) override;
	};

	std::string do_format(ArgsRefTuple t) const override;
};

} // namespace arcsapp

#endif

