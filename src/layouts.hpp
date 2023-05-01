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

#include <array>                  // for array
#include <cstddef>                // for size_t
#include <cstdint>                // for uint32_t
#include <map>                    // for map
#include <memory>                 // for unique_ptr
#include <ostream>                // for ostream
#include <string>                 // for string
#include <tuple>                  // for tuple, make_tuple
#include <utility>                // for move
#include <vector>                 // for vector

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

#ifndef __ARCSTOOLS_APPLICATION_HPP__
#include "application.hpp"
#endif
#ifndef __ARCSTOOLS_RESULT_HPP__
#include "result.hpp"                // for ResultObject, Result
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

class Result;

using table::StringTable;
using table::StringTableLayout;


/**
 * \brief Interface for composing a result by records.
 *
 * \tparam T Type of the result object
 * \tparam F Type of the field keys
 */
template<typename T, typename F>
class RecordInterface
{
public:

	using size_type = std::size_t;

	virtual ~RecordInterface() noexcept = default;

	/**
	 * \brief Set the value for the specified field in record \c i.
	 */
	inline void set_field(const int i, const F& field_type,
			const std::string& value)
	{
		this->do_set_field(i, field_type, value);
	}

	/**
	 * \brief The value of the specified field.
	 */
	inline std::string field(const int i, const F& field_type) const
	{
		return this->do_field(i, field_type);
	}

	/**
	 * \brief Set the label for the specified field type.
	 */
	inline void set_label(const F& field_type, const std::string& label)
	{
		this->do_set_label(field_type, label);
	}

	/**
	 * \brief Label for the specified field type.
	 */
	inline std::string label(const F& field_type) const
	{
		return this->do_label(field_type);
	}

	/**
	 * \brief Index of specified field type.
	 */
	inline int field_idx(const F& field_type) const
	{
		return this->do_field_idx(field_type);
	}

	/**
	 * \brief TRUE iff specified field type is part of the result.
	 */
	inline bool has_field(const F& field_type) const
	{
		return this->do_has_field(field_type);
	}

	/**
	 * \brief Total number of records.
	 */
	inline size_type total_records() const
	{
		return this->do_total_records();
	}

	/**
	 * \brief Total number of fields per record.
	 */
	inline size_type fields_per_record() const
	{
		return this->do_fields_per_record();
	}

	/**
	 * \brief Get the result.
	 */
	inline std::unique_ptr<Result> result() const
	{
		return this->do_result();
	}

protected:

	using result_type = ResultObject<T>;

	inline RecordInterface(T&& object)
		: object_ { std::move(object) }
	{
		// empty
	}

	inline T& object()
	{
		return object_;
	}

	inline const T& object() const
	{
		return object_;
	}

private:

	virtual void do_set_field(const int i, const F& field_type,
			const std::string& value)
	= 0;

	virtual const std::string& do_field(const int i, const F& field_type) const
	= 0;

	virtual void do_set_label(const F& field_type, const std::string& label)
	= 0;

	virtual std::string do_label(const F& field_type) const
	= 0;

	virtual int do_field_idx(const F& field_type) const
	= 0;

	virtual bool do_has_field(const F& field_type) const
	= 0;

	virtual size_type do_total_records() const
	= 0;

	virtual size_type do_fields_per_record() const
	= 0;

	virtual std::unique_ptr<Result> do_result() const
	= 0;

	/**
	 * \brief Internal result object.
	 */
	T object_;
};


/**
 * \brief Attributes for representing result data.
 *
 * Use this attributes to define a layout for printing the result. The concrete
 * ResultComposer will know whether the attributes are rows or columns in the
 * result table to build.
 */
enum class ATTR: int
{
	TRACK,
	OFFSET,
	LENGTH,
	FILENAME,
	CHECKSUM_ARCS1,
	CHECKSUM_ARCS2,
	THEIRS_ARCS1,
	MINE_ARCS1,
	THEIRS_ARCS2,
	MINE_ARCS2
};


/**
 * \brief Produce default label for a specified attribute.
 */
template<ATTR A>
std::string DefaultLabel();


/**
 * \brief Interface for constructing a result table.
 *
 * A ResultComposer builds a StringTable with entries and attributes. Each
 * record contains a value for each of its defined fields.
 */
class ResultComposer : public RecordInterface<StringTable, ATTR>
{
public:

	void set_layout(std::unique_ptr<StringTableLayout> layout);

