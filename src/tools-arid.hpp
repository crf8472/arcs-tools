#ifndef __ARCSTOOLS_FMTARID_HPP__
#define __ARCSTOOLS_FMTARID_HPP__

/**
 * \file
 *
 * \brief Formatter for ARId objects.
 */

#include <memory>       // for unique_ptr
#include <ostream>      // for ostream
#include <string>       // for string

#ifndef __LIBARCSTK_IDENTIFIER_HPP__
#include <arcstk/identifier.hpp>  // for ARId
#endif

#ifndef __ARCSTOOLS_LAYOUTS_HPP__
#include "layouts.hpp"      // for Layout, ARIdLayout
#endif

namespace arcsapp
{

/**
 * \brief Tools and helpers for managing ARIds.
 */
namespace arid
{

using ARId = arcstk::ARId;


/**
 * \brief Interface for formatting ARIds.
 */
using IdLayout = Layout<std::string, arcstk::ARId, std::string>;


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
		static_cast<std::underlying_type_t<ARID_FLAG>>(ARID_FLAG::COUNT)>
			show_flags_
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
		static_cast<std::underlying_type_t<ARID_FLAG>>(ARID_FLAG::COUNT)>
			labels_
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
	 *
	 * \return Show flags
	 */
	auto show_flags() const -> decltype( show_flags_ );

	/**
	 * \brief Getter for the flag labels.
	 *
	 * \return Flag labels
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
	arcstk::ARId id_;

	std::unique_ptr<ARIdLayout> layout_;

	std::string alt_prefix_;

public:

	/**
	 * \brief An ARId with every information required for printing.
	 *
	 * \param[in] id      ARId to print
	 * \param[in] layout  Layout to use for printing
	 */
	RichARId(const arcstk::ARId& id, std::unique_ptr<ARIdLayout> layout);

	/**
	 * \brief An ARId with every information required for printing.
	 *
	 * \param[in] id         ARId to print
	 * \param[in] layout     Layout to use for printing
	 * \param[in] alt_prefix Optional alternative URL prefix
	 */
	RichARId(const arcstk::ARId& id, std::unique_ptr<ARIdLayout> layout,
			const std::string& alt_prefix);

	/**
	 * \brief ARId of this instance.
	 *
	 * \return ARId of this instance
	 */
	const arcstk::ARId& id() const;

	/**
	 * \brief ARIdLayout of this instance.
	 *
	 * \return Layout used to print ARId
	 */
	const ARIdLayout& layout() const;

	/**
	 * \brief Alternative URL prefix used when printing the ARId.
	 *
	 * \return Alterntaive URL prefix
	 */
	const std::string& alt_prefix() const;
};


/**
 * \brief Stream insertion operator for RichARId.
 */
std::ostream& operator << (std::ostream& o, const RichARId& a);

} // namespace arid
} // namespace arcsapp

#endif