	/**
	 * \brief Return the StringTable instance.
	 *
	 * \return StringTable
	 */
	StringTable table() const;

protected:

	/**
	 * \brief Common constructor.
	 *
	 * \param[in] fields   Ordering of attributes
	 * \param[in] table    Table with dimensions for entries and attributes
	 */
	ResultComposer(const std::vector<ATTR>& fields, StringTable&& table);

	/**
	 * \brief Write value to specified field in record.
	 *
	 * \param[in] record_idx  The record to modify
	 * \param[in] field_idx   The field to modify
	 */
	std::string& value(const int record_idx, const int field_idx);

	/**
	 * \brief Write to table.
	 *
	 * \return Reference to internal table
	 */
	StringTable& in_table();

	/**
	 * \brief Read from table.
	 *
	 * \return Reference to internal table
	 */
	const StringTable& from_table() const;

	/**
	 * \brief Assign each field its respective label.
	 *
	 * The labels are composed from the default names of the attribute labels.
	 */
	void assign_labels();

private:

	void do_set_field(const int record_idx, const ATTR& field_type,
			const std::string& value) final;

	const std::string& do_field(const int i, const ATTR& field_type)
		const final;

	void do_set_label(const ATTR& field_type, const std::string& label) final;

	std::string do_label(const ATTR& field_type) const final;

	int do_field_idx(const ATTR& field_type) const final;

	bool do_has_field(const ATTR& field_type) const final;

	std::unique_ptr<Result> do_result() const final;

	//

	virtual int get_row(const int i, const int j) const
	= 0;

	virtual int get_col(const int i, const int j) const
	= 0;

	virtual void set_field_label(const int field_idx, const std::string& label)
	= 0;

	virtual std::string field_label(const int field_idx) const
	= 0;

	std::vector<ATTR> fields_;
	std::map<ATTR, std::string> labels_;
};


/**
 * \brief Build a table whose records are rows and the fields are columns.
 */
class RowResultComposer final : public ResultComposer
{
	size_type do_total_records() const final;
	size_type do_fields_per_record() const final;

	void set_field_label(const int field_idx, const std::string& label) final;
	std::string field_label(const int field_idx) const final;
	int get_row(const int i, const int j) const final;
	int get_col(const int i, const int j) const final;

public:

	RowResultComposer(const std::size_t entries,
		const std::vector<ATTR>& order, const bool with_labels);
};


/**
 * \brief Build a table whose records are columns and the fields are rows.
 */
class ColResultComposer final : public ResultComposer
{
	size_type do_total_records() const final;
	size_type do_fields_per_record() const final;

	void set_field_label(const int field_idx, const std::string& label) final;
	std::string field_label(const int field_idx) const final;
	int get_row(const int i, const int j) const final;
	int get_col(const int i, const int j) const final;

public:

	ColResultComposer(const std::size_t entries,
		const std::vector<ATTR>& order, const bool with_labels);
};


/**
 * \brief Create a ResultComposer.
 */
class ResultComposerBuilder
{
public:

	std::unique_ptr<ResultComposer> create_composer(
		const std::size_t entries,
		const std::vector<ATTR>& attributes, const bool with_labels) const;

private:

	virtual std::unique_ptr<ResultComposer> do_create_composer(
		const std::size_t entries,
		const std::vector<ATTR>& attributes, const bool with_labels) const
	= 0;
};


/**
 * \brief Create a RowResultComposer.
 */
class RowResultComposerBuilder final : public ResultComposerBuilder
{
	std::unique_ptr<ResultComposer> do_create_composer(
		const std::size_t entries,
		const std::vector<ATTR>& attributes, const bool with_labels) const
		final;
};


/**
 * \brief Create a ColResultComposer.
 */
class ColResultComposerBuilder final : public ResultComposerBuilder
{
	std::unique_ptr<ResultComposer> do_create_composer(
		const std::size_t entries,
		const std::vector<ATTR>& attributes, const bool with_labels) const
		final;
};


/**
 * \brief Provide easy format templates for subclasses.
 *
 * Each concrete subclass will provide a function \c format() that accepts
 * exactly the parameters from the template parameter pack as const references.
 */
template <typename T, typename ...Args>
class Layout
{
public:

	/**
	 * \brief A tuple of const-references of the input arguments.
	 */
	using InputTuple = std::tuple<const Args&...>;

	/**
	 * \brief Virtual default destructor
	 */
	inline virtual ~Layout() = default;

	/**
	 * \brief Format objects.
	 *
	 * \param[in] t Tuple of the objects to format
	 */
	inline T format(InputTuple t) const
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
	inline T format(const Args&... args) const
	{
		return this->format(std::make_tuple(args...));
	}

protected:

	virtual void assertions(InputTuple) const { /* empty */ };

private:

	virtual T do_format(InputTuple args) const
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
//	using InputTuple  = std::tuple<const Args*...>;
//
//	...
//};
//
// It could then be used like:
// Layout<std::tuple<int, bool>, std::<Checksum, const char*>)


/**
 * \brief Access non-public flags.
 *
 * Provides 32 boolean states with accessors.
 */
class InternalFlags final
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
	explicit WithInternalFlags(const uint32_t flags) : flags_ { flags }
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
	InternalFlags& flags() { return flags_; }

	/**
	 * \brief Access internal settings.
	 *
	 * \return Settings.
	 */
	const InternalFlags& flags() const { return flags_; }

private:

	InternalFlags flags_;
};


/**
 * \brief Interface for formatting Checksums.
 */
using ChecksumLayout = Layout<std::string, Checksum, int>;


/**
 * \brief Format Checksums in hexadecimal representation.
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

	std::string do_format(InputTuple t) const override;
};


/**
 * \brief Interface for formatting ARTriplets.
 */
using TripletLayout = Layout<std::string, int, ARTriplet>;


/**
 * \brief Interface for formatting ARTriplet instances for output.
 */
class ARTripletLayout : protected WithInternalFlags
					  , public TripletLayout
{
public:

	using TripletLayout::Layout;

private:

	// no assertions()

	std::string do_format(InputTuple t) const override;
};


/**
 * \brief Interface for formatting ARIds.
 */
using IdLayout = Layout<std::string, ARId, std::string>;


/**
 * \brief Interface for formatting ARId instances for output.
 */
class ARIdLayout : protected WithInternalFlags
				 , public IdLayout // TODO Do also Settings!
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

	std::unique_ptr<ARIdLayout> clone() const;

private:

	/**
	 * \brief Iterable aggregate of the defined flags.
	 *
	 * Order matches definition order in ARID_FLAG.
	 */
	const std::array<ARID_FLAG,
		details::to_underlying(ARID_FLAG::COUNT)> show_flags_
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
	const std::array<std::string,
		details::to_underlying(ARID_FLAG::COUNT)> labels_
	{
		"ID",
		"URL",
		"Filename",
		"Tracks",
		"ID1",
		"ID2",
		"CDDB ID"
	};

	/**
	 * \brief Field labels.
	 */
	bool field_labels_;

	virtual std::unique_ptr<ARIdLayout> do_clone() const
	= 0;

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

	std::unique_ptr<ARIdLayout> do_clone() const override;

	std::string do_format(InputTuple t) const override;
};


/**
 * \brief An ARId accompanied by a layout and an optional URL prefix.
 *
 * This object contains all information necessary to be printed.
 */
class RichARId
{
public:

	/**
	 * \brief An ARId with every information required for printing.
	 *
	 * \param[in] id      ARId to print
	 * \param[in] layout  Layout to use for printing
	 */
	RichARId(const ARId& id, std::unique_ptr<ARIdLayout> layout);

	/**
	 * \brief An ARId with every information required for printing.
	 *
	 * \param[in] id         ARId to print
	 * \param[in] layout     Layout to use for printing
	 * \param[in] alt_prefix Optional alternative URL prefix
	 */
	RichARId(const ARId& id, std::unique_ptr<ARIdLayout> layout,
			const std::string& alt_prefix);


	const ARId& id() const;

	const ARIdLayout& layout() const;

	const std::string& alt_prefix() const;

private:

	ARId id_;

	std::unique_ptr<ARIdLayout> layout_;

	std::string alt_prefix_;
};


std::ostream& operator << (std::ostream& o, const RichARId& a);


/**
 * \brief Abstract base class for result formatting.
 */
class ResultFormatter : public WithInternalFlags
{
public:

	/**
	 * \brief Set the ResultComposerBuilder to use.
	 *
	 * \param[in] c The ResultComposerBuilder to use
	 */
	void set_builder_creator(std::unique_ptr<ResultComposerBuilder> c);

	/**
	 * \brief The ResultComposerBuilder to use.
	 *
	 * \return The ResultComposerBuilder to use
	 */
	const ResultComposerBuilder* builder_creator() const;

	/**
	 * \brief Create the internal ResultComposer to compose the result data.
	 *
	 * Uses the internal ResultComposer instance.
	 */
	std::unique_ptr<ResultComposer> create_composer(
		const std::size_t entries,
		const std::vector<ATTR>& attributes, const bool with_labels) const;

	/**
	 * \brief Set the layout to use for formatting the output table.
	 *
	 * \param[in] table_layout The StringTableLayout to set
	 */
	void set_table_layout(const StringTableLayout& table_layout);

	/**
	 * \brief Layout to use for formatting the output table.
	 *
	 * \return The StringTableLayout to use
	 */
	StringTableLayout table_layout() const;

	/**
	 * \brief Set the layout to use for formatting the ARId.
	 *
	 * \param[in] arid_layout The ARIdLayout to set
	 */
	void set_arid_layout(std::unique_ptr<ARIdLayout> arid_layout);

	/**
	 * \brief Layout to use for formatting the ARId.
	 *
	 * \return The internal ARIdLayout
	 */
	const ARIdLayout* arid_layout() const;

	/**
	 * \brief Set the layout for printing the checksums
	 *
	 * \param[in] layout Layout for printing the checksums
	 */
	void set_checksum_layout(std::unique_ptr<ChecksumLayout> layout);

	/**
	 * \brief Return the layout for printing the checksums
	 *
	 * \return Layout for printing the checksums
	 */
	const ChecksumLayout* checksum_layout() const;

	/**
	 * \brief Set the list of requested checksum types.
	 *
	 * \param[in] types List of requested checksum types.
	 */
	void set_types_to_print(std::vector<arcstk::checksum::type> types);

	/**
	 * \brief List of requested checksum types.
	 *
	 * \return List of requested checksum types.
	 */
	std::vector<arcstk::checksum::type> types_to_print() const;

	/**
	 * \brief Return TRUE iff instance is configured to format the label.
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

protected:

	/**
	 * \brief Build the result representation.
	 *
	 * Calls create_attributes() to determine and create the attributes for the
	 * result object, create_composer() to instantiate the ResultComposer
	 * instance, and build_table() to populate the result table with the
	 * relevant data.
	 *
	 * \param[in] checksums      Calculated checksums
	 * \param[in] refsums        Reference checksums
	 * \param[in] diff           Match between calculation and reference
	 * \param[in] block          Best block in \c diff
	 * \param[in] toc            TOC for calculated checksums
	 * \param[in] arid           ARId of input
	 * \param[in] alt_prefix     Alternative AccurateRip URL prefix
	 * \param[in] filenames      List of input filenames
	 * \param[in] types_to_print List of Checksum types requested for print
	 */
	std::unique_ptr<Result> build_result(
		const Checksums& checksums, const std::vector<Checksum>& refsums,
		const Match* diff, int block, const TOC* toc, const ARId& arid,
		const std::string& alt_prefix,
		const std::vector<std::string>& filenames,
		const std::vector<arcstk::checksum::type>& types_to_print) const;

	/**
	 * \brief Validate the result objects common to every result.
	 *
	 * Throws if validation fails.
	 *
	 * \param[in] checksums  Checksums as resulted
	 * \param[in] filenames  Filenames as resulted
	 * \param[in] toc        TOC as resulted
	 * \param[in] arid       ARId as resulted
	 */
	void validate(const Checksums& checksums, const TOC* toc,
		const ARId& arid, const std::vector<std::string>& filenames) const;

	/**
	 * \brief Create the result attributes.
	 *
	 * \param[in] tracks    Iff TRUE, print tracks
	 * \param[in] offsets   Iff TRUE, print offsets
	 * \param[in] lengths   Iff TRUE, print lengths
	 * \param[in] filenames Iff TRUE, print filenames
	 * \param[in] types     List of checksum types to print
	 *
	 * \return Sequence of result attributes to form an record
	 */
	std::vector<ATTR> create_attributes(const bool tracks,
		const bool offsets, const bool lengths, const bool filenames,
		const std::vector<arcstk::checksum::type>& types) const;

	/**
	 * \brief Description.
	 */
	RichARId build_id(const TOC* toc, const ARId& arid,
		const std::string& alt_prefix) const;

	/**
	 * \brief Build the result table.
	 *
	 * \param[in] checksums   Checksums as resulted
	 * \param[in] refsums     Reference checksums as specified
	 * \param[in] match       Match object (maybe null)
	 * \param[in] block       Index to choose from \c match
	 * \param[in] filenames   Filenames as resulted
	 * \param[in] toc         TOC as resulted
	 * \param[in] arid        ARId as resulted
	 * \param[in] b           ResultComposer to use
	 * \param[in] p_tracks    Iff TRUE, print tracks
	 * \param[in] p_offsets   Iff TRUE, print offsets
	 * \param[in] p_lengths   Iff TRUE, print lengths
	 * \param[in] p_filenames Iff TRUE, print filenames
	 * \param[in] types       List of checksum types to print
	 *
	 * \return Table with result data
	 */
	StringTable build_table(const Checksums& checksums,
		const std::vector<Checksum>& refsums,
		const Match* match, const int block,
		const TOC* toc, const ARId& arid,
		const std::vector<std::string>& filenames,
		const bool p_tracks, const bool p_offsets, const bool p_lengths,
		const bool p_filenames,
		const std::vector<arcstk::checksum::type>& types) const;

	/**
	 * \brief Print their checksums.
	 *
	 * Used by build_table().
	 *
	 * Note that \c record also determines access to \c checksums.
	 *
	 * \param[in] checksums  Reference checksums
	 * \param[in] t          Checksum type to print
	 * \param[in] record      Index of the record in \c b to edit
	 * \param[in] b          ResultComposer to use
	 */
	void their_checksum(const std::vector<Checksum>& checksums,
		const arcstk::checksum::type t, const int record, ResultComposer* b)
		const;

	/**
	 * \brief Print my checksums.
	 *
	 * Used by build_table().
	 *
	 * Note that \c record also determines access to \c checksums.
	 *
	 * \param[in] checksums  Result checksums
	 * \param[in] t          Checksum type to print
	 * \param[in] record      Index of the record in \c b to edit
	 * \param[in] b          ResultComposer to use
	 * \param[in] match      Decide to print match_symbol() or checksum
	 */
	void mine_checksum(const Checksums& checksums,
		const arcstk::checksum::type t, const int record, ResultComposer* b,
		const bool match) const;

	/**
	 * \brief Worker for mine_checksums().
	 *
	 * If checksum_layout() is available for formatting the checksums, it
	 * is used, otherwise the fitting implementation of operator '<<' is
	 * picked (which could be libarcstk's).
	 *
	 * The caller is responsible that \c record picks an existing column
	 * in \c b. (E.g. you wont' pass 'mine' in some subclass like
	 * CalcResultFormatter that does not define an attribute 'mine'.)
	 *
	 * \param[in] checksums  Result checksums
	 * \param[in] type       Checksum type to print
	 * \param[in] record      Index of the record in \c b to edit
	 * \param[in] func       Cell reference function to use
	 */
	void checksum_worker(const int record, ATTR a, const Checksum& checksum,
		ResultComposer* b) const;

private:

	virtual std::vector<ATTR> do_create_attributes(
		const bool tracks, const bool offsets, const bool lengths,
		const bool filenames,
		const std::vector<arcstk::checksum::type>& types_to_print) const
	= 0;

	/**
	 * \brief Configure ResultComposer.
	 *
	 * This is called by build_result() before the composer creates the result
	 * object. Its configuration can be adjusted by implementing this function.
	 *
	 * \param[in,out] composer The ResultComposer to be configured
	 */
	virtual void configure_composer(ResultComposer& composer) const
	= 0;

	virtual void do_their_checksum(const std::vector<Checksum>& checksums,
		const arcstk::checksum::type t, const int record, ResultComposer* b) const
	= 0;

	virtual void do_mine_checksum(const Checksums& checksums,
		const arcstk::checksum::type t, const int record, ResultComposer* b,
		const bool match) const
	= 0;

	/**
	 * \brief Internal ResultComposerBuilder.
	 */
	std::unique_ptr<ResultComposerBuilder> builder_creator_;

	/**
	 * \brief Format for the result StringTable.
	 */
	StringTableLayout table_layout_;

	/**
	 * \brief Format for the ARId.
	 */
	std::unique_ptr<ARIdLayout> arid_layout_;

	/**
	 * \brief Format for the Checksums.
	 */
	std::unique_ptr<ChecksumLayout> checksum_layout_;

	/**
	 * \brief List of requested checksum types.
	 */
	std::vector<arcstk::checksum::type> types_;
};

} // namespace arcsapp

#endif

